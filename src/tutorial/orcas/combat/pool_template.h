#ifndef TUTORIAL_ORCAS_COMBAT_POOL_TEMPLATE_H
#define TUTORIAL_ORCAS_COMBAT_POOL_TEMPLATE_H

#include <list>
#include <memory>

namespace tutorial {
namespace orcas {
namespace combat {

template <class E>
class PoolTemplate {
  typedef std::list<E *> PoolList;
  typedef typename PoolList::iterator PoolListIter;

 public:
  PoolTemplate(size_t initial_size, size_t extend_size);
  ~PoolTemplate();

  E *Allocate();
  void Deallocate(E *e);

 private:
  void Extend();
  void Reduce(size_t size);

  size_t initial_size_;
  size_t extend_size_;

  PoolList list_;
};

template <class E>
PoolTemplate<E>::PoolTemplate(size_t initial_size, size_t extend_size)
  : initial_size_(initial_size), extend_size_(extend_size) {
  for (size_t i = 0; i < this->initial_size_; ++i) {
    std::unique_ptr<E> e(new (std::nothrow) E());
    if (e.get() != NULL) {
      this->list_.push_back(e.get());
      e.release();
    }
  }
}

template <class E>
PoolTemplate<E>::~PoolTemplate() {
  for (PoolListIter iter = this->list_.begin();
      iter != this->list_.end(); ++iter) {
    delete *iter;
  }

  this->list_.clear();
}

template <class E>
E *PoolTemplate<E>::Allocate() {
  if (this->list_.empty() == true) {
    this->Extend();
  }

  E *e = NULL;
  PoolListIter iter = this->list_.begin();
  if (iter != this->list_.end()) {
    e = *iter;
    this->list_.erase(iter);
  }

  return e;
}

template <class E>
void PoolTemplate<E>::Deallocate(E *e) {
  this->list_.push_back(e);

  if (this->list_.size() > this->initial_size_) {
    this->Reduce(this->list_.size() - this->initial_size_);
  }
}

template <class E>
void PoolTemplate<E>::Extend() {
  for (size_t i = 0; i < this->extend_size_; ++i) {
    std::unique_ptr<E> e(new (std::nothrow) E());
    if (e.get() != NULL) {
      this->list_.push_back(e.get());
      e.release();
    }
  }
}

template <class E>
void PoolTemplate<E>::Reduce(size_t size) {
  for (size_t i = 0; i < size; ++i) {
    PoolListIter iter = this->list_.begin();
    if (iter == this->list_.end()) {
      break;
    }
    delete *iter;
    this->list_.erase(iter);
  }
}

}  // namespace combat
}  // namespace orcas
}  // namespace tutorial

#endif  // TUTORIAL_ORCAS_COMBAT_POOL_TEMPLATE_H
