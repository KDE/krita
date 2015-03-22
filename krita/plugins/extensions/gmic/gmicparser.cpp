#include <QApplication>
#include <kis_gmic_parser.h>
#include <Command.h>
#include <kis_gmic_filter_model.h>
#include <QDebug>
#include <kis_gmic_widget.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QStringList definitions;
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            definitions << QString::fromUtf8(argv[i]);
        }

    }
    else
    {
        definitions << "~/.kde/share/apps/krita/gmic/gmic_def.gmic";
    }

    qDebug() << definitions;


    KisGmicParser parser(definitions);
    Component * root = parser.createFilterTree();

    KisGmicFilterModel * model = new KisGmicFilterModel(root);

    KisGmicWidget gmicWidget(model);
    gmicWidget.show();

    return app.exec();
 }
