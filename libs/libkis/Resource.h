/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_RESOURCE_H
#define LIBKIS_RESOURCE_H

#include <QObject>
#include <kis_types.h>
#include "kritalibkis_export.h"
#include "libkis.h"


class KoResource;

/**
 * Resource
 */
class KRITALIBKIS_EXPORT Resource : public QObject
{
    Q_OBJECT

public:
    explicit Resource(KoResource *resource, QObject *parent = 0);
    virtual ~Resource();

public Q_SLOTS:
    
    QString type() const;

    QString name() const;
    void setName(QString value);

    QString filename() const;

    QImage image() const;
    void setImage(QImage image);

    QByteArray data() const;
    bool setData(QByteArray data);

private:
    struct Private;
    const Private *const d;

};

#endif // LIBKIS_RESOURCE_H
