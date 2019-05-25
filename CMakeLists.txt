cmake_minimum_required(VERSION 3.2)
project(mykvs)

set(MYCLIB_SOURCES src/myclib/linkedlist.c  src/myclib/chlist.c src/myclib/chlist.h src/myclib/trie.c src/myclib/trie.h src/myclib/myclib.h src/pal/pal_linux.c src/myclib/stringutil.c src/myclib/stringutil.h src/myclib/hashtable.c src/myclib/hashtable.h src/myclib/dictionaryitem.c src/myclib/dictionaryitem.h)

set(OTHER_SOURCES src/kvs.c src/kvs.h src/connection.c src/connection.h src/kvsthread.c src/kvsthread.h src/utils.c src/utils.h src/timerthread.c src/timerthread.h)
set(SOURCE_FILES src/main.c ${MYCLIB_SOURCES} ${OTHER_SOURCES})

add_executable(mykvs ${SOURCE_FILES})

target_link_libraries(mykvs pthread)

set_property(TARGET mykvs PROPERTY C_STANDARD 99)

enable_testing ()

set(TEST_FILES test/testmain.c test/myclib/test_linkedlist.h test/myclib/test_chlist.h test/myclib/test_trie.h test/myclib/test_stringutil.h test/myclib/test_hashtable.h test/test_timerthread.h)

include_directories(src)

link_directories(/usr/lib)

add_executable(testmain ${MYCLIB_SOURCES} ${TEST_FILES} ${OTHER_SOURCES})

target_link_libraries(testmain cunit pthread)

set_property(TARGET testmain PROPERTY C_STANDARD 99)