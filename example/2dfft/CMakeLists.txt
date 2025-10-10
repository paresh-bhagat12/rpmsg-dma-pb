include(FindPkgConfig)

file(GLOB_RECURSE SRC_FILES "src/*.c")

add_executable(rpmsg_2dfft_example ${SRC_FILES})

target_compile_options(rpmsg_2dfft_example PRIVATE -Wall -g -O2)

target_include_directories(rpmsg_2dfft_example PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/inc
)

target_link_libraries(rpmsg_2dfft_example
	ti_rpmsg_dma
)

install(TARGETS rpmsg_2dfft_example RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_data/2dfft_input_data.bin
	DESTINATION /usr/share/2dfft_test_data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/test_data/2dfft_expected_output_data.bin
	DESTINATION /usr/share/2dfft_test_data/)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/firmware/fft2d_linux_dsp_offload_example.c75ss0-0.release.strip.out
	DESTINATION /usr/lib/firmware)
