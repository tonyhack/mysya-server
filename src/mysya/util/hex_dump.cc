#include <mysya/util/hex_dump.h>

#include <stdio.h>

namespace mysya {
namespace util {

void Hexdump(const char *buffer, size_t size, const char *desc) {
#define HEX_DUMP_LINE_CHAR_COUNT 16
  const char *buffer_end = buffer + size;
  const char *line_start = buffer;

  ::printf("========= Hexdump beg size=%lu desc=%s =========\n", size, desc);

  for (;;) {
    const char *line_end = std::min(line_start + HEX_DUMP_LINE_CHAR_COUNT,
        buffer_end);
    if (line_start == line_end) {
      break;
    }

    char output[1024];
    size_t count = 0;
    size_t blank_count = HEX_DUMP_LINE_CHAR_COUNT;

    // hex part
    for (const char *p = line_start; p < line_end; ++p, --blank_count) {
      count += snprintf(output + count, sizeof(output), "%02hhx ", *p);
    }
    for (size_t i = 0; i < blank_count; ++i) {
      count += snprintf(output + count, sizeof(output), "   ");
    }

    // blank
    count += snprintf(output + count, sizeof(output), "    ");

    // acsii part
    for (const char *p = line_start; p < line_end; ++p) {
      count += snprintf(output + count, sizeof(output), "%c",
          isprint(*p) ? *p : '.');
    }

    ::printf("%s\n", output);

    line_start = line_end;
  }

  ::printf("========= Hexdump end size=%lu desc=%s =========\n", size, desc);
#undef HEX_DUMP_LINE_CHAR_COUNT
}

}  // namespace util
}  // namespace mysya
