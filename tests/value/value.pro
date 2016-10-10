QT += testlib
QT -= gui

CONFIG += qt console testcase link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += lua

TEMPLATE = app

INCLUDEPATH += ../../src
LIBS += -L../../src -lqtlua

SOURCES += tst_value.cc
