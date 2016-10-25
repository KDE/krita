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
#ifndef LIBKIS_IMPORTER_H
#define LIBKIS_IMPORTER_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Importer
 */
class KRITALIBKIS_EXPORT Importer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Importer)
    
    Q_PROPERTY(Document* Document READ document WRITE setDocument)
    Q_PROPERTY(InfoObject* ImportSettings READ importSettings WRITE setImportSettings)

public:
    explicit Importer(QObject *parent = 0);
    virtual ~Importer();

    Document* document() const;
    void setDocument(Document* value);

    InfoObject* importSettings() const;
    void setImportSettings(InfoObject* value);



public Q_SLOTS:
    
    bool Import(const QString &filename);


    
Q_SIGNALS:



private:
    struct Private;
    const Private *const d;

};

#endif // LIBKIS_IMPORTER_H
