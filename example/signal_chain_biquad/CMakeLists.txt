include(FindPkgConfig)

# Cascade Biquad Parametric EQ Signal Chain Example
add_executable(signal_chain_biquad_linux_example
    src/signal_chain_biquad_linux_example.c
    src/audio_codecs.c
)

target_compile_options(signal_chain_biquad_linux_example PRIVATE -Wall -g -O2)

# Include header directory
target_include_directories(signal_chain_biquad_linux_example PRIVATE inc)

target_link_libraries(signal_chain_biquad_linux_example
	ti_rpmsg_dma
	pthread
)

install(TARGETS signal_chain_biquad_linux_example RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/firmware/sigchain_biquad_cascade.c75ss0-0.release.strip.out
        DESTINATION /usr/lib/firmware)
