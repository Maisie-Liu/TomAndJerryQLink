QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    archive.cpp \
    archivedialog.cpp \
    block.cpp \
    game.cpp \
    gamehelp.cpp \
    main.cpp \
    mainscene.cpp \
    modechoosescene.cpp \
    objects.cpp \
    pausedialog.cpp \
    player.cpp \
    prop.cpp \
    resultdialog.cpp \
    settingdialog.cpp

HEADERS += \
    archive.h \
    archivedialog.h \
    block.h \
    game.h \
    gamehelp.h \
    mainscene.h \
    modechoosescene.h \
    objects.h \
    pausedialog.h \
    player.h \
    prop.h \
    resultdialog.h \
    settingdialog.h

FORMS += \
    archive.ui \
    archivedialog.ui \
    gamehelp.ui \
    mainscene.ui \
    modechoosescene.ui \
    pausedialog.ui \
    resultdialog.ui \
    settingdialog.ui

TRANSLATIONS += \
    TomAndJerryQLink_en_US.ts \
    TomAndJerryQLink_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
