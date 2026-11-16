QT += core gui qml quick quickcontrols2 testlib
CONFIG += c++17 testcase
TARGET = astramon-tests
SOURCES += tests.cpp ../src/backend.cpp
HEADERS += ../src/backend.h
INCLUDEPATH += ../src
RESOURCES += ../src/resources.qrc
