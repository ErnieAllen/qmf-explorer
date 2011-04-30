#-------------------------------------------------
#
# Project created by QtCreator 2011-04-29T19:26:41
#
#-------------------------------------------------

QT       += core gui

TARGET = qmfe
TEMPLATE = app


SOURCES += main.cpp\
    agent-detail-model.cpp \
    agent-model.cpp \
    object-detail-model.cpp \
    object-model.cpp \
    qmf-thread.cpp \
    opendialog.cpp

HEADERS  += \
    agent-detail-model.h \
    agent-model.h \
    main.h \
    object-detail-model.h \
    object-model.h \
    qmf-thread.h \
    opendialog.h

FORMS    += \
    explorer_main.ui \
    opendialog.ui

OTHER_FILES += \
    license.txt
