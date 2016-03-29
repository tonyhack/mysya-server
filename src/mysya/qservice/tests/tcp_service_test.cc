#include <map>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/qservice/event_loop_thread_pool.h>
#include <mysya/qservice/tcp_service.h>
#include <mysya/qservice/transport_agent.h>

namespace mysya {
namespace qservice {
namespace test {

namespace socket_server {

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
  typedef TcpService::TransportAgentVector TransportAgentVector;

  EchoServer(const ::mysya::ioevent::SocketAddress &listen_addr)
    : app_event_loop_(), thread_pool_(2), tcp_service_(listen_addr, &app_event_loop_, &thread_pool_) {
    TransportAgentVector &transport_agents = this->tcp_service_.GetTransportAgents();

    for (TransportAgentVector::iterator iter = transport_agents.begin();
        iter != transport_agents.end(); ++iter) {
      ::mysya::qservice::TransportAgent *transport_agent = *iter;
      transport_agent->SetConnectAppCallback(
          std::bind(&EchoServer::OnNewConnection, this, std::placeholders::_1,
            std::placeholders::_2));
      transport_agent->SetReceiveAppCallback(
          std::bind(&EchoServer::OnReceive, this, std::placeholders::_1,
            std::placeholders::_2, std::placeholders::_3));
      transport_agent->SetCloseAppCallback(
          std::bind(&EchoServer::OnClose, this, std::placeholders::_1));
      transport_agent->SetErrorAppCallback(
          std::bind(&EchoServer::OnError, this, std::placeholders::_1,
            std::placeholders::_2));
      transport_agent->SetReceiveDecodeCallback(
          std::bind(&EchoServer::OnReceiveDecode, this, std::placeholders::_1,
            std::placeholders::_2));
    }

    this->app_event_loop_.Loop();
  }

  ~EchoServer() {}

  bool AddConnection(EchoConnection *connection) {
    ConnectionMap::iterator iter = this->connections_.find(connection->GetSockfd());
    if (iter != this->connections_.end()) {
      return false;
    }

    this->connections_.insert(std::make_pair(connection->GetSockfd(), connection));
    return true;
  }

  EchoConnection *RemoveConnection(int sockfd) {
    EchoConnection *connection = NULL;

    ConnectionMap::iterator iter = this->connections_.find(sockfd);
    if (iter != this->connections_.end()) {
      connection = iter->second;
      this->connections_.erase(iter);
    }

    return connection;
  }

  EchoConnection *GetConnection(int sockfd) {
    EchoConnection *connection = NULL;

    ConnectionMap::iterator iter = this->connections_.find(sockfd);
    if (iter != this->connections_.end()) {
      connection = iter->second;
    }

    return connection;
  }

  void OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
    EchoConnection *connection = this->GetConnection(sockfd);
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

    MYSYA_DEBUG("[ECHO] OnReceiveDecode read_bytes(%d)", read_bytes);
  }

  void OnNewConnection(int sockfd, ::mysya::qservice::TransportAgent *transport_agent) {
    std::unique_ptr<EchoConnection> connection(
        new (std::nothrow) EchoConnection(sockfd, transport_agent));
    if (connection.get() == NULL) {
      return;
    }

    this->AddConnection(connection.get());

    connection.release();

    MYSYA_DEBUG("[ECHO] OnNewConnection.");
  }

  void OnReceive(int sockfd, const char *data, int size) {
    EchoConnection *connection = this->GetConnection(sockfd);
    if (connection == NULL) {
      return;
    }

    connection->SendMessage(data, size);
  }

  void OnClose(int sockfd) {
    delete this->RemoveConnection(sockfd);

    MYSYA_DEBUG("[ECHO] OnClose.");
  }

  void OnError(int sockfd, int sys_errno) {
    delete this->RemoveConnection(sockfd);

    MYSYA_DEBUG("[ECHO] OnError errno(%d).", sys_errno);
  }

 private:
  ::mysya::ioevent::EventLoop app_event_loop_;
  ::mysya::qservice::EventLoopThreadPool thread_pool_;

  ::mysya::qservice::TcpService tcp_service_;

  ConnectionMap connections_;
};

void TestFunc() {
  ::mysya::ioevent::SocketAddress listen_addr("0.0.0.0", 9999);
  EchoServer server(listen_addr);
}

}  // namespace socket_server

}  // namespace test
}  // namespace qservice
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::qservice::test::socket_server::TestFunc();

  return 0;
}
