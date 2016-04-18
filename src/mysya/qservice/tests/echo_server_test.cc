#include <map>
#include <memory>

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
    : sockfd_(sockfd), host_(transport_agent) {}
  ~EchoConnection() {}

  void SendMessage(const char *data, int size) {
    this->host_->SendMessage(this->sockfd_, data, size);
  }

  int GetSockfd() const {
    return this->sockfd_;
  }
  ::mysya::qservice::TransportAgent *GetHost() const {
    return this->host_;
  }

 private:
  int sockfd_;
  ::mysya::qservice::TransportAgent *host_;
};

class EchoServer {
 public:
  typedef std::map<int, EchoConnection *> ConnectionMap;
  typedef std::map<int, ::mysya::ioevent::SocketAddress> ListenAddrMap;

  EchoServer();
  ~EchoServer();

  void Start();
  int Listen(const ::mysya::ioevent::SocketAddress &listen_addr);

  bool AddConnection(EchoConnection *connection);
  EchoConnection *RemoveConnection(int sockfd);
  EchoConnection *GetConnection(int sockfd);

  bool OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);

  void OnNewConnection(int sockfd, ::mysya::qservice::TransportAgent *transport_agent);
  void OnReceive(int sockfd, const char *data, int size);
  void OnClose(int sockfd);
  void OnError(int sockfd, int socket_errno);

  void OnListened(int listen_sockfd);
  void OnListenError(int listen_sockfd, int socket_errno);

 private:
  ::mysya::ioevent::EventLoop app_event_loop_;
  ::mysya::qservice::EventLoopThreadPool thread_pool_;

  ::mysya::qservice::TcpService tcp_service_;

  ConnectionMap connections_;
  ListenAddrMap listen_addrs_;
};


EchoServer::EchoServer()
  : app_event_loop_(), thread_pool_(2),
    tcp_service_(&app_event_loop_, &thread_pool_) {
  this->tcp_service_.SetListenedCallback(
      std::bind(&EchoServer::OnListened, this, std::placeholders::_1));
  this->tcp_service_.SetListenErrorCallback(
      std::bind(&EchoServer::OnListenError, this, std::placeholders::_1,
        std::placeholders::_2));

  this->tcp_service_.SetConnectCallback(
      std::bind(&EchoServer::OnNewConnection, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetReceiveCallback(
      std::bind(&EchoServer::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->tcp_service_.SetCloseCallback(
      std::bind(&EchoServer::OnClose, this, std::placeholders::_1));
  this->tcp_service_.SetErrorCallback(
      std::bind(&EchoServer::OnError, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetReceiveDecodeCallback(
      std::bind(&EchoServer::OnReceiveDecode, this, std::placeholders::_1,
        std::placeholders::_2));
}

EchoServer::~EchoServer() {}

void EchoServer::Start() {
  this->app_event_loop_.Loop();
}

int EchoServer::Listen(const ::mysya::ioevent::SocketAddress &listen_addr) {
  int listen_sockfd = this->tcp_service_.Listen(listen_addr);
  this->listen_addrs_.insert(std::make_pair(listen_sockfd, listen_addr));
  return listen_sockfd;
}

bool EchoServer::AddConnection(EchoConnection *connection) {
  ConnectionMap::iterator iter = this->connections_.find(connection->GetSockfd());
  if (iter != this->connections_.end()) {
    return false;
  }

  this->connections_.insert(std::make_pair(connection->GetSockfd(), connection));
  return true;
}

EchoConnection *EchoServer::RemoveConnection(int sockfd) {
  EchoConnection *connection = NULL;

  ConnectionMap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
    this->connections_.erase(iter);
  }

  return connection;
}

EchoConnection *EchoServer::GetConnection(int sockfd) {
  EchoConnection *connection = NULL;

  ConnectionMap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
  }

  return connection;
}

bool EchoServer::OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
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

void EchoServer::OnNewConnection(int sockfd, ::mysya::qservice::TransportAgent *transport_agent) {
  std::unique_ptr<EchoConnection> connection(
      new (std::nothrow) EchoConnection(sockfd, transport_agent));
  if (connection.get() == NULL) {
    return;
  }

  this->AddConnection(connection.get());

  connection.release();

  MYSYA_DEBUG("[ECHO] OnNewConnection.");
}

void EchoServer::OnReceive(int sockfd, const char *data, int size) {
  EchoConnection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  connection->SendMessage(data, size);
}

void EchoServer::OnClose(int sockfd) {
  delete this->RemoveConnection(sockfd);

  MYSYA_DEBUG("[ECHO] OnClose.");
}

void EchoServer::OnError(int sockfd, int socket_errno) {
  delete this->RemoveConnection(sockfd);

  MYSYA_DEBUG("[ECHO] OnError errno(%d).", socket_errno);
}

void EchoServer::OnListened(int listen_sockfd) {
  ListenAddrMap::iterator iter = this->listen_addrs_.find(listen_sockfd);
  if (iter == this->listen_addrs_.end()) {
    MYSYA_ERROR("listen_sockfd(%d) not found in listen_addrs_.", listen_sockfd);
    return;
  }

  MYSYA_DEBUG("listened addr(%s:%d)", iter->second.GetHost().data(), iter->second.GetPort());
}

void EchoServer::OnListenError(int listen_sockfd, int socket_errno) {
  ListenAddrMap::iterator iter = this->listen_addrs_.find(listen_sockfd);
  if (iter == this->listen_addrs_.end()) {
    MYSYA_ERROR("listen_sockfd(%d) not found in listen_addrs_.", listen_sockfd);
    return;
  }

  ::mysya::ioevent::SocketAddress listen_addr = iter->second;

  this->listen_addrs_.erase(iter);

  MYSYA_DEBUG("listened addr(%s:%d) failed, socket_errno(%d).",
      listen_addr.GetHost().data(), listen_addr.GetPort(), socket_errno);
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

  EchoServer server;

  if (server.Listen(listen_addr1) == -1 ||
      server.Listen(listen_addr2) == -1 ||
      server.Listen(listen_addr3) == -1 ||
      server.Listen(listen_addr4) == -1 ||
      server.Listen(listen_addr5) == -1 ||
      server.Listen(listen_addr6) == -1 ||
      server.Listen(listen_addr7) == -1 ||
      server.Listen(listen_addr8) == -1 ||
      server.Listen(listen_addr9) == -1) {
    MYSYA_ERROR("Listen failed.");
    return;
  }

  server.Start();
}

}  // namespace test
}  // namespace qservice
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::qservice::test::TestFunc();

  return 0;
}
