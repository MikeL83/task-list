#-------------------------------------------------
#
# Project created by QtCreator 2014-04-05T10:25:29
#
#-------------------------------------------------

QT       += core gui sql

CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TaskList
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tasksdb.cpp \
    userinputdialog.cpp \
    taskinputdialog.cpp \
    reminderdialog.cpp

HEADERS  += mainwindow.h \
    tasksdb.h \
    userinputdialog.h \
    taskinputdialog.h \
    reminderdialog.h

FORMS    += mainwindow.ui

RESOURCES += \
    MyResource.qrc
