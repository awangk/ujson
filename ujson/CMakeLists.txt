# ujson

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/ujson.cpp
  COMMAND re2c
  --no-generation-date
  -i
  -o ${CMAKE_CURRENT_SOURCE_DIR}/ujson.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ujson.re2c
  DEPENDS ujson.re2c)

set(DOUBLE_CONVERSION_SRC
    double-conversion.h
    double-conversion.cc
)

add_library(ujson STATIC

    ujson.hpp
    ujson.re2c
    ujson.cpp

    ${DOUBLE_CONVERSION_SRC}

    ../README.md
    ../LICENSE.md
    ../LICENSE-3RD-PARTY.md
)

set_target_properties(ujson PROPERTIES FOLDER "ujson")

source_group(DoubleConversion FILES ${DOUBLE_CONVERSION_SRC})

if (MSVC)
  set_target_properties(ujson PROPERTIES
    STATIC_LIBRARY_FLAGS_RELEASE "/ltcg")
endif ()

target_include_directories(ujson PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>>)

set(HEADERS ujson.hpp)
install(FILES ${HEADERS} DESTINATION "include/ujson")
install(TARGETS ujson EXPORT ujson-export DESTINATION lib)
