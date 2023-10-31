#-------------------------------------------------
#
# Project created by QtCreator 2023-08-17T15:02:12
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

TARGET = uart1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qcustomplot.cpp \
    plot.cpp

HEADERS  += mainwindow.h \
    qcustomplot.h \
    plot.h

FORMS    += mainwindow.ui
