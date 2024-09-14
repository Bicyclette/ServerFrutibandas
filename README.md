# ServerFrutibandas

<p align=center style="font-size:150%;">
C++ server for Frutibandas.<br><br>
</p>

<p align="center">
<img alt="command_line_server" src="imgs/server.png">
</p>

## Getting started

### Dependencies

- [Conan](https://conan.io/)
- [Cmake](https://cmake.org/)

## Build

```
conan install . -s build_type=Release --build missing -of=build
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

## Commands

- PC = print the number of players connected
- GC = print the number of ongoing games
- Q  = shtudown the server
