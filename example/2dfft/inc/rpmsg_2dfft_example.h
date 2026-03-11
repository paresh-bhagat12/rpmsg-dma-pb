#ifndef RPMSG_2DFFT_EXAMPLE_H
#define RPMSG_2DFFT_EXAMPLE_H

#define DEBUG 0
#if DEBUG
#define DBG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
#define DBG(fmt, ...)
#endif

/* Local (host) descriptor of the buffer */
typedef struct {
	uint32_t *data_buf;	/* mmaped tx dma-buf */
	uint32_t *params_buf;	/* mmaped rx dma-buf */
	int data_size;		/* Data dma-buf size */
	int params_size;	/* Param dma-buf size */
} local_buf_t;

//------- Define EQ control params structure --------
typedef struct __attribute__((__packed__))
{
	int32_t dsp_load;
	int32_t cycle_count;
	float ddr_throughput;
}
params_t;

//------- Define C7 IPC message structure --------
typedef struct __attribute__((__packed__))
{
	uint32_t data_buffer;
	uint32_t params_buffer;
	int32_t data_size;
	int32_t params_size;

}
ipc_msg_buf_t;

static int rpmsg_fd = -1;
params_t *dspParams;
local_buf_t lbuf;
ipc_msg_buf_t ibuf;

#endif //RPMSG_2DFFT_EXAMPLE_H
