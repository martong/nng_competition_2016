project(Schrodinger)

include_directories(.)

add_executable(schrodinger solution.cpp test.cpp)
target_link_libraries(schrodinger gtest gmock_main)
add_test(NAME schrodinger_test COMMAND schrodinger)
