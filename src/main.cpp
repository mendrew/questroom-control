#include "mainwindow.hpp"
#include <QApplication>

#include <QLocale>
#include <QTextCodec>
#include <QTranslator>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    QTextCodec::setCodecForTr (QTextCodec::codecForName ("UTF-8"));
    QTextCodec::setCodecForCStrings (QTextCodec::codecForName ("UTF-8"));
#endif
    QTextCodec::setCodecForLocale (QTextCodec::codecForName ("UTF-8"));

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
