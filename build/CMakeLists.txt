cmake_minimum_required (VERSION 2.6)

project (LibRetroServerCore)

add_subdirectory("${PROJECT_SOURCE_DIR}/../../Platform/build" "${PROJECT_SOURCE_DIR}/../../Platform_build")
include_directories("${PROJECT_SOURCE_DIR}/../../3rdParty/libretro")
include_directories("${PROJECT_SOURCE_DIR}/../../core/inc")
include_directories("${PROJECT_SOURCE_DIR}/../../Platform/inc")


###############################################
#exe
###############################################
file(GLOB_RECURSE lrc_src1 "${PROJECT_SOURCE_DIR}/../src/*.h")
file(GLOB_RECURSE lrc_src2 "${PROJECT_SOURCE_DIR}/../src/*.cpp")
list(APPEND lrc_src ${lrc_src1})
list(APPEND lrc_src ${lrc_src2})

add_executable(LibRetroServerCore $<TARGET_OBJECTS:platform> ${lrc_src})
						
set_property(TARGET LibRetroServerCore APPEND PROPERTY COMPILE_FLAGS "-WX -W4")

add_custom_command(TARGET LibRetroServerCore POST_BUILD        
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
    "${PROJECT_SOURCE_DIR}/../../3rdParty/libretro/mame_libretro.dll"
    "$<TARGET_FILE_DIR:LibRetroServerCore>/cores/mame_libretro.dll")
	
add_custom_command(TARGET LibRetroServerCore POST_BUILD        
	COMMAND ${CMAKE_COMMAND} -E copy_if_different  
    "${PROJECT_SOURCE_DIR}/../../3rdParty/libretro/libwinpthread-1.dll"
    "$<TARGET_FILE_DIR:LibRetroServerCore>/libwinpthread-1.dll")
		