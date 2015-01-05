# ujson

add_executable(ujson_test test.cpp)
target_link_libraries(ujson_test ujson)
add_test(ujson_unit_test ujson_test)

add_executable(ujson_fuzz fuzz.cpp)
target_link_libraries(ujson_fuzz ujson)
