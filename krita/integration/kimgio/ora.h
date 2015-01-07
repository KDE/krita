/* This file is part of the KDE project
   Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#ifndef KIMG_ORA_H
#define KIMG_ORA_H

#include <QImageIOPlugin>

class OraHandler : public QImageIOHandler
{
public:
    OraHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

#endif

