#-------------------------------------------------
#
# Project created by QtCreator 2014-10-08T08:11:05
#
#-------------------------------------------------

QT          +=  core network sql websockets

QT          -=  gui

TARGET      =   Centini
CONFIG      +=  console
CONFIG      -=  app_bundle

TEMPLATE    =   app

SOURCES     +=  main.cpp \
                asteriskmanager.cpp \
				centiniserver.cpp \
				desktopuser.cpp \
				user.cpp \
    webuser.cpp \
    callbackquery.cpp

HEADERS     +=  asteriskmanager.h \
				centiniserver.h \
				desktopuser.h \
				user.h \
    webuser.h \
    callbackquery.h
