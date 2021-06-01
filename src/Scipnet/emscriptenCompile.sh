mkdir bin/Emscripten
source ~/Projects/emscripten/emsdk/emsdk_env.sh

em++ --std=c++17 -s TOTAL_MEMORY=256MB --bind -O3 -o bin/Emscripten/scipnet.js -I./ Scipnet/layout/Layout.cpp Scipnet/Cli.cpp Scipnet/ColorTable.cpp Scipnet/Converter.cpp Scipnet/Emscript.cpp
#cp scipnet.html bin/Emscripten/scipnet.html
