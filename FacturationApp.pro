QT       += core gui sql widgets network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

SOURCES += \
    dialogs/logindialog.cpp \
    main.cpp \
    mainwindow.cpp \
    database/database.cpp \
    models/client.cpp \
    dialogs/registerdialog.cpp \
    utils/emailsender.cpp \
    views/clientwindow.cpp\
     views/adminwindow.cpp \
    dialogs/client_edit_dialog.cpp
HEADERS += \
    dialogs/logindialog.h \
    mainwindow.h \
    database/database.h \
    models/client.h \
   dialogs/registerdialog.h \
    utils/emailsender.h \
    views/clientwindow.h\
     views/adminwindow.h \
    dialogs/client_edit_dialog.h

FORMS += \
    mainwindow.ui