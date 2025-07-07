/*
 * 2dfft_main.c - Fully aligned with audio_offload reference example
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "rpmsg_2dfft_example.h"
#include "rpmsg.h"
#include "dmabuf.h"
#include "fw_loader.h"

#define INPUT_FILE		"/usr/share/2dfft_input_data.bin"
#define EXPECTED_OUTPUT_FILE	"/usr/share/2dfft_expected_output_data.bin"
#define C7_BASE_FW		"/lib/firmware/ti-ipc/am62dxx/ipc_echo_test_c7x_1_release_strip.xe71"
#define C7_TEST_FW		"/lib/firmware/dsp_2dfft_offload.c75ss0-0.release.strip.out"
#define C7_FW_LINK		"/lib/firmware/am62d-c71_0-fw"
#define C7_FW_STATE		"/sys/class/remoteproc/remoteproc0/stat"
#define RPROC_NAME		"/dev/remoteproc0"
#define DMA_HEAP_NAME		"linux,cma"
#define WIDTH 			256
#define HEIGHT 			256
#define ELEMENTS 		(WIDTH * HEIGHT * 8)
#define DATA_BUF_SIZE 		(ELEMENTS * sizeof(int16_t))
#define PARAM_BUF_SIZE 		(128)
#define C7_PROC_ID		8
#define RMT_EP			14

static struct dma_buf_params  *data_buf = NULL;
static struct dma_buf_params  *param_buf = NULL;

static void cleanup()
{
	switch_firmware(C7_BASE_FW, C7_FW_LINK, C7_FW_STATE);
	dmabuf_heap_destroy(param_buf);
	dmabuf_heap_destroy(data_buf);
	close(rpmsg_fd);
}

void handle_sigint(int sig) {
	DBG("\n Caught signal %d (Ctrl+C). Cleaning up...\n", sig);
        cleanup();
        exit(0);
}

static bool load_test_data(const char *filename, void *buffer, size_t size_bytes) {
	FILE *f = fopen(filename, "rb");
	if (!f) {
		perror("Failed to open binary file");
		return false;
	}
	size_t read = fread(buffer, 1, size_bytes, f);
	fclose(f);
	return (read == size_bytes);
}

static bool compare_output(const void *out_buf, const char *expected_file, size_t size_bytes)
{
	void *expected = malloc(size_bytes);
	if (!expected) {
		perror("Failed to allocate memory for expected output");
		return false;
	}
	if (!load_test_data(expected_file, expected, size_bytes)) {
		free(expected);
		return false;
	}
	bool match = (memcmp(out_buf, expected, size_bytes) == 0);
	free(expected);
	return match;
}

void init_rpmsg_buffer()
{
        lbuf.data_buf = data_buf->kern_addr;
        lbuf.params_buf = param_buf->kern_addr ;
        lbuf.data_size = data_buf->size;
        lbuf.params_size = param_buf->size;

        ibuf.data_buffer = (uint32_t)data_buf->phys_addr;
        ibuf.params_buffer = (uint32_t)param_buf->phys_addr;
        ibuf.data_size = data_buf->size;
        ibuf.params_size = param_buf->size;

        dspParams = (params_t*)lbuf.params_buf;
}

int main()
{
	int ret = -1;

	printf("RPMsg based 2D FFT Offload Example\n");

	rpmsg_fd = init_rpmsg(C7_PROC_ID, RMT_EP);
	if(rpmsg_fd < 0) {
		fprintf(stderr, "Failed to initialize rpmsg\n");
		return rpmsg_fd;
	}
	if(dmabuf_heap_init(DMA_HEAP_NAME, DATA_BUF_SIZE, RPROC_NAME, data_buf) < 0)
	{
		fprintf(stderr, "Failed to initialize data DMA buffer\n");
		goto data_dma_allocate_fail;
	}
	if(dmabuf_heap_init(DMA_HEAP_NAME, PARAM_BUF_SIZE, RPROC_NAME, param_buf) < 0)
	{
		fprintf(stderr, "Failed to initialize params DMA buffer\n");
		goto param_dma_allocate_fail;
	}
        init_rpmsg_buffer();

        DBG("dmabuf for data buffer::  Kernel: %p Phy: 0x%x Size = %d\n",
			lbuf.data_buf, ibuf.data_buffer, lbuf.data_size);
        DBG("dmabuf for params buffer::  Kernel: %p Phy: 0x%x Size = %d\n",
			lbuf.params_buf, ibuf.params_buffer, lbuf.params_size);

	// Load Test firmware
	if(switch_firmware(C7_TEST_FW, C7_FW_LINK, C7_FW_STATE) < 0) {
		fprintf(stderr, "Failed to load C7 firmware\n");
		goto switch_fw_fail;
	}

	dmabuf_sync(data_buf->dma_buf_fd, DMA_BUF_SYNC_START);
	ret = load_test_data(INPUT_FILE, data_buf->kern_addr, DATA_BUF_SIZE);
	dmabuf_sync(data_buf->dma_buf_fd, DMA_BUF_SYNC_END);
	if(ret < 0) {
		fprintf(stderr, "Failed to load input binary\n");
		goto test_data_load_fail;
	}

	// Register signal handler for SIGINT
        signal(SIGINT, handle_sigint);

	int packet_len;
	packet_len = sizeof(ibuf);

	ret = send_msg(rpmsg_fd, (char *)&ibuf, sizeof(ibuf));
	if (ret < 0) {
		printf("send_msg failed, ret = %d\n", ret);
		goto rpmsg_snd_recv_fail;
	}
	if (ret != packet_len) {
		printf("bytes written does not match send request, ret = %d, packet_len = %d\n", ret, packet_len);
		goto rpmsg_snd_recv_fail;
	}
	ret = recv_msg(rpmsg_fd, 256, (char *)&ibuf, &packet_len);
	if (ret < 0) {
		printf("recv_msg failed ret = %d\n", ret);
		goto rpmsg_snd_recv_fail;
	}

	dmabuf_sync(data_buf->dma_buf_fd, DMA_BUF_SYNC_START);
	dmabuf_sync(param_buf->dma_buf_fd, DMA_BUF_SYNC_START);

	bool pass = compare_output(data_buf->kern_addr, EXPECTED_OUTPUT_FILE, DATA_BUF_SIZE);

	printf("\nTest Status: %s\n", pass ? "PASSED" : "FAILED");
	printf("\nDSP Load: %d\n", dspParams->dsp_load);
	printf("Cycle Count: %d\n", dspParams->cycle_count);
	printf("DDR Throughput: %f MB/s\n", dspParams->ddr_throughput);

	dmabuf_sync(data_buf->dma_buf_fd, DMA_BUF_SYNC_START);
	dmabuf_sync(param_buf->dma_buf_fd, DMA_BUF_SYNC_START);

	cleanup();

	return pass ? 0 : 1;

rpmsg_snd_recv_fail:
test_data_load_fail:
	switch_firmware(C7_BASE_FW, C7_FW_LINK, C7_FW_STATE);
switch_fw_fail:
	dmabuf_heap_destroy(param_buf);
param_dma_allocate_fail:
	dmabuf_heap_destroy(data_buf);
data_dma_allocate_fail:
	close(rpmsg_fd);
	return ret;
}
