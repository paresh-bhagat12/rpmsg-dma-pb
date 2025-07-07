# 2DFFT Offload Example
```
This example demonstrates how to offload test data for 2D FFT processing from Linux user-space
to the C7x DSP on TI AM62x platforms using TI’s RPMsg-char framework and Linux DMA Heaps.
```
## Features
```
- Test data (256x256x8) for 2DFFT processing on the C7x DSP
- Output on logging console
```
## Prerequisites
```
- Linux kernel with:
  - remoteproc & rpmsg_char drivers enabled
  - DMA Heap support (e.g. linux,cma heap)
- TI’s ti-rpmsg-char user-space library (installed or in your SDK)
```
## Building
```
Run the following commands from the root:
cmake -S . -B build
cmake --build build
- This will build:
  - The shared library (`libti_rpmsg_dma.so`)
  - The example application (`rpmsg_2dfft_offload_example`)
To install the built files (requires root privileges):
sudo cmake --install build
This install:
- The library to `/usr/lib` (by default)
- The example binary to `/usr/bin`
- Test input data `2dfft_input_data.bin` to `/usr/share/2dfft_test_data/`
- Test expected output data `2dfft_expected_output_data.bin` to `/usr/share/2dfft_test_data/`
- The DSP Test firmware file (dsp_2dfft_offload_offload.c75ss0-0.release.strip.out) to `/usr/lib/firmware`

To build only the example, use:
cmake -S . -B build -DBUILD_2DFFT_OFFLOAD_EXAMPLE=ON
```
## Running the Example
```
1. Ensure your DSP firmware images are installed under /lib/firmware/ as referenced in the config.
2. Launch the example:
	rpmsg_2dfft_offload_example
3. Monitor output logs via UART console/dmesg.
```
