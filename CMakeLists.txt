cmake_minimum_required(VERSION 3.20)
project(asio_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS)
find_package(Threads REQUIRED)   # для pthread на Linux

add_executable(asio_test src/main.cpp
        src/server/server.cpp
        src/client/client.cpp
        include/server.h
        include/client.h
        include/utils.h
        src/utils/utils.cpp)

target_include_directories(asio_test PRIVATE ${Boost_INCLUDE_DIRS})

if(WIN32)
    target_link_options(asio_test PRIVATE
            -static
            -static-libgcc
            -static-libstdc++
    )
    target_link_libraries(asio_test PRIVATE
            ${Boost_LIBRARIES}
            ws2_32
            mswsock
    )
else()
    target_link_libraries(asio_test PRIVATE
            ${Boost_LIBRARIES}
            Threads::Threads
    )
endif()
