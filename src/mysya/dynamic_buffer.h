#ifndef MYSYA_DYNAMIC_BUFFER_H
#define MYSYA_DYNAMIC_BUFFER_H

#include <stddef.h>

#include <vector>

namespace mysya {

class DynamicBuffer {
 public:
  DynamicBuffer(size_t init_size = 1024, size_t expand_size = 256);
  ~DynamicBuffer();

  void Swap(DynamicBuffer &buffer);

  size_t CapacityBytes() const;
  size_t DeprecatedBytes() const;
  size_t ReadableBytes() const;
  size_t WritableBytes() const;

  const char *ReadBegin() const;
  void ReadBytes(size_t size);

  char *WriteBegin();
  void WrittenBytes(size_t size);

  void Append(const char *data, size_t size);
  void Append(const void *data, size_t size);

  size_t Read(char *buffer, size_t size);
  size_t Read(void *buffer, size_t size);

  void ReserveWritableBytes(size_t size);
  void ReduceWritableBytes(size_t size = 0);

 private:
  std::vector<char> buffer_;

  size_t expand_size_;
  size_t read_index_;
  size_t write_index_;
};

}  // namespace mysya

#endif  // MYSYA_DYNAMIC_BUFFER_H
