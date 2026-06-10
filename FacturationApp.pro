QT += core gui sql widgets network printsupport charts
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += c++17

SOURCES += \
    dialogs/paymentdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    database/database.cpp \
    models/client.cpp \
    dialogs/logindialog.cpp \
    dialogs/registerdialog.cpp \
    dialogs/client_edit_dialog.cpp \
    dialogs/invoicecreatedialog.cpp \
    dialogs/invoiceactiondialog.cpp \
    dialogs/invoicedialog.cpp \
    dialogs/invoiceitemdialog.cpp \
    dialogs/invoiceeditdialog.cpp \
    utils/emailsender.cpp \
    utils/invoicesenderdialog.cpp \
    utils/invoicegenerator.cpp \
    views/clientwindow.cpp \
    views/dashboardwidget.cpp \
    views/adminwindow.cpp \
    views/invoicemanagementwidget.cpp \
    views/articleswidget.cpp \
    dialogs/articleeditdialog.cpp \
    views/entreprise_config_widget.cpp \
    utils/entreprise_config_manager.cpp

HEADERS += \
    mainwindow.h \
    database/database.h \
    models/client.h \
    dialogs/logindialog.h \
    dialogs/registerdialog.h \
    dialogs/client_edit_dialog.h \
    dialogs/invoicecreatedialog.h \
    dialogs/invoiceactiondialog.h \
    dialogs/invoiceeditdialog.h \
    dialogs/invoicedialog.h \
    dialogs/invoiceitemdialog.h \
    models/payment.h \
    utils/emailsender.h \
    utils/invoicehtmlgenerator.h \
    utils/invoicesenderdialog.h \
    utils/invoicegenerator.h \
    views/clientwindow.h \
    views/adminwindow.h \
    views/dashboardwidget.h \
    views/invoicemanagementwidget.h \
    views/articleswidget.h \
    dialogs/articleeditdialog.h \
    dialogs/paymentdialog.h \
    models/entreprise_config.h \
    views/entreprise_config_widget.h \
    utils/entreprise_config_manager.h
FORMS += \
    mainwindow.ui

QMAKE_CXXFLAGS_WARN_ON -= -Werror