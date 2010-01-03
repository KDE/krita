/*  This file is part of the KDE project
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 */
#ifndef KOCOLORSET
#define KOCOLORSET

#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtCore/QVector>
#include <QtGui/QPixmap>

#include "KoResource.h"
#include "KoColor.h"

class QPixmap;

struct KoColorSetEntry {
    KoColor color;
    QString name;
    bool operator==(const KoColorSetEntry& rhs) const {
        //FIXME return color == rhs.color && name == rhs.name;
        return name == rhs.name;
    }
};

/**
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class PIGMENTCMS_EXPORT KoColorSet : public KoResource
{
    typedef KoResource super;

public:
    /**
     * Load a color set from a file. This can be a Gimp
     * palette, a RIFF palette or a Photoshop palette.
     */
    KoColorSet(const QString& filename);

    /// Create an empty color set
    KoColorSet();

    /// Explicit copy constructor (KoResource copy constructor is private)
    KoColorSet(const KoColorSet& rhs);

    virtual ~KoColorSet();

    virtual bool load();
    virtual bool save();

public:

    void add(const KoColorSetEntry &);
    void remove(const KoColorSetEntry &);
    KoColorSetEntry getColor(quint32 index);
    qint32 nColors();

private:
    bool init();

private:

    QByteArray m_data;
    bool m_ownData;
    QString m_name;
    QString m_comment;
    qint32 m_columns;
    QVector<KoColorSetEntry> m_colors;

};
#endif // KOCOLORSET

