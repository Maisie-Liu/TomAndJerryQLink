#include "mainscene.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator * translator = new QTranslator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "TomAndJerryQLink_" + QLocale(locale).name();
        if (translator->load(":/i18n/" + baseName)) {
            a.installTranslator(translator);
            break;
        }
    }

    MainScene w(nullptr,translator);
    w.show();

    return a.exec();
}
