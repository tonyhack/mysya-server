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

class BroadcastConnection {
 public:
  BroadcastConnection(int sockfd, TransportAgent *transport_agent)
    : sockfd_(sockfd), timer_id_(-1), host_(transport_agent) {
    char data[1024];
    size_t data_size = snprintf(data, sizeof(data),
        "[%d] %s %s", sockfd, "################################################################",
        "################################################################");

    this->timer_id_ = this->host_->GetAppEventLoop()->StartTimer(100,
        std::bind(&BroadcastConnection::BroadcastRequest, this, std::placeholders::_1,
          std::string(data, data_size)), -1);
  }
  ~BroadcastConnection() {
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

  void BroadcastRequest(int timer_id, const ::std::string &data) {
    MYSYA_DEBUG("BroadcastClient %d %s", data.size(), data.data());
    this->SendMessage(data.data(), data.size());
  }

 private:
  int sockfd_;
  int64_t timer_id_;
  ::mysya::qservice::TransportAgent *host_;
};

class BroadcastClient {
 public:
  typedef std::map<int, BroadcastConnection *> ConnectionMap;
  typedef std::set<int> ConnectionSet;

  BroadcastClient();
  ~BroadcastClient();

  void Start();

  void AsyncConnect(const ::mysya::ioevent::SocketAddress &addr, int timeout_ms = -1);

  bool AddConnection(BroadcastConnection *connection);
  BroadcastConnection *RemoveConnection(int sockfd);
  BroadcastConnection *GetConnection(int sockfd);

  void OnAsyncConnected(int sockfd, TransportAgent *transport_agent);
  void OnAsyncConnectError(int sockfd, int socket_errno);

  void OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);

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

BroadcastClient::BroadcastClient()
  : app_event_loop_(), thread_pool_(2),
    tcp_service_(&app_event_loop_, &thread_pool_) {

  this->tcp_service_.SetAsyncConnectedCallback(
      std::bind(&BroadcastClient::OnAsyncConnected, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetAsyncConnectErroCallback(
      std::bind(&BroadcastClient::OnAsyncConnectError, this, std::placeholders::_1,
        std::placeholders::_2));

  this->tcp_service_.SetReceiveCallback(
      std::bind(&BroadcastClient::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->tcp_service_.SetCloseCallback(
      std::bind(&BroadcastClient::OnClose, this, std::placeholders::_1));
  this->tcp_service_.SetErrorCallback(
      std::bind(&BroadcastClient::OnError, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetReceiveDecodeCallback(
      std::bind(&BroadcastClient::OnReceiveDecode, this, std::placeholders::_1,
        std::placeholders::_2));
}

BroadcastClient::~BroadcastClient() {}

void BroadcastClient::Start() {
  this->app_event_loop_.Loop();
}

void BroadcastClient::AsyncConnect(const ::mysya::ioevent::SocketAddress &addr, int timeout_ms) {
  int sockfd = tcp_service_.AsyncConnect(addr, timeout_ms);
  if (sockfd == -1) {
    MYSYA_ERROR("[ECHO] AsyncConnect(%s:%d, %d) failed.",
        addr.GetHost().data(), addr.GetPort(), timeout_ms);
    return;
  }

  this->pendings_.insert(sockfd);
}

bool BroadcastClient::AddConnection(BroadcastConnection *connection) {
  ConnectionMap::iterator iter = this->connections_.find(connection->GetSockfd());
  if (iter != this->connections_.end()) {
    return false;
  }

  this->connections_.insert(std::make_pair(connection->GetSockfd(), connection));
  return true;
}

BroadcastConnection *BroadcastClient::RemoveConnection(int sockfd) {
  BroadcastConnection *connection = NULL;

  ConnectionMap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
    this->connections_.erase(iter);
  }

  return connection;
}

BroadcastConnection *BroadcastClient::GetConnection(int sockfd) {
  BroadcastConnection *connection = NULL;

  ConnectionMap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
  }

  return connection;
}

void BroadcastClient::OnAsyncConnected(int sockfd, TransportAgent *transport_agent) {
  ConnectionSet::iterator iter = this->pendings_.find(sockfd);
  if (iter == this->pendings_.end()) {
    MYSYA_ERROR("[ECHO] sockfd(%d) not found in pendings.", sockfd);
    return;
  }

  std::unique_ptr<BroadcastConnection> connection(
      new (std::nothrow) BroadcastConnection(sockfd, transport_agent));
  if (connection.get() == NULL) {
    MYSYA_ERROR("[ECHO] Allocate BroadcastConnection failed.");
    return;
  }

  if (this->AddConnection(connection.get()) == false) {
    MYSYA_ERROR("[ECHO] AddConnection(%d) failed.", sockfd);
    return;
  }

  connection.release();

  MYSYA_DEBUG("[ECHO] async connected sockfd(%d).", sockfd);
}

void BroadcastClient::OnAsyncConnectError(int sockfd, int socket_errno) {
  this->pendings_.erase(sockfd);

  MYSYA_DEBUG("[ECHO] async connect error sockfd(%d) socket_errno(%d).", socket_errno);
}

void BroadcastClient::OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
  BroadcastConnection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  int readable_bytes = buffer->ReadableBytes();
  if (readable_bytes <= 0) {
    return;
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
      return;
    }

    buffer->ReadBytes(read_bytes);
  }
}

void BroadcastClient::OnReceive(int sockfd, const char *data, int size) {
  BroadcastConnection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  MYSYA_DEBUG("[ECHO] receive[%s]", data);
}

void BroadcastClient::OnClose(int sockfd) {
  delete this->RemoveConnection(sockfd);

  MYSYA_DEBUG("[ECHO] OnClose.");
}

void BroadcastClient::OnError(int sockfd, int socket_errno) {
  delete this->RemoveConnection(sockfd);

  MYSYA_DEBUG("[ECHO] OnError errno(%d).", socket_errno);
}

void TestFunc() {
  ::mysya::ioevent::SocketAddress listen_addr("0.0.0.0", 9999);

  BroadcastClient client;

  for (size_t i = 0; i < 500; ++i) {
    client.AsyncConnect(listen_addr, 1000);
  }

  client.Start();
}

}  // namespace test
}  // namespace qservice
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::qservice::test::TestFunc();

  return 0;
}
