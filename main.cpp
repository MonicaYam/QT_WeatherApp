#include "locationlist.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LocationList w;
    w.show();
    return a.exec();
}
