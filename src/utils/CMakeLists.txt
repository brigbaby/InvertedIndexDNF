CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

#
SET(UTILS_SRCS RedisLoader.hpp)

# ===================== build
if(TARGET utils)
  MESSAGE("utils has already been defined")
else()
  # -o libioutils.la *.hpp
  ADD_LIBRARY(utils INTERFACE)
  # -ljsoncpp
  TARGET_LINK_LIBRARIES(utils INTERFACE jsoncpp)
  #
  target_link_libraries(utils INTERFACE libhiredis.so)
endif()

