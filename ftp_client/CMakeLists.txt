cmake_minimum_required(VERSION 3.0.0)
project(final_lab_v2 VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(CTest)
enable_testing()

add_executable(final_lab_v2 main.cpp
                            FrontEnd.cpp
                            Connector.cpp
                            Session.cpp
                            OnceBuffer.cpp
                            Buffer.cpp)

find_library(POCO_NET PocoNet)
find_library(POCO_FD PocoFoundation)

target_link_libraries(final_lab_v2 ${POCO_NET} ${POCO_FD})

add_definitions(-I/usr/local/opt/openssl/include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
