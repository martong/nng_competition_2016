project(lakotelep)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
#find_package(Boost 1.62)
find_package(Boost)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Cannot find boost")
endif()

include_directories(.)
include_directories(${CMAKE_SOURCE_DIR}/util)

#add_executable(generator generator.cpp ${CMAKE_SOURCE_DIR}/util/Point.cpp)

add_executable(lakotelep_test test.cpp)
target_link_libraries(lakotelep_test gtest gmock_main)
add_test(NAME lakotelep_test COMMAND lakotelep_test)

add_executable(lakotelep_main main.cpp)

#add_executable(main solution.cpp main.cpp)
