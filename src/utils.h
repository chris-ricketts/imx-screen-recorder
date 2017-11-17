#ifndef _UTILS_H_
#define _UTILS_H_
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <cstring>
#include <cerrno>

typedef struct {
  unsigned int screenWidth;
  unsigned int screenHeight;
  unsigned long addr;
} ScreenInfo;

inline std::string getError(const std::string& msg) {
  std::stringstream ss;
  ss << msg << ": ";
  ss << strerror(errno);

  return ss.str();
}

inline std::string errorWithCode(const std::string& errorMsg, int ret) {
  std::stringstream ss;
  ss << errorMsg << ": ";
  ss << "0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex;
  ss << ret;

  return ss.str();
}
#endif
