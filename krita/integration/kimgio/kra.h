/* This file is part of the KDE project
   Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIMG_KRA_H
#define KIMG_KRA_H

#include <QImageIOPlugin>

class KraHandler : public QImageIOHandler
{
public:
    KraHandler();

    bool canRead() const Q_DECL_OVERRIDE;
    bool read(QImage *image)  Q_DECL_OVERRIDE;

    static bool canRead(QIODevice *device);
};

class KraPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "kra.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};



#endif

