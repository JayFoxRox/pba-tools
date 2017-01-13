set(CMAKE_C_STANDARD 11)

add_executable(parseRez parseRez.c)
add_executable(format0001 format0001.c)
add_executable(format0022 format0022.c)
target_link_libraries(format0022 txc_dxtn)
add_executable(format0024 format0024.c)
add_executable(format002A format002A.c)
add_executable(format002C format002C.c)
