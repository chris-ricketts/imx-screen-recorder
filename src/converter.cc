#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <stdexcept>
#include "converter.h"

IPUConverter::IPUConverter(ScreenInfo& screenInfo, dma_addr_t outputAddr) {
  ipuFd = open("/dev/mxc_ipu", O_RDWR, 0);

  if (!ipuFd)
    throw std::runtime_error(getError("Failed to open IPU"));

  memset(&task, 0, sizeof(task));

  task.input.width = screenInfo.screenWidth;
  task.input.height = screenInfo.screenHeight;
  task.input.format = IPU_PIX_FMT_RGB32;
  task.input.paddr = screenInfo.addr;

  task.output.width = screenInfo.screenWidth;
  task.output.height = screenInfo.screenHeight;
  task.output.format = IPU_PIX_FMT_YVU420P;
  task.output.paddr = outputAddr;
}

void IPUConverter::convert() {
  int ret = ioctl(ipuFd, IPU_QUEUE_TASK, &task);

  if (ret < 0)
    throw std::runtime_error(errorWithCode("Queuing IPU task failed", ret));
}

IPUConverter::~IPUConverter() {
  close(ipuFd);
}
