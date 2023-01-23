cmake_minimum_required(VERSION 3.10)
project(rocket_launcher C)

message("Duff beer for me")
message("Duff beer for you")
message("I'll have a duff")
message("You'll have one, too")

set(CMAKE_C_STANDARD 11)
# execute_process(COMMAND curl-config --cflags RESULT_VARIABLE CURL_FLAGS)
# execute_process(COMMAND "curl-config --libs" OUTPUT_VARIABLE CURL_LIBS)
set(CURL_FLAGS "-I/opt/local/include")
set(CURL_LIBS "-L/opt/local/lib -lcurl")
message("curl flags = " ${CURL_FLAGS})
message("curl libs = " ${CURL_LIBS})

add_compile_options(${CURL_FLAGS})
add_link_options(${CURL_LIBS})

add_executable(rocket_launcher src/main.c src/udp-connection.c)
