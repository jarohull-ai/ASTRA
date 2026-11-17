QT += core gui qml quick quickcontrols2 network testlib
CONFIG += c++17 testcase
TARGET = astramon-tests
SOURCES += tests.cpp ../src/backend.cpp ../src/paradox.cpp
HEADERS += ../src/backend.h ../src/paradox.h
INCLUDEPATH += ../src
RESOURCES += ../src/resources.qrc
