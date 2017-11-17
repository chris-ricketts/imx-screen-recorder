#ifndef _ENCODER_H_
#define _ENCODER_H_
#include <imxvpuapi/imxvpuapi.h>
#include "utils.h"

#define COLOR_FORMAT IMX_VPU_COLOR_FORMAT_YUV420
#define FPS_NUM 30
#define FPS_DEN 1

class VPUEncoder {
  private:
    ImxVpuEncoder *enc;
    ImxVpuDMABuffer *bstreamBuf;
    size_t bstreamBufSize;
    unsigned int bstreamBufAlign;
    ImxVpuEncInitialInfo initialInfo;
    ImxVpuFramebuffer inputFb;
    ImxVpuDMABuffer *inputFbBuf;
    ImxVpuFramebuffer *framebuffers;
    ImxVpuDMABuffer **fbDMABuffers;
    unsigned int numFramebuffers;
    ImxVpuFramebufferSizes calculatedFbSizes;
    ImxVpuRawFrame inputFrame;
    ImxVpuEncodedFrame outputFrame;
    ImxVpuEncParams encParams;

  public:
    VPUEncoder(ScreenInfo& screenInfo);
    unsigned long getDMAInputAddr();
    void* encode();
    size_t encodedSize();
    ~VPUEncoder();
};
    
    
#endif
