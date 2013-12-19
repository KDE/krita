/* This file is part of the KDE project
   Copyright (C) 2013 Boudewijn Rempt <boud@valdyas.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This code is based on Thacher Ulrich PSD loading code released
   on public domain. See: http://tulrich.com/geekstuff/
*/

#include "ora.h"

#include <QImage>

#include <KoStore.h>

#include <kis_image.h>
#include <kis_doc2.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_open_raster_stack_load_visitor.h>
#include "ora_load_context.h"
#include <kis_paint_layer.h>

#include <kdebug.h>

OraHandler::OraHandler()
{
}

bool OraHandler::canRead() const
{
    if (canRead(device())) {
        setFormat("ora");
        return true;
    }
    return false;
}

bool OraHandler::read(QImage *image)
{
    KoStore *store = KoStore::createStore(device(), KoStore::Read, "image/openraster", KoStore::Zip);
    if (!store || store->bad()) {
        delete store;
        return false;
    }
    store->disallowNameExpansion();
    KisDoc2 doc;
    OraLoadContext olc(store);
    KisOpenRasterStackLoadVisitor orslv(&doc, &olc);
    orslv.loadImage();
    KisImageWSP img = orslv.image();
    img->initialRefreshGraph();
    *image = img->projection()->convertToQImage(0);

    delete store;
    return true;
}

bool OraHandler::write(const QImage &)
{
    // TODO Stub!
    return false;
}

QByteArray OraHandler::name() const
{
    return "ora";
}

bool OraHandler::canRead(QIODevice *device)
{
    if (!device) {
        qWarning("OraHandler::canRead() called with no device");
        return false;
    }
    return true;
}


class OraPlugin : public QImageIOPlugin
{
public:
    QStringList keys() const;
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const;
};

QStringList OraPlugin::keys() const
{
    return QStringList() << "ora" << "ORA";
}

QImageIOPlugin::Capabilities OraPlugin::capabilities(QIODevice *device, const QByteArray &format) const
{
    if (format == "ora" || format == "ORA")
        return Capabilities(CanRead);
    if (!format.isEmpty())
        return 0;
    if (!device->isOpen())
        return 0;

    Capabilities cap;
    if (device->isReadable() && OraHandler::canRead(device))
        cap |= CanRead;
    return cap;
}

QImageIOHandler *OraPlugin::create(QIODevice *device, const QByteArray &format) const
{
    QImageIOHandler *handler = new OraHandler;
    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

Q_EXPORT_STATIC_PLUGIN(OraPlugin)
Q_EXPORT_PLUGIN2(Ora, OraPlugin)
