QT += testlib
QT -= gui

CONFIG += qt console warn_on testcase link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += lua

TEMPLATE = app

INCLUDEPATH += ../../src
LIBS += -L../../src -lqtlua

SOURCES += tst_coroutines.cc
