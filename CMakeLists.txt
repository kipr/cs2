project(cs2)

cmake_minimum_required(VERSION 2.8.12)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(INCLUDE ${CMAKE_SOURCE_DIR}/include)
set(SRC ${CMAKE_SOURCE_DIR}/src)
set(UI ${CMAKE_SOURCE_DIR}/ui)
set(RC ${CMAKE_SOURCE_DIR}/rc)

include_directories(${INCLUDE} ${CMAKE_BINARY_DIR})

file(GLOB INCLUDES ${INCLUDE}/*.hpp)
file(GLOB SOURCES ${SRC}/*.cpp)
file(GLOB UIS ${UI}/*.ui)
file(GLOB RCS ${RC}/*.qrc)

qt5_wrap_cpp(SOURCES ${INCLUDES})
qt5_wrap_ui(SOURCES ${UIS})
qt5_add_resources(SOURCES ${RCS})

IF(WIN32)
  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/../prefix/include)
  LINK_DIRECTORIES(${CMAKE_SOURCE_DIR}/../prefix/lib)
ENDIF(WIN32)

IF(APPLE)
  add_definitions(-g)
	LINK_DIRECTORIES(/Library/Frameworks/ /usr/local/lib)
	include_directories(/usr/local/include)
	ADD_EXECUTABLE(cs2 MACOSX_BUNDLE ${SOURCES})
ELSEIF(UNIX)
	add_definitions(-fPIC)
	ADD_EXECUTABLE(cs2 ${SOURCES})
ELSEIF(WIN32)
	ADD_EXECUTABLE(cs2 WIN32 ${SOURCES} ${CMAKE_SOURCE_DIR}/compat/mingw.c)
	target_link_libraries(cs2 ws2_32)
ENDIF()

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/deploy)

target_link_libraries(cs2 kovanserial pcompiler kar kovan)

if(WIN32)
	target_link_libraries(cs2 opencv_core300 opencv_imgproc300)
else()
	target_link_libraries(cs2 opencv_core opencv_imgproc)
endif()

qt5_use_modules(cs2 Widgets Network)

#######################################
#            Installation             #
#######################################

macro(copy_files NAME FILES DESTINATION)
  add_custom_target(copy_${NAME} ALL
    COMMENT "Copying files: ${FILES}")

  foreach(FILENAME ${FILES})
    get_filename_component(REAL_FILENAME ${FILENAME} NAME)
    set(DST "${DESTINATION}/${REAL_FILENAME}")

    add_custom_command(
      TARGET copy_${NAME}
      COMMAND ${CMAKE_COMMAND} -E copy "${FILENAME}" "${DST}"
      )
  endforeach(FILENAME)
endmacro(copy_files)

IF(APPLE)
	copy_files(board ${RC}/2013.board ${CMAKE_SOURCE_DIR}/deploy/cs2.app/Contents/)
	copy_files(platform.hints ${RC}/platform.hints ${CMAKE_SOURCE_DIR}/deploy/cs2.app/Contents)
ELSE(APPLE)
	copy_files(board ${RC}/2013.board ${CMAKE_SOURCE_DIR}/deploy/)
	copy_files(platform.hints ${RC}/platform.hints ${CMAKE_SOURCE_DIR}/deploy)
ENDIF(APPLE)

IF(APPLE)
	ADD_CUSTOM_TARGET(cs2_MacDeployQt ALL 
		COMMAND ${_qt5Core_install_prefix}/bin/macdeployqt ${CMAKE_SOURCE_DIR}/deploy/cs2.app -verbose=0
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		DEPENDS cs2)
	
	ADD_CUSTOM_TARGET(Info.plist ALL 
		COMMAND ${CMAKE_COMMAND} -E copy ${RC}/Info.plist ${CMAKE_SOURCE_DIR}/deploy/cs2.app/Contents
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		DEPENDS cs2)
	
	ADD_CUSTOM_TARGET(Icons ALL 
		COMMAND ${CMAKE_COMMAND} -E copy ${RC}/mac_icon.icns ${CMAKE_SOURCE_DIR}/deploy/cs2.app/Contents/Resources
		WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
		DEPENDS cs2 Info.plist cs2_MacDeployQt)
ENDIF()