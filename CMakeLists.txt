cmake_minimum_required(VERSION 1.0)
project(MUXTS)

#set(CMAKE_CXX_FLAGES -g -w -std=c++11)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++11 -Wall")


set(PWDIR /home/yanghe/teddy/project_app/en_de_ts/)
include_directories(
${PWDIR}ffmpeg/include
${PWDIR}
)

link_directories(
${PWDIR}ffmpeg/lib
)

aux_source_directory(${PWDIR} SRC_LIST)

add_executable(mux_ts ${SRC_LIST})

