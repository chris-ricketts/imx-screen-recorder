#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <chrono>
#include "utils.h"
#include "converter.h"
#include "encoder.h"

using namespace std::chrono;

typedef high_resolution_clock hrClock;

volatile sig_atomic_t stop = 0;

void interupt_handler(int signum) {
  stop = 1;
}

int main() {
  IPUConverter* converter = NULL;
  VPUEncoder* encoder = NULL;

  int fbFd = 0;
  ScreenInfo info;
  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;

  signal(SIGINT, interupt_handler);

  time_point<hrClock> convStart, convEnd;
  time_point<hrClock> encStart, encEnd;
  time_point<hrClock> writeStart, writeEnd;
  unsigned long long totalFrames;
  unsigned long long convDurationTotal;
  unsigned long long encDurationTotal;
  unsigned long long writeDurationTotal;

  try {

    fbFd = open("/dev/fb0", O_RDONLY);

    if (!fbFd)
      throw std::runtime_error(getError("Failed to open /dev/fb0"));

    if (ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) < 0)
      throw std::runtime_error(getError("Failed to get fixed screen info"));

    if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) < 0)
      throw std::runtime_error(getError("Failed to get variable screen info"));

    info.screenWidth = vinfo.xres;
    info.screenHeight = vinfo.yres;
    info.addr = finfo.smem_start;

    encoder = new VPUEncoder(info);

    converter = new IPUConverter(info, encoder->getDMAInputAddr());

    int dummy = 0;
    void* frame = NULL;

    while(!stop) {
      if (ioctl(fbFd, FBIO_WAITFORVSYNC, &dummy) < 0)
        throw std::runtime_error(getError("Failed to wait for vsync"));
      
      convStart = hrClock::now();
      converter->convert();
      convEnd = hrClock::now();
      encStart = hrClock::now();
      frame = encoder->encode();
      encEnd = hrClock::now();
      writeStart = hrClock::now();
      fwrite(frame, 1, encoder->encodedSize(), stdout);
      writeEnd = hrClock::now();
      free(frame);

      auto convDuration = duration_cast<microseconds>(convEnd - convStart);
      auto encDuration = duration_cast<microseconds>(encEnd - encStart);
      auto writeDuration = duration_cast<microseconds>(writeEnd - writeStart);

      convDurationTotal += convDuration.count();
      encDurationTotal += encDuration.count();
      writeDurationTotal += writeDuration.count();
      totalFrames++;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  auto convAvg = (convDurationTotal / totalFrames) / 1000.0f;
  auto encAvg = (encDurationTotal / totalFrames) / 1000.0f;
  auto writeAvg = (writeDurationTotal / totalFrames) / 1000.0f;

  std::cerr << "Average Conversion Time (ms): " << convAvg << std::endl;
  std::cerr << "Average Encoding Time (ms): " << encAvg << std::endl;
  std::cerr << "Average Writing Conversion Time (ms): " << writeAvg << std::endl;

  if (fbFd)
    close(fbFd);

  if (converter)
    delete converter;

  if (encoder)
    delete encoder;

  return 0;
}

