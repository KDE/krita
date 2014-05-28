#include <QApplication>
#include <kis_gmic_parser.h>
#include <Command.h>
#include <kis_gmic_filter_model.h>
#include <kis_gmic_widget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList definitions;
    definitions << "gmic_def.gmic";

    KisGmicParser parser(definitions);
    Component * root = parser.createFilterTree();

    KisGmicFilterModel * model = new KisGmicFilterModel(root);

    KisGmicWidget gmicWidget(model);
    gmicWidget.show();

    return app.exec();
 }
