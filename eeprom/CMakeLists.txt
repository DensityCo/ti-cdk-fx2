cmake_minimum_required (VERSION 2.8.10.2)
project(eepromprogrammer)

if(NOT DEFINED CMAKE_BUILD_TYPE)
set(CMAKE_BUILD_TYPE )
endif(NOT DEFINED CMAKE_BUILD_TYPE)

if(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++14")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -std=c++14")
set(CMAKE_C_FLAGS_RELEASE   "-O3 -std=c11")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -g -std=c11")
else(APPLE)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -ggdb -g3 -std=c++11")
set(CMAKE_C_FLAGS_RELEASE   "-O3 -std=c11")
set(CMAKE_C_FLAGS_DEBUG     "-O0 -ggdb -g3 -std=c11")
endif(APPLE)

set(EEPROM_SOURCE_FILES main.cpp)

include_directories(${CMAKE_INCLUDE_PATH})

add_library(eeprom STATIC eeprom.cpp eeprom.h)
add_executable(eepromprog ${EEPROM_SOURCE_FILES} ${EEPROM_HEADERS} )
if(APPLE)
target_link_libraries(eepromprog libeeprom.a ${CMAKE_INSTALL_PREFIX}/lib/libdocopt.a stdc++ "-framework Foundation" "-framework IOKit" )
else(APPLE)
target_link_libraries(eepromprog ${CMAKE_INSTALL_PREFIX}/lib/libdocopt.a pthread)
endif(APPLE)

install(TARGETS eepromprog DESTINATION bin)
install(TARGETS eeprom DESTINATION lib)
install(FILES eeprom.h DESTINATION include/eeprom)

