#include "pipeclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pipeClient w;
    w.show();
    return a.exec();
}
