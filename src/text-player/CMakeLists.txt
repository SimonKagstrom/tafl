add_executable(text-player
    main.cpp
)

target_link_libraries(text-player
PRIVATE
    tafl_release
    fmt::fmt
)
