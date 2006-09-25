/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_PALETTE_
#define KIS_PALETTE_

#include <QImage>
#include <QColor>
#include <QVector>
#include <QPixmap>

#include <kio/job.h>
#include <kpalette.h>

#include "kis_types.h"
#include "kis_resource.h"
#include "kis_global.h"
#include "kis_gradient.h"
#include "kis_alpha_mask.h"

class QPoint;
class QPixmap;
class KisPaintDevice;

struct KisPaletteEntry {
    QColor color;
    QString name;
    bool operator==(const KisPaletteEntry& rhs) const {
        return color == rhs.color && name == rhs.name;
    }
};

/**
 * Open Gimp, Photoshop or RIFF palette files. This is a straight port
 * from the Gimp.
 */
class KRITAIMAGE_EXPORT KisPalette : public KisResource {
    typedef KisResource super;

    Q_OBJECT

public:
    /**
     * Create a palette from the colors in an image
     */
    KisPalette(const QImage * img, qint32 nColors, const QString & name);

    /**
     * Create a palette from the colors in a paint device
     */
    KisPalette(const KisPaintDeviceSP device, qint32 nColors, const QString & name);

    /**
     * Create a palette from the colors in a gradient
     */
    KisPalette(const KisGradient * gradient, qint32 nColors, const QString & name);

    /**
     * Load a palette from a file. This can be a Gimp
     * palette, a RIFF palette or a Photoshop palette.
     */
    KisPalette(const QString& filename);

    /// Create an empty palette
    KisPalette();

    /// Explicit copy constructor (KisResource copy constructor is private)
    KisPalette(const KisPalette& rhs);

    virtual ~KisPalette();

    virtual bool load();
    virtual bool save();
    virtual QImage img();


public:

    void add(const KisPaletteEntry &);
    void remove(const KisPaletteEntry &);
    KisPaletteEntry getColor(quint32 index);
    qint32 nColors();

private:
    bool init();

private:

    QByteArray m_data;
    bool m_ownData;
    QImage m_img;
    QString m_name;
    QString m_comment;
    qint32 m_columns;
    QVector<KisPaletteEntry> m_colors;

};
#endif // KIS_PALETTE_

