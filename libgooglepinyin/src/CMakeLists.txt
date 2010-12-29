set(GOOGLEPINYIN_SRC
	dictbuilder.cpp
	dictlist.cpp
	dicttrie.cpp
	lpicache.cpp
	matrixsearch.cpp
	mystdlib.cpp
	ngram.cpp
	pinyinime.cpp
	searchutility.cpp
	spellingtable.cpp
	spellingtrie.cpp
	splparser.cpp
	sync.cpp
	userdict.cpp
	utf16char.cpp
	utf16reader.cpp
)

add_library(googlepinyin SHARED ${GOOGLEPINYIN_SRC})

target_link_libraries(googlepinyin pthread )