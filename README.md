# ServerFrutibandas

<p align=center style="font-size:150%;">
C++ server for Frutibandas.<br><br>
</p>

## Getting started

### Dependencies

- [Conan](https://conan.io/)
- [Cmake](https://cmake.org/)

## Build

```
conan install . -s build_type=Release --build missing --install-folder=build
cmake -B build -S .
```
