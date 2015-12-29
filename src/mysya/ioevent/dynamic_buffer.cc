#include <mysya/ioevent/dynamic_buffer.h>

#include <string.h>

namespace mysya {
namespace ioevent {

DynamicBuffer::DynamicBuffer(size_t init_size, size_t expand_size) :
  buffer_(init_size), init_size_(init_size), expand_size_(expand_size),
  read_index_(0), write_index_(0) {}
DynamicBuffer::~DynamicBuffer() {}

void DynamicBuffer::Swap(DynamicBuffer &buffer) {
  this->buffer_.swap(buffer.buffer_);
  std::swap(this->expand_size_, buffer.expand_size_);
  std::swap(this->read_index_, buffer.read_index_);
  std::swap(this->write_index_, buffer.write_index_);
}

size_t DynamicBuffer::CapacityBytes() const {
  return this->buffer_.size();
}

size_t DynamicBuffer::DeprecatedBytes() const {
  return this->read_index_;
}

size_t DynamicBuffer::ReadableBytes() const {
  return this->write_index_ - this->read_index_;
}

size_t DynamicBuffer::WritableBytes() const {
  return this->buffer_.size() - this->write_index_;
}

char *DynamicBuffer::ReadBegin() {
  return &this->buffer_[this->read_index_];
}

const char *DynamicBuffer::ReadBegin() const {
  return &this->buffer_[this->read_index_];
}

void DynamicBuffer::ReadBytes(size_t size) {
  this->read_index_ += std::min(size, this->ReadableBytes());

  if (this->read_index_ == this->write_index_) {
    this->read_index_ = 0;
    this->write_index_ = 0;
  }
}

char *DynamicBuffer::WriteBegin() {
  return &this->buffer_[this->write_index_];
}

void DynamicBuffer::WrittenBytes(size_t size) {
  this->write_index_ += std::min(size, this->WritableBytes());
}

void DynamicBuffer::Append(const char *data, size_t size) {
  this->ReserveWritableBytes(size);
  ::memcpy(this->WriteBegin(), data, size);
  this->WrittenBytes(size);
}

void DynamicBuffer::Append(const void *data, size_t size) {
  this->Append(static_cast<const char *>(data), size);
}

size_t DynamicBuffer::Read(char *buffer, size_t size) {
  size = std::min(size, this->ReadableBytes());
  memcpy(buffer, this->ReadBegin(), size);
  return size;
}

size_t DynamicBuffer::Read(void *buffer, size_t size) {
  return this->Read(static_cast<char *>(buffer), size);
}

void DynamicBuffer::ReserveWritableBytes(size_t size) {
  if (this->WritableBytes() >= size) {
    return;
  }

  size_t free_size = this->DeprecatedBytes() + this->WritableBytes();
  size_t calloc_size = this->buffer_.size();

  // buffer_.size():
  // +----------------------+--------------+--------------+-----
  // |      init_size_      | expand_size_ | expand_size_ | ...
  // +----------------------+--------------+--------------+-----
  // |                      |              |              |
  // init_size_      +      n * expand_size_
  if (free_size < size) {
    calloc_size += this->expand_size_ *
      ((size - free_size) / this->expand_size_ + 1);
  }

  size_t readable_size = this->ReadableBytes();
  std::vector<char> new_buffer(calloc_size);
  ::memcpy(&new_buffer[0], this->ReadBegin(), readable_size);

  this->buffer_.swap(new_buffer);
  this->read_index_ = 0;
  this->write_index_ = readable_size;
}

void DynamicBuffer::ReduceWritableBytes(size_t size) {
  // TODO
  // Reduce to init_size_ + n * expand_size_

  size_t readable_size = this->ReadableBytes();

  // size_t free_size = this->DeprecatedBytes() + readable_size;

  if (readable_size <= this->init_size_) {
    size_t expand_multiple = (this->CapacityBytes() - this->init_size_) / this->expand_size_;
    expand_multiple /= 2;

    std::vector<char> new_buffer(this->init_size_ + expand_multiple * this->expand_size_);
    ::memcpy(&new_buffer[0], this->ReadBegin(), readable_size);

    this->buffer_.swap(new_buffer);
    this->read_index_ = 0;
    this->write_index_ = readable_size;
  } else {
    size_t readable_expand_size = readable_size - this->init_size_;
    size_t readable_expand_multiple = (readable_expand_size + this->expand_size_) / this->expand_size_;
    size_t expand_multiple =
      (((this->CapacityBytes() - readable_size) / this->expand_size_) - readable_expand_multiple) / 2;

    std::vector<char> new_buffer(this->init_size_ + expand_multiple * this->expand_size_);
    ::memcpy(&new_buffer[0], this->ReadBegin(), readable_size);

    this->buffer_.swap(new_buffer);
    this->read_index_ = 0;
    this->write_index_ = readable_size;
  }
}

}  // namespace ioevent
}  // namespace mysya
