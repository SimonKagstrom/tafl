enable_testing()

add_executable(auto-player
    main.cpp
)

target_link_libraries(auto-player
PRIVATE
    tafl_release
    fmt::fmt
)
