#include "mainwindow.h"

#include <QApplication>

#undef main

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    if(!w.initializeSDL())
        qDebug() << "SDL connot be initialized !";

    w.show();
    return a.exec();
}
