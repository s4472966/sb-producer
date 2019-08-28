SOURCES += \
    main.cpp


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../librdkafka-master/src/release/ -lrdkafka
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../librdkafka-master/src/debug/ -lrdkafka
else:unix: LIBS += -L$$PWD/../../librdkafka-master/src/ -lrdkafka

INCLUDEPATH += $$PWD/../../librdkafka-master/src
DEPENDPATH += $$PWD/../../librdkafka-master/src

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../cpp_redis/build/lib/release/ -lcpp_redis
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../cpp_redis/build/lib/debug/ -lcpp_redis
else:unix: LIBS += -L$$PWD/../cpp_redis/build/lib/ -lcpp_redis

INCLUDEPATH += $$PWD/../cpp_redis/build
DEPENDPATH += $$PWD/../cpp_redis/build

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../cpp_redis/build/lib/release/libcpp_redis.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../cpp_redis/build/lib/debug/libcpp_redis.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../cpp_redis/build/lib/release/cpp_redis.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../cpp_redis/build/lib/debug/cpp_redis.lib
else:unix: PRE_TARGETDEPS += $$PWD/../cpp_redis/build/lib/libcpp_redis.a

unix:!macx: LIBS += -L$$PWD/../cpp_redis/tacopie/lib/ -ltacopie

INCLUDEPATH += $$PWD/../cpp_redis/tacopie
DEPENDPATH += $$PWD/../cpp_redis/tacopie

unix:!macx: PRE_TARGETDEPS += $$PWD/../cpp_redis/tacopie/lib/libtacopie.a

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../sb-loader/sb_loader/lib/release/ -lSBReadFile
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../sb-loader/sb_loader/lib/debug/ -lSBReadFile
else:unix: LIBS += -L$$PWD/../sb-loader/sb_loader/lib/ -lSBReadFile

INCLUDEPATH += $$PWD/../sb-loader/sb_loader
DEPENDPATH += $$PWD/../sb-loader/sb_loader

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../sb-loader/sb_loader/lib/release/ -lSlideBook6Reader
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../sb-loader/sb_loader/lib/debug/ -lSlideBook6Reader
else:unix: LIBS += -L$$PWD/../sb-loader/sb_loader/lib/ -lSlideBook6Reader

INCLUDEPATH += $$PWD/../sb-loader/sb_loader
DEPENDPATH += $$PWD/../sb-loader/sb_loader


HEADERS += \
    ../sb-loader/fmt/include/fmt/core.h \
    ../sb-loader/fmt/include/fmt/format-inl.h \
    ../sb-loader/fmt/include/fmt/format.h \
    ../sb-loader/fmt/include/fmt/ostream.h \
    ../sb-loader/fmt/include/fmt/posix.h \
    ../sb-loader/fmt/include/fmt/printf.h \
    ../sb-loader/fmt/include/fmt/ranges.h \
    ../sb-loader/fmt/include/fmt/time.h \
    ../sb-loader/sb_loader/lib/SBReadFile.h \
    ../sb-loader/sb_loader/src/sb_loader.h \
    ../sb-loader/util/include/phoebe_util/util.h

unix|win32: LIBS += -luuid
