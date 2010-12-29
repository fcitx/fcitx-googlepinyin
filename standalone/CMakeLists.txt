set(GPY_STANDALONE_SRCS
	main.cpp
)

include_directories(${PROJECT_SOURCE_DIR}/libgooglepinyin/include)

add_executable(gpy_standalone ${GPY_STANDALONE_SRCS})

target_link_libraries(gpy_standalone googlepinyin)