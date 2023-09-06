cmake_minimum_required(VERSION 3.10)

project(WebServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")

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

link_directories(/usr/local/mysql/lib)  # 添加链接器查找路径，以便能找到mysql链接库
add_executable(HttpServer ${SRCS})
target_include_directories(HttpServer PUBLIC . /usr/local/mysql/include)  # 添加目标依赖的头文件，以便能找到mysql头文件
target_link_libraries(HttpServer mysqlclient)  # 链接mysql链接库


MESSAGE(STATUS "project_directory_is:" ${CMAKE_SOURCE_DIR})
MESSAGE(STATUS "binary_directory_is:" ${CMAKE_BINARY_DIR})