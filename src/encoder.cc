#include <cstdlib>
#include <cstring>
#include "encoder.h"

void* acquire_output_buffer(void *context, size_t size, void **acquired_handle)
{
	void *mem;

	((void)(context));

	mem = malloc(size);
	*acquired_handle = mem;
	return mem;
}


void finish_output_buffer(void *context, void *acquired_handle)
{
	((void)(context));
}

VPUEncoder::VPUEncoder(ScreenInfo& screenInfo) {
  ImxVpuEncOpenParams openParams;

  memset(&openParams, 0, sizeof(openParams));
  imx_vpu_enc_set_default_open_params(IMX_VPU_CODEC_FORMAT_H264, &openParams);
  
	openParams.bitrate = 0; 
	openParams.frame_width = screenInfo.screenWidth; 
	openParams.frame_height = screenInfo.screenHeight;
	openParams.frame_rate_numerator = FPS_NUM;
  openParams.frame_rate_denominator = FPS_DEN;

  imx_vpu_enc_load();
  imx_vpu_enc_get_bitstream_buffer_info(&bstreamBufSize, &bstreamBufAlign);

  bstreamBuf = imx_vpu_dma_buffer_allocate(
      imx_vpu_dec_get_default_allocator(),
      bstreamBufSize,
      bstreamBufAlign,
      0);
  
  imx_vpu_enc_open(&enc, &openParams, bstreamBuf);

  imx_vpu_enc_get_initial_info(enc, &initialInfo);

  numFramebuffers = initialInfo.min_num_required_framebuffers;

  imx_vpu_calc_framebuffer_sizes(
      COLOR_FORMAT, 
      screenInfo.screenWidth, 
      screenInfo.screenHeight,
      initialInfo.framebuffer_alignment, 
      0, 0, 
      &calculatedFbSizes);

  framebuffers = (ImxVpuFramebuffer*) malloc(sizeof(ImxVpuFramebuffer) * numFramebuffers);
  fbDMABuffers = (ImxVpuDMABuffer**) malloc(sizeof(ImxVpuDMABuffer) * numFramebuffers);

  for (int i = 0; i < numFramebuffers; ++i)
	{
		fbDMABuffers[i] = imx_vpu_dma_buffer_allocate(
        imx_vpu_dec_get_default_allocator(), 
        calculatedFbSizes.total_size, 
        initialInfo.framebuffer_alignment, 
        0);

		imx_vpu_fill_framebuffer_params(
        &framebuffers[i], 
        &calculatedFbSizes, 
        fbDMABuffers[i], 
        0);
  }
  
  inputFbBuf = imx_vpu_dma_buffer_allocate(
      imx_vpu_dec_get_default_allocator(), 
      calculatedFbSizes.total_size, 
      initialInfo.framebuffer_alignment, 
      0);

  imx_vpu_fill_framebuffer_params(
      &inputFb, 
      &calculatedFbSizes, 
      inputFbBuf, 
      0);

  imx_vpu_enc_register_framebuffers(enc, framebuffers, numFramebuffers);

  memset(&inputFrame, 0, sizeof(inputFrame));
  inputFrame.framebuffer = &inputFb;

  memset(&encParams, 0, sizeof(encParams));
  encParams.quant_param = 0;
  encParams.acquire_output_buffer = acquire_output_buffer;
	encParams.finish_output_buffer = finish_output_buffer;
  encParams.output_buffer_context = NULL;

  memset(&outputFrame, 0, sizeof(outputFrame));
}

unsigned long VPUEncoder::getDMAInputAddr() {
  return imx_vpu_dma_buffer_get_physical_address(inputFbBuf);
}

void* VPUEncoder::encode() {
  unsigned int result;

  imx_vpu_enc_encode(enc, &inputFrame, &outputFrame, &encParams, &result);

  return outputFrame.acquired_handle;
}

size_t VPUEncoder::encodedSize() {
  return outputFrame.data_size;
}

VPUEncoder::~VPUEncoder() {
  imx_vpu_enc_close(enc);

	imx_vpu_dma_buffer_deallocate(inputFbBuf);

	free(framebuffers);

	for (int i = 0; i < numFramebuffers; i++)
		imx_vpu_dma_buffer_deallocate(fbDMABuffers[i]);

	free(fbDMABuffers);

	imx_vpu_dma_buffer_deallocate(bstreamBuf);

  imx_vpu_enc_unload();
}
