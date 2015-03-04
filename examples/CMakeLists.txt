# ujson

add_executable(example1 example1.cpp)
set_target_properties(example1 PROPERTIES FOLDER "ujson")
target_link_libraries(example1 ujson)

add_executable(example2 example2.cpp)
set_target_properties(example2 PROPERTIES FOLDER "ujson")
target_link_libraries(example2 ujson)
