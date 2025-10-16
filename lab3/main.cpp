#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QStringLiteral("Обработка изображений"));
    w.showMaximized();
    w.show();
    return a.exec();
}
