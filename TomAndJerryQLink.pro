QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD/include

SOURCES += \
    src/archive.cpp \
    src/archivedialog.cpp \
    src/block.cpp \
    src/game.cpp \
    src/gamehelp.cpp \
    src/main.cpp \
    src/mainscene.cpp \
    src/modechoosescene.cpp \
    src/objects.cpp \
    src/pausedialog.cpp \
    src/player.cpp \
    src/prop.cpp \
    src/resultdialog.cpp \
    src/settingdialog.cpp

HEADERS += \
    include/archive.h \
    include/archivedialog.h \
    include/block.h \
    include/game.h \
    include/gamehelp.h \
    include/mainscene.h \
    include/modechoosescene.h \
    include/objects.h \
    include/pausedialog.h \
    include/player.h \
    include/prop.h \
    include/resultdialog.h \
    include/settingdialog.h

FORMS += \
    ui/archive.ui \
    ui/archivedialog.ui \
    ui/gamehelp.ui \
    ui/mainscene.ui \
    ui/modechoosescene.ui \
    ui/pausedialog.ui \
    ui/resultdialog.ui \
    ui/settingdialog.ui

TRANSLATIONS += \
    translation/TomAndJerryQLink_en_US.ts \
    translation/TomAndJerryQLink_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
