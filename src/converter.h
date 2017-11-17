#ifndef _CONVERTER_H_
#define _CONVERTER_H_
#include "ipu.h"
#include "utils.h"

class IPUConverter {
  private:
    int ipuFd;
    struct ipu_task task;

  public:
    IPUConverter(ScreenInfo& screenInfo, dma_addr_t outputAddr);
    void convert();
    ~IPUConverter();
};
#endif
