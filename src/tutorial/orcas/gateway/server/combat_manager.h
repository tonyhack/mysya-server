#ifndef TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_MANAGER_H
#define TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_MANAGER_H

#include <stdint.h>

#include <set>
#include <unordered_map>

#include <mysya/util/class_util.h>

namespace google {
namespace protobuf {

class Message;

}  // namespace protobuf
}  // namespace google

namespace tutorial {
namespace orcas {
namespace gateway {
namespace server {

class AppServer;
class CombatActor;

// 正式的战斗逻辑对象应该是一个状态机(分成准备、部署、连接、战斗中箸状态)
class Combat {
 public:
  typedef std::pair<int32_t, int32_t> CombatServerArgentKey;

  Combat();
  ~Combat();

  void SetId(int32_t id);
  int32_t GetId() const;
  void SetHost(AppServer *host);
  void SetMapId(int32_t id);
  int32_t GetMapId() const;
  void SetCombatServerId(int32_t id);
  int32_t GetCombatServerId() const;
  void SetCombatArgentId(int32_t id);
  int32_t GetCombatArgentId() const;
  void SetLeft(CombatActor *actor);
  CombatActor *GetLeft();
  void SetRight(CombatActor *actor);
  CombatActor *GetRight();
  int32_t GetConnectedNum() const;
  void SetConnectedNum(int value);

  CombatServerArgentKey GetArgentKey() const;

  void SendMessage(const ::google::protobuf::Message &message);

 private:
  int32_t id_;
  int32_t combat_server_id_;
  int32_t combat_argent_id_;
  AppServer *host_;
  CombatActor *al_;
  CombatActor *ar_;
  int32_t map_id_;
  int32_t connected_num_;
};

class CombatManager {
 public:
  typedef Combat::CombatServerArgentKey CombatServerArgentKey;

  struct CombatServerArgentKeyHash {
    std::size_t operator()(const CombatServerArgentKey &key) const {
      return ((size_t)key.first << 32) + (size_t)key.second;
    }
  };

  typedef std::set<CombatActor *> PendingCombatActorSet;
  typedef std::unordered_map<int32_t, Combat *> CombatHashmap;
  typedef std::unordered_map<CombatServerArgentKey, Combat *, CombatServerArgentKeyHash>
    CombatArgentHashMap;

  void SetHost(AppServer *host);

  bool PushCombat(CombatActor *actor, int32_t map_id);
  void Offline(CombatActor *actor);

  Combat *Allocate();
  void Deallocate(Combat *combat);
  void AddPendiong(Combat *combat);
  Combat *GetPending(int32_t id);
  Combat *RemovePending(int32_t id);

  void AddCombat(Combat *combat);
  Combat *GetCombat(int32_t server_id, int32_t combat_argent_id);
  Combat *RemoveCombat(int32_t server_id, int32_t combat_argent_id);

 private:
  void AddPendingCombatActor(CombatActor *actor);
  void RemovePendingCombatActor(CombatActor *actor);

  int32_t id_allocator_;
  PendingCombatActorSet pending_combat_actors_;
  CombatHashmap pending_combats_;
  CombatArgentHashMap combats_;

  AppServer *host_;

  MYSYA_SINGLETON(CombatManager);
};

}  // namespace server
}  // namespace gateway
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_GATEWAY_SERVER_COMBAT_MANAGER_H
