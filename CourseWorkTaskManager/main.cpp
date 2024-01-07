#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QIcon icon(":/icons/images/appImage2.png");
    int iconSize = 100; // Размер иконки
    a.setWindowIcon(icon.pixmap(iconSize, iconSize));
    MainWindow taskManager;
    taskManager.show();
    return a.exec();
}
