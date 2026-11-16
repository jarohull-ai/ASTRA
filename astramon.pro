QT += core gui qml quick quickcontrols2
CONFIG += c++17 release
TARGET = astramon
SOURCES += src/main.cpp src/backend.cpp src/translator.cpp
HEADERS += src/backend.h src/translator.h
RESOURCES += src/resources.qrc
