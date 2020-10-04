#include "mainwindow.h"
#include "webradioapplication.h"

int main(int argc, char *argv[])
{
    webradioapplication a(argc, argv);
    MainWindow w(QRect(0, 0, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT));
    w.show();

    return a.exec();
}
