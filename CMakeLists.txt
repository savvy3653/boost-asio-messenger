cmake_minimum_required(VERSION 3.20)   # можно и 3.31, но лучше чуть ниже для совместимости
project(asio_test)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ищем Boost (нам нужны заголовки и библиотека system)
find_package(Boost REQUIRED COMPONENTS system)

# Создаём исполняемый файл
add_executable(asio_test main.cpp
        server.cpp
        client.cpp
        server.h
        client.h
        additions.h
        additions.cpp)

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

# Подключаем заголовочные файлы Boost (целевой подход)
target_include_directories(asio_test PRIVATE ${Boost_INCLUDE_DIRS})

# Линкуем библиотеки: Boost.System и ws2_32 (последняя нужна для Windows)
target_link_libraries(asio_test PRIVATE ${Boost_LIBRARIES} ws2_32)