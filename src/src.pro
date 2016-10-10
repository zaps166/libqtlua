QT += core gui widgets

TEMPLATE = lib
TARGET = qtlua

SOURCES +=                                 \
    qtluadispatchproxy.cc                  \
    qtluaenum.cc                           \
    qtluaenumiterator.cc                   \
    qtluafunction.cc                       \
    qtluamember.cc                         \
    qtluametacache.cc                      \
    qtluamethod.cc                         \
    qtluapixmap.cc                         \
    qtluaproperty.cc                       \
    qtluaqmetaobjecttable.cc               \
    qtluaqmetaobjectwrapper.cc             \
    qtluaqmetavalue.cc                     \
    qtluaqobjectiterator.cc                \
    qtluaqobjectwrapper.cc                 \
    qtluaqtlib.cc                          \
    qtluastate.cc                          \
    qtluatableiterator.cc                  \
    qtluauserdata.cc                       \
    qtluavalue.cc                          \
    qtluavaluebase.cc                      \
    qtluavalueref.cc
HEADERS +=                                 \
    internal/qtluaenum.hh                  \
    internal/qtluaenum.hxx                 \
    internal/qtluaenumiterator.hh          \
    internal/qtluamember.hh                \
    internal/qtluamember.hxx               \
    internal/qtluametacache.hh             \
    internal/qtluametacache.hxx            \
    internal/qtluamethod.hh                \
    internal/qtluamethod.hxx               \
    internal/qtluapoolarray.hh             \
    internal/qtluaproperty.hh              \
    internal/qtluaproperty.hxx             \
    internal/qtluaqmetaobjectwrapper.hh    \
    internal/qtluaqmetavalue.hh            \
    internal/qtluaqmetavalue.hxx           \
    internal/qtluaqobjectiterator.hh       \
    internal/qtluaqobjectwrapper.hh        \
    internal/qtluaqobjectwrapper.hxx       \
    internal/qtluaqtlib.hh                 \
    internal/qtluatableiterator.hh         \
                                           \
    QtLua/qtluaarrayproxy.hh               \
    QtLua/qtluaarrayproxy.hxx              \
    QtLua/qtluadispatchproxy.hh            \
    QtLua/qtluadispatchproxy.hxx           \
    QtLua/qtluafunction.hh                 \
    QtLua/qtluafunction.hxx                \
    QtLua/qtluaiterator.hh                 \
    QtLua/qtluaiterator.hxx                \
    QtLua/qtluametatype.hh                 \
    QtLua/qtluametatype.hxx                \
    QtLua/qtluapixmap.hh                   \
    QtLua/qtluaqhashproxy.hh               \
    QtLua/qtluaqhashproxy.hxx              \
    QtLua/qtluaqlinkedlistproxy.hh         \
    QtLua/qtluaqlinkedlistproxy.hxx        \
    QtLua/qtluaqlistproxy.hh               \
    QtLua/qtluaqlistproxy.hxx              \
    QtLua/qtluaqvectorproxy.hh             \
    QtLua/qtluaqvectorproxy.hxx            \
    QtLua/qtluaref.hh                      \
    QtLua/qtluastate.hh                    \
    QtLua/qtluastate.hxx                   \
    QtLua/qtluastring.hh                   \
    QtLua/qtluastring.hxx                  \
    QtLua/qtluauserdata.hh                 \
    QtLua/qtluauserdata.hxx                \
    QtLua/qtluauserobject.hh               \
    QtLua/qtluauserobject.hxx              \
    QtLua/qtluavalue.hh                    \
    QtLua/qtluavalue.hxx                   \
    QtLua/qtluavaluebase.hh                \
    QtLua/qtluavaluebase.hxx               \
    QtLua/qtluavalueref.hh                 \
    QtLua/qtluavalueref.hxx
