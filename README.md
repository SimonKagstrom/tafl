Tafl game client (someday)

## Building

Dependencies are fetched by CMake (FetchContent) at configure time:

```
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build ut auto-player text-player
```
