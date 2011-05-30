# vim: ft=python

# We should follow the standards strictly.
stdFlags = "-std=c++98 -pedantic -Wall -Werror"

env = Environment(CXXFLAGS = " ".join([stdFlags, "-ggdb -g3 -O0"]))
env.Program(source = ["main.cpp"], target = "main")