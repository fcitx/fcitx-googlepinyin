set(DICTBUILDER_SRCS
	pinyinime_dictbuilder.cpp
)

add_executable(dictbuilder ${DICTBUILDER_SRCS})
target_link_libraries(dictbuilder googlepinyin)

