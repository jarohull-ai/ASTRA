QT += core gui qml quick quickcontrols2 network
CONFIG += c++17 release
TARGET = astramon
SOURCES += src/main.cpp src/backend.cpp src/translator.cpp src/paradox.cpp
HEADERS += src/backend.h src/translator.h src/paradox.h
RESOURCES += src/resources.qrc
