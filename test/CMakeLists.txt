# ujson

add_executable(ujson_test test.cpp)
set_target_properties(ujson_test PROPERTIES FOLDER "ujson")
target_link_libraries(ujson_test ujson)
add_test(ujson_unit_test ujson_test)

add_executable(ujson_fuzz fuzz.cpp)
set_target_properties(ujson_fuzz PROPERTIES FOLDER "ujson")
target_link_libraries(ujson_fuzz ujson)
