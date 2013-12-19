/* This file is part of the KDE project
   Copyright (C) 2013 Boudewijn Rempt <boud@valdyas.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This code is based on Thacher Ulrich PSD loading code released
   on public domain. See: http://tulrich.com/geekstuff/
*/

#include "kra.h"

#include <QImage>

#include <kdebug.h>

#include <kis_doc2.h>
#include <kis_image.h>


KraHandler::KraHandler()
{
}

bool KraHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("kra");
        return true;
    }
    return false;
}

bool KraHandler::read(QImage *image)
{
    KisDoc2 doc;
    bool retval = doc.load(device());
    if (!retval) return false;

    *image = doc.image()->projection()->convertToQImage(0);
    return true;
}

bool KraHandler::write(const QImage &)
{
    // TODO Stub!
    return false;
}

QByteArray KraHandler::name() const
{
    return "kra";
}

bool KraHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("KraHandler::canRead() called with no device");
        return false;
    }
    return true;
}


class KraPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList KraPlugin::keys() const
{
    return QStringList() << "kra" << "KRA";
}

QImageIOPlugin::Capabilities KraPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "kra" || format == "KRA")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && KraHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *KraPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new KraHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(KraPlugin)
Q_EXPORT_PLUGIN2(Kra, KraPlugin)
