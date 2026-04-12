QT       += core gui sql widgets network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    database/database.cpp \
    models/client.cpp \
    dialogs/registerdialog.cpp \
    utils/emailsender.cpp

HEADERS += \
    mainwindow.h \
    database/database.h \
    models/client.h \
   dialogs/registerdialog.h \
    registerdialog.h \
    utils/emailsender.h

FORMS += \
    mainwindow.ui