#include <QApplication>

#include "MyWidget.h"

int main(int argc, char *argv[])
{
  	QApplication app(argc, argv);
    MyWidget widget;
	widget.show();
    return app.exec();
}

