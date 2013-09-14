#include <QApplication>
#include <kis_gmic_parser.h>
#include <Command.h>
#include <kis_gmic_filter_model.h>
#include <kis_gmic_widget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KisGmicParser parser("gmic_def.gmic");
    Component * root = parser.createFilterTree();

    KisGmicFilterModel model(root);

    KisGmicWidget gmicWidget(&model);
    gmicWidget.show();

    return app.exec();
 }
