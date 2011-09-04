# vim: ft=python

# We should follow the standards strictly.
stdFlags = "-std=c++98 -pedantic -Wall -Werror"

env = Environment(CXXFLAGS = " ".join([stdFlags, "-ggdb -g3 -O0", "-I.."]),
        LIBS=["mysqlclient", "boost_thread"])
env.Program(source = ["main.cpp", "exception.cpp", "connection.cpp"], target = "main")
