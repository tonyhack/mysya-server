#include <map>
#include <memory>
#include <set>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/qservice/event_loop_thread_pool.h>
#include <mysya/qservice/tcp_service.h>
#include <mysya/qservice/transport_agent.h>

namespace mysya {
namespace qservice {
namespace test {

class EchoConnection {
 public:
  EchoConnection(int sockfd, TransportAgent *transport_agent)
    : sockfd_(sockfd), timer_id_(-1), host_(transport_agent) {
    char data[1024];
    size_t data_size = snprintf(data, sizeof(data), "[%d] this is a echo test.", sockfd);

    this->timer_id_ = this->host_->GetAppEventLoop()->StartTimer(200,
        std::bind(&EchoConnection::EchoRequest, this, std::placeholders::_1,
          std::string(data, data_size)), -1);
  }
  ~EchoConnection() {
    this->host_->GetAppEventLoop()->StopTimer(this->timer_id_);
  }

  void SendMessage(const char *data, int size) {
    this->host_->SendMessage(this->sockfd_, data, size);
  }

  int GetSockfd() const {
    return this->sockfd_;
  }
  ::mysya::qservice::TransportAgent *GetHost() const {
    return this->host_;
  }

  void EchoRequest(int timer_id, const ::std::string &data) {
    MYSYA_DEBUG("EchoClient %d %s", data.size(), data.data());
    this->SendMessage(data.data(), data.size());
  }

 private:
  int sockfd_;
  int64_t timer_id_;
  ::mysya::qservice::TransportAgent *host_;
};

class EchoClient {
 public:
  typedef std::map<int, EchoConnection *> ConnectionMap;
  typedef std::set<int> ConnectionSet;

  EchoClient();
  ~EchoClient();

  void Start();

  void AsyncConnect(const ::mysya::ioevent::SocketAddress &addr, int timeout_ms = -1);

  bool AddConnection(EchoConnection *connection);
  EchoConnection *RemoveConnection(int sockfd);
  EchoConnection *GetConnection(int sockfd);

  void OnAsyncConnected(int sockfd, TransportAgent *transport_agent);
  void OnAsyncConnectError(int sockfd, int socket_errno);

  bool OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);

  void OnReceive(int sockfd, const char *data, int size);
  void OnClose(int sockfd);
  void OnError(int sockfd, int socket_errno);

 private:
  ::mysya::ioevent::EventLoop app_event_loop_;
  ::mysya::qservice::EventLoopThreadPool thread_pool_;

  ::mysya::qservice::TcpService tcp_service_;

  ConnectionSet pendings_;
  ConnectionMap connections_;
};

EchoClient::EchoClient()
  : app_event_loop_(), thread_pool_(2),
    tcp_service_(&app_event_loop_, &thread_pool_) {

  this->tcp_service_.SetAsyncConnectedCallback(
      std::bind(&EchoClient::OnAsyncConnected, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetAsyncConnectErroCallback(
      std::bind(&EchoClient::OnAsyncConnectError, this, std::placeholders::_1,
        std::placeholders::_2));

  this->tcp_service_.SetReceiveCallback(
      std::bind(&EchoClient::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->tcp_service_.SetCloseCallback(
      std::bind(&EchoClient::OnClose, this, std::placeholders::_1));
  this->tcp_service_.SetErrorCallback(
      std::bind(&EchoClient::OnError, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetReceiveDecodeCallback(
      std::bind(&EchoClient::OnReceiveDecode, this, std::placeholders::_1,
        std::placeholders::_2));
}

EchoClient::~EchoClient() {}

void EchoClient::Start() {
  this->app_event_loop_.Loop();
}

void EchoClient::AsyncConnect(const ::mysya::ioevent::SocketAddress &addr, int timeout_ms) {
  int sockfd = tcp_service_.AsyncConnect(addr, timeout_ms);
  if (sockfd == -1) {
    MYSYA_ERROR("[ECHO] AsyncConnect(%s:%d, %d) failed.",
        addr.GetHost().data(), addr.GetPort(), timeout_ms);
    return;
  }

  this->pendings_.insert(sockfd);
}

bool EchoClient::AddConnection(EchoConnection *connection) {
  ConnectionMap::iterator iter = this->connections_.find(connection->GetSockfd());
  if (iter != this->connections_.end()) {
    return false;
  }

  this->connections_.insert(std::make_pair(connection->GetSockfd(), connection));
  return true;
}

EchoConnection *EchoClient::RemoveConnection(int sockfd) {
  EchoConnection *connection = NULL;

  ConnectionMap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
    this->connections_.erase(iter);
  }

  return connection;
}

EchoConnection *EchoClient::GetConnection(int sockfd) {
  EchoConnection *connection = NULL;

  ConnectionMap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
  }

  return connection;
}

void EchoClient::OnAsyncConnected(int sockfd, TransportAgent *transport_agent) {
  ConnectionSet::iterator iter = this->pendings_.find(sockfd);
  if (iter == this->pendings_.end()) {
    MYSYA_ERROR("[ECHO] sockfd(%d) not found in pendings.", sockfd);
    return;
  }

  std::unique_ptr<EchoConnection> connection(
      new (std::nothrow) EchoConnection(sockfd, transport_agent));
  if (connection.get() == NULL) {
    MYSYA_ERROR("[ECHO] Allocate EchoConnection failed.");
    return;
  }

  if (this->AddConnection(connection.get()) == false) {
    MYSYA_ERROR("[ECHO] AddConnection(%d) failed.", sockfd);
    return;
  }

  connection.release();

  MYSYA_DEBUG("[ECHO] async connected sockfd(%d).", sockfd);
}

void EchoClient::OnAsyncConnectError(int sockfd, int socket_errno) {
  this->pendings_.erase(sockfd);

  MYSYA_DEBUG("[ECHO] async connect error sockfd(%d) socket_errno(%d).", socket_errno);
}

bool EchoClient::OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
  EchoConnection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return false;
  }

  int readable_bytes = buffer->ReadableBytes();
  if (readable_bytes <= 0) {
    return false;
  }

  int read_bytes = 0;
  for (int i = 0; i < readable_bytes; ++i) {
    if (buffer->ReadBegin()[i] == '\n') {
      read_bytes = i + 1;
    }
  }

  if (read_bytes > 0) {
    read_bytes = connection->GetHost()->DoReceive(sockfd, buffer->ReadBegin(), read_bytes);
    if (read_bytes < 0) {
      return false;
    }

    buffer->ReadBytes(read_bytes);

    return true;
  }

  return false;
}

void EchoClient::OnReceive(int sockfd, const char *data, int size) {
  EchoConnection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  MYSYA_DEBUG("[ECHO] receive[%s]", data);
}

void EchoClient::OnClose(int sockfd) {
  delete this->RemoveConnection(sockfd);

  MYSYA_DEBUG("[ECHO] OnClose.");
}

void EchoClient::OnError(int sockfd, int socket_errno) {
  delete this->RemoveConnection(sockfd);

  MYSYA_DEBUG("[ECHO] OnError errno(%d).", socket_errno);
}

void TestFunc() {
  ::mysya::ioevent::SocketAddress listen_addr1("0.0.0.0", 9991);
  ::mysya::ioevent::SocketAddress listen_addr2("0.0.0.0", 9992);
  ::mysya::ioevent::SocketAddress listen_addr3("0.0.0.0", 9993);
  ::mysya::ioevent::SocketAddress listen_addr4("0.0.0.0", 9994);
  ::mysya::ioevent::SocketAddress listen_addr5("0.0.0.0", 9995);
  ::mysya::ioevent::SocketAddress listen_addr6("0.0.0.0", 9996);
  ::mysya::ioevent::SocketAddress listen_addr7("0.0.0.0", 9997);
  ::mysya::ioevent::SocketAddress listen_addr8("0.0.0.0", 9998);
  ::mysya::ioevent::SocketAddress listen_addr9("0.0.0.0", 9999);

  EchoClient client;

  client.AsyncConnect(listen_addr1, 100);
  client.AsyncConnect(listen_addr2, 100);
  client.AsyncConnect(listen_addr3, 100);
  client.AsyncConnect(listen_addr4, 100);
  client.AsyncConnect(listen_addr5, 100);
  client.AsyncConnect(listen_addr6, 100);
  client.AsyncConnect(listen_addr7, 100);
  client.AsyncConnect(listen_addr8, 100);
  client.AsyncConnect(listen_addr9, 100);

  client.Start();
}

}  // namespace test
}  // namespace qservice
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::qservice::test::TestFunc();

  return 0;
}
