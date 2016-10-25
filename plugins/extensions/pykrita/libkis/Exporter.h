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
#ifndef LIBKIS_EXPORTER_H
#define LIBKIS_EXPORTER_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Exporter
 */
class KRITALIBKIS_EXPORT Exporter : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Exporter)
    
    Q_PROPERTY(Document* Document READ document WRITE setDocument)
    Q_PROPERTY(InfoObject* ExportSetttings READ exportSetttings WRITE setExportSetttings)

public:
    explicit Exporter(QObject *parent = 0);
    virtual ~Exporter();

    Document* document() const;
    void setDocument(Document* value);

    InfoObject* exportSetttings() const;
    void setExportSetttings(InfoObject* value);



public Q_SLOTS:
    
    bool Export(const QString &filename);


    
Q_SIGNALS:



private:
    struct Private;
    const Private *const d;

};

#endif // LIBKIS_EXPORTER_H
