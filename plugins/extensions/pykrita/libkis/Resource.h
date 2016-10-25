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

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Resource
 */
class KRITALIBKIS_EXPORT Resource : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Resource)
    
    Q_PROPERTY(QString Type READ type WRITE setType)
    Q_PROPERTY(QString Name READ name WRITE setName)
    Q_PROPERTY(QString Filename READ filename WRITE setFilename)

public:
    explicit Resource(QObject *parent = 0);
    virtual ~Resource();

    QString type() const;
    void setType(QString value);

    QString name() const;
    void setName(QString value);

    QString filename() const;
    void setFilename(QString value);



public Q_SLOTS:
    

    
Q_SIGNALS:



private:
    struct Private;
    const Private *const d;

};

#endif // LIBKIS_RESOURCE_H
