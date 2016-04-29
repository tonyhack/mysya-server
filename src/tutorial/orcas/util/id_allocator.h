#ifndef TUTORIAL_ORCAS_UTIL_ID_ALLOCATOR_H
#define TUTORIAL_ORCAS_UTIL_ID_ALLOCATOR_H

namespace tutorial {
namespace orcas {
namespace util {

template <class INT>
class IdAllocator {
 public:
  explicit IdAllocator(INT init_id) : ids_(init_id) {}
  ~IdAllocator() {}

  INT Allocate() { return this->ids_++; }
  void Deallocate(INT value) {}

 private:
  INT ids_;
};

}  // namespace util
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_UTIL_ID_ALLOCATOR_H
