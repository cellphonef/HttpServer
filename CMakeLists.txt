cmake_minimum_required(VERSION 3.10)

project(WebServer)

set(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

set(SRCS 
  main.cpp
  HttpServer.cpp
  HttpRequest.cpp
  HttpResponse.cpp
  HttpConn.cpp
  SocketOps.cpp
  EpollOps.cpp
  Buffer.cpp
  Timer.cpp
  ConnectionPool.cpp
  Mutex.cpp
  ConditionVar.cpp
  ErrorHandler.cpp
  Thread.cpp
  )


add_executable(HttpServer ${SRCS})
target_include_directories(HttpServer PUBLIC . /usr/include/mysql)
link_directories(/usr/local/mysql/lib)
target_link_libraries(HttpServer mysqlclient)


MESSAGE(STATUS "project_directory_is:" ${CMAKE_SOURCE_DIR})
MESSAGE(STATUS "build_directory_is:" ${CMAKE_BINARY_DIR})