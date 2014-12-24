include_directories(${inc})
include_directories($ENV{EIGEN3_INCLUDE_DIR})
message("Eigen path: $ENV{EIGEN3_INCLUDE_DIR}")

include_directories($ENV{PNG_INCLUDE_DIR})
message("PNG path: $ENV{PNG_INCLUDE_DIR}")

set(xform_src
    main.cpp
    XImage.cpp
    Warp.cpp
    Recipe.cpp
    util.cpp
    TransformModel.cpp
)

add_executable( xform
    ${xform_src}
)


target_link_libraries(xform
    ${PNG_LIBRARIES}
)
