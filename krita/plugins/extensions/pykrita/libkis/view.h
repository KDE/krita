#ifndef LIBKIS_VIEW_H
#define LIBKIS_VIEW_H

#include <QObject>

#include <krita_export.h>

class KisView2;

class LIBKIS_EXPORT View : public QObject
{
    Q_OBJECT
public:
    explicit View(QObject *view, QObject *parent = 0);

signals:

public slots:

private:

    KisView2 *m_view;

};

#endif // LIBKIS_VIEW_H
