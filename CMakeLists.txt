cmake_minimum_required(VERSION 4.2)
project(MouseTracker C)

set(CMAKE_C_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED gtk4)

add_executable(MouseTracker 
  main.c
  opts.c
  flock.c
  message_box.c
  mouse/input_selector.c
  record/recording.c
  storage/chunk_writer.c
  compression/select_compression.c
  compression/compression_none.c
  storage/reader.c
  plot/plot.c
  plot/session_chooser.c
  plot/plot_window.c
)

add_compile_options(-Wall)

find_library(BZIP2_LIB bz2)
if (BZIP2_LIB)
  target_sources(MouseTracker PRIVATE compression/compression_bzip2.c)

  target_compile_definitions(MouseTracker PRIVATE HAVE_BZIP2)
  target_link_libraries(MouseTracker ${BZIP2_LIB})
endif()

find_library(ZLIB_LIB z)
if (ZLIB_LIB)
  target_sources(MouseTracker PRIVATE compression/compression_gzip.c)

  target_compile_definitions(MouseTracker PRIVATE HAVE_GZIP)
  target_link_libraries(MouseTracker ${ZLIB_LIB})
endif()

find_library(ZSTD_LIB zstd)
if (ZSTD_LIB)
  target_sources(MouseTracker PRIVATE compression/compression_zstd.c)

  target_compile_definitions(MouseTracker PRIVATE HAVE_ZSTD)
  target_link_libraries(MouseTracker ${ZSTD_LIB})
endif()

target_include_directories(MouseTracker PUBLIC ${GTK4_INCLUDE_DIRS})
target_compile_options(MouseTracker PUBLIC ${GTK4_CFLAGS_OTHER})

target_link_libraries(MouseTracker ${GTK4_LIBRARIES} input udev)
