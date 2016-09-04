
cmake_minimum_required(VERSION 2.4)

SET(DIRS
  ./
  /usr/local/include/opencv
  )

set(SRCS
	main.cpp
	opencv_sfm_viewer.cpp
  )

##OpenCV Config
FIND_PACKAGE(OpenCV REQUIRED)
MESSAGE("-- OPENCV include:   " ${OPENCV_INCLUDE_DIR})
MESSAGE("-- OPENCV libs:   " ${OPENCV_LIBRARIES} )

##OpenGL Config
find_package(GLUT)
find_package(OpenGL)
message("opengl found: " ${OPENGL_FOUND})
message("GLUT found: " ${GLUT_FOUND} ${GLUT_LIBRARIES})

INCLUDE_DIRECTORIES(
  ${DIRS}
  )

LINK_DIRECTORIES(
)

ADD_DEFINITIONS(-DDEBUG -ggdb -O3 -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -fbranch-probabilities)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

################################

ADD_EXECUTABLE(
  opencv_sfm_viewer
  ${SRCS}
)

TARGET_LINK_LIBRARIES(
  opencv_sfm_viewer
  ${OPENCV_LIBRARIES} 
  ${OpenCV_LIBS} 
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
)

################################

foreach(dir ${DIRS})
  FILE(GLOB files ${dir}/*.h*)
  LIST(APPEND h_files ${files})
endforeach(dir)
add_custom_target(TAGS etags --members -l c++ ${h_files} ${SRCS})

