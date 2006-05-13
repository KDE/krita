/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_RESOURCE_H_
#define KIS_RESOURCE_H_

#include <QImage>
#include <QObject>
#include <QString>
#include <krita_export.h>

/**
 * The KisResource class provides a representation of Krita image resources.  This
 * includes, but not limited to, brushes and patterns.
 *
 * This replaces the KisKrayon facility that used to be present in Krayon.
 */
class KRITAIMAGE_EXPORT KisResource : public QObject {
    typedef QObject super;
    Q_OBJECT

public:

    /**
     * Creates a new KisResource object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    KisResource(const QString& filename);
    virtual ~KisResource();

public:
    /**
     * Load this resource.
     */
    virtual bool load() = 0;

    /**
     * Save this resource asynchronously.  The signal saveComplete is emitted when
     * the resource has been saved.
     */
    virtual bool save() = 0;

    /**
     * Returns a QImage representing this resource.  This image could be null.
     */
    virtual QImage img() = 0;

public:
    QString filename() const;
    void setFilename(const QString& filename);
    QString name() const;
    void setName(const QString& name);
    bool valid() const;
    void setValid(bool valid);

private:
    KisResource(const KisResource&);
    KisResource& operator=(const KisResource&);

private:
    QString m_name;
    QString m_filename;
    bool m_valid;
};

#endif // KIS_RESOURCE_H_

