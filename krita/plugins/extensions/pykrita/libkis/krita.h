#ifndef LIBKIS_KRITA_H
#define LIBKIS_KRITA_H

#include <QObject>
#include <QPointer>

#include "application.h"
#include "mainwindow.h"
#include "view.h"
#include "document.h"
#include "image.h"

#include <krita_export.h>

class LIBKIS_EXPORT Krita : public QObject
{
    Q_OBJECT
public:
    explicit Krita(QObject *parent = 0);

    QList<MainWindow*> mainWindows();
    QList<View*> views();
    QList<Document*> documents();
    QList<Image*> images();

signals:

public slots:

private:
};

#endif // LIBKIS_KRITA_H
