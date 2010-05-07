ADD_LIBRARY(exception STATIC Exception)

ADD_EXECUTABLE(test main.cpp)

TARGET_LINK_LIBRARIES(test exception)