#include "KoFileDialogTester.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KoFileDialogTester w;
    w.show();
    
    return a.exec();
}
