#include <QtGui/QApplication>
#include "kis_curve_widget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KisCurveWidget w;
    w.show();

    return a.exec();
}
