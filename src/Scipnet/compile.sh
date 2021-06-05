#!/bin/sh

g++ --std=c++17 -O3 -o a.out Scipnet/Cli.cpp Scipnet/ColorTable.cpp Scipnet/Converter.cpp Scipnet/Database.cpp Scipnet/main.cpp Scipnet/Ncurses.cpp Scipnet/layout/Layout.cpp -lcurlpp -lcurl -lncursesw $(GraphicsMagick++-config --cppflags --cxxflags --ldflags --libs)
