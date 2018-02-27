/*
 * Copyright (C) 2009 Shawn T. Rutledge (shawn.t.rutledge@gmail.com)
 * Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 *
 */
#ifndef QGIFLIBHANDLER_H
#define QGIFLIBHANDLER_H

#include <QImageIOHandler>
#include <QImage>
#include <QVariant>

class QGIFLibHandler : public QImageIOHandler
{
public:
    QGIFLibHandler();
    bool canRead () const;
    bool read ( QImage * image );
    bool write ( const QImage & image );
    static bool canRead(QIODevice *device);
    bool supportsOption ( ImageOption option ) const;
    void setOption ( ImageOption option, const QVariant & value );
    QVariant option( ImageOption option ) const;

private:
    QString m_description;
};

#endif // QGIFLIBHANDLER_H
