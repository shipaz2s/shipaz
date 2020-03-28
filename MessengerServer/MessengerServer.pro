TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG += qt
QT -= gui
QT += network core sql

SOURCES += \
        main.cpp \
        serv.cpp

HEADERS += \
    serv.h
