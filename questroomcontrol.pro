#-------------------------------------------------
#
# Project created by QtCreator 2016-03-24T11:18:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = questroomcontrol
TEMPLATE = app


SOURCES += \
    src/main.cpp \
    src/qrc_connection.cpp \
    src/qrc_device.cpp \
    src/qrc_protocol.cpp \
    src/mainwindow.cpp \
    src/qrc_ledmodel.cpp \
    src/qrc_smartledmodel.cpp

HEADERS  += \
    src/qrc_connection.hpp \
    src/qrc_device.hpp \
    src/qrc_protocol.hpp \
    src/mainwindow.hpp \
    src/qrc_ledmodel.hpp \
    src/qrc_smartledmodel.hpp

FORMS    += \
    src/mainwindow.ui

equals(QT_MAJOR_VERSION, 4) {
    CONFIG += serialport # QT 4
}
greaterThan(QT_MAJOR_VERSION, 4) {
    QT += serialport #QT5
}

QMAKE_CXXFLAGS += -std=c++11

QMAKE_LFLAGS_RELEASE += -static -static-libgcc
