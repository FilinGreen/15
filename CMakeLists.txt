cmake_minimum_required(VERSION 3.11)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_router.proto graph.proto svg.proto map_renderer.proto transport_catalogue.proto)

set(FILES_CATALOGUE "transport_router.proto" "graph.proto" "svg.proto" "map_renderer.proto" "transport_catalogue.proto" "main.cpp" "json_builder.h" "json_builder.cpp" "json_reader.h" "json_reader.cpp" "map_renderer.h" "map_renderer.cpp" "router.h" "serialization.h" "serialization.cpp" "domain.h" "transport_catalogue.h" "transport_catalogue.cpp" "transport_router.h" "transport_router.cpp")  
set(LIBS "geo.h" "geo.cpp" "json.h" "json.cpp" "svg.h" "svg.cpp" "graph.h" "ranges.h")

source_group("Files" FILES ${FILES_CATALOGUE})
source_group("Libs" FILES ${LIBS})

add_executable(transport_catalogue "${FILES_CATALOGUE}" "${LIBS}" "${PROTO_SRCS}" "${PROTO_HDRS}")

target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)