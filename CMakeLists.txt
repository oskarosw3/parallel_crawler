cmake_minimum_required(VERSION 3.3)

project(parallel_crawler)

list(APPEND CMAKE_PREFIX_PATH "/usr/local")
find_package(CURL REQUIRED)

set(CMAKE_CXX_STANDARD 20)


add_executable(parallel_crawler main.cpp
        SetList.cpp
        SetList.h
        scraper.cpp
        queue.h
        scraper.h
        )




target_link_libraries(parallel_crawler PRIVATE CURL::libcurl )