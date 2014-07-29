#ifndef LIBKIS_NODE_H
#define LIBKIS_NODE_H

#include <QObject>

#include <krita_export.h>

class LIBKIS_EXPORT Node : public QObject
{
    Q_OBJECT
public:
    explicit Node(QObject *parent = 0);

signals:

public slots:

};

#endif // LIBKIS_NODE_H
