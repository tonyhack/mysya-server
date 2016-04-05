#include <map>
#include <memory>
#include <set>
#include <unordered_map>

#include <mysya/ioevent/event_loop.h>
#include <mysya/ioevent/logger.h>
#include <mysya/qservice/event_loop_thread_pool.h>
#include <mysya/qservice/tcp_service.h>
#include <mysya/qservice/transport_agent.h>

namespace mysya {
namespace qservice {
namespace test {

class Room;
class BroadcastServer;

class Connection {
 public:
  Connection(int sockfd, TransportAgent *transport_agent)
    : sockfd_(sockfd), host_(transport_agent), room_(NULL) {}
  ~Connection() {}

  void SendMessage(const char *data, int size) {
    this->host_->SendMessage(this->sockfd_, data, size);
  }

  int GetSockfd() const {
    return this->sockfd_;
  }
  ::mysya::qservice::TransportAgent *GetHost() const {
    return this->host_;
  }

  Room *GetRoom() {
    return this->room_;
  }
  void SetRoom(Room *room) {
    this->room_ = room;
  }

 private:
  int sockfd_;
  ::mysya::qservice::TransportAgent *host_;
  Room *room_;
};

class Room {
 public:
  typedef std::set<int> ConnectionSet;

  Room(BroadcastServer *host, int max_size = 50);
  ~Room();

  int GetSize() const;
  int GetMaxSize() const;

  void AddConnection(int sockfd);
  void RemoveConnection(int sockfd);

  void Broadcast(const char *data, int size);

 private:
  BroadcastServer *host_;
  int max_size_;
  ConnectionSet connections_;
};

class BroadcastServer {
 public:
  typedef std::unordered_map<int, Connection *> ConnectionHashmap;
  typedef std::vector<Room *> RoomVector;

  BroadcastServer(const ::mysya::ioevent::SocketAddress &listen_addr);
  ~BroadcastServer();

  bool AddConnection(Connection *connection);
  Connection *RemoveConnection(int sockfd);
  Connection *GetConnection(int sockfd);

  Room *AllocateRoom();

  void OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer);

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

  int listen_sockfd_;

  ConnectionHashmap connections_;
  RoomVector rooms_;
};

Room::Room(BroadcastServer *host, int max_size)
  : host_(host), max_size_(max_size) {}
Room::~Room() {}

int Room::GetSize() const {
  return (int)this->connections_.size();
}

int Room::GetMaxSize() const {
  return this->max_size_;
}

void Room::AddConnection(int sockfd) {
  this->connections_.insert(sockfd);
}

void Room::RemoveConnection(int sockfd) {
  this->connections_.erase(sockfd);
}

void Room::Broadcast(const char *data, int size) {
  for (ConnectionSet::const_iterator iter = this->connections_.begin();
      iter != this->connections_.end(); ++iter) {
    Connection *connection = this->host_->GetConnection(*iter);
    connection->SendMessage(data, size);
  }
}

BroadcastServer::BroadcastServer(const ::mysya::ioevent::SocketAddress &listen_addr)
  : app_event_loop_(), thread_pool_(4), tcp_service_(&app_event_loop_, &thread_pool_) {
  this->tcp_service_.SetListenedCallback(
      std::bind(&BroadcastServer::OnListened, this, std::placeholders::_1));
  this->tcp_service_.SetListenErrorCallback(
      std::bind(&BroadcastServer::OnListenError, this, std::placeholders::_1,
        std::placeholders::_2));

  this->tcp_service_.SetConnectCallback(
      std::bind(&BroadcastServer::OnNewConnection, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetReceiveCallback(
      std::bind(&BroadcastServer::OnReceive, this, std::placeholders::_1,
        std::placeholders::_2, std::placeholders::_3));
  this->tcp_service_.SetCloseCallback(
      std::bind(&BroadcastServer::OnClose, this, std::placeholders::_1));
  this->tcp_service_.SetErrorCallback(
      std::bind(&BroadcastServer::OnError, this, std::placeholders::_1,
        std::placeholders::_2));
  this->tcp_service_.SetReceiveDecodeCallback(
      std::bind(&BroadcastServer::OnReceiveDecode, this, std::placeholders::_1,
        std::placeholders::_2));

  this->listen_sockfd_ = this->tcp_service_.Listen(listen_addr);
  this->app_event_loop_.Loop();
}

BroadcastServer::~BroadcastServer() {}

bool BroadcastServer::AddConnection(Connection *connection) {
  ConnectionHashmap::iterator iter = this->connections_.find(connection->GetSockfd());
  if (iter != this->connections_.end()) {
    return false;
  }

  this->connections_.insert(std::make_pair(connection->GetSockfd(), connection));
  return true;
}

Connection *BroadcastServer::RemoveConnection(int sockfd) {
  Connection *connection = NULL;

  ConnectionHashmap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
    this->connections_.erase(iter);
  }

  return connection;
}

Connection *BroadcastServer::GetConnection(int sockfd) {
  Connection *connection = NULL;

  ConnectionHashmap::iterator iter = this->connections_.find(sockfd);
  if (iter != this->connections_.end()) {
    connection = iter->second;
  }

  return connection;
}

Room *BroadcastServer::AllocateRoom() {
  Room *room = NULL;

  for (RoomVector::iterator iter = this->rooms_.begin();
      iter != this->rooms_.end(); ++iter) {
    Room *candidate = *iter;
    if (candidate->GetSize() < candidate->GetMaxSize()) {
      room = candidate;
      break;
    }
  }

  if (room == NULL) {
    room = new (std::nothrow) Room(this);
    this->rooms_.push_back(room);
  }

  return room;
}

void BroadcastServer::OnReceiveDecode(int sockfd, ::mysya::ioevent::DynamicBuffer *buffer) {
  Connection *connection = this->GetConnection(sockfd);
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

void BroadcastServer::OnNewConnection(int sockfd, ::mysya::qservice::TransportAgent *transport_agent) {
  std::unique_ptr<Connection> connection(
      new (std::nothrow) Connection(sockfd, transport_agent));
  if (connection.get() == NULL) {
    return;
  }

  Room *room = this->AllocateRoom();
  if (room == NULL) {
    return;
  }

  connection->SetRoom(room);
  room->AddConnection(sockfd);

  this->AddConnection(connection.get());

  connection.release();
}

void BroadcastServer::OnReceive(int sockfd, const char *data, int size) {
  Connection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  Room *room = connection->GetRoom();
  room->Broadcast(data, size);
}

void BroadcastServer::OnClose(int sockfd) {
  Connection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  Room *room = connection->GetRoom();
  room->RemoveConnection(sockfd);
  delete this->RemoveConnection(sockfd);
}

void BroadcastServer::OnError(int sockfd, int socket_errno) {
  Connection *connection = this->GetConnection(sockfd);
  if (connection == NULL) {
    return;
  }

  Room *room = connection->GetRoom();
  room->RemoveConnection(sockfd);
  delete this->RemoveConnection(sockfd);
}

void BroadcastServer::OnListened(int listen_sockfd) {}

void BroadcastServer::OnListenError(int listen_sockfd, int socket_errno) {
  MYSYA_DEBUG("listened addr failed, socket_errno(%d).", socket_errno);
  exit(0);
}

void TestFunc() {
  ::mysya::ioevent::SocketAddress listen_addr("0.0.0.0", 9999);

  BroadcastServer server(listen_addr);
}

}  // namespace test
}  // namespace qservice
}  // namespace mysya

int main(int argc, char *argv[]) {
  ::mysya::qservice::test::TestFunc();

  return 0;
}
