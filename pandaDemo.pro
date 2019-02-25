TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


INCLUDEPATH += /usr/include/python2.7
INCLUDEPATH += /usr/include/panda3d
INCLUDEPATH += /usr/include/eigen3


LIBS += -L/usr/lib/x86_64-linux-gnu/panda3d


LIBS += -lp3framework
LIBS += -lpanda
LIBS += -lpandafx
LIBS += -lpandaexpress
LIBS += -lp3dtoolconfig
LIBS += -lp3dtool
LIBS += -lp3pystub
LIBS += -lp3direct
LIBS += -lpthread


SOURCES += main.cpp
