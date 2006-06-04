/*
 *  kis_pattern.h - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
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

#ifndef __kis_pattern_h__
#define __kis_pattern_h__

#include <kio/job.h>

#include "kis_debug_areas.h"
#include "kis_resource.h"
#include <krita_export.h>
#include "kis_types.h"

class QPoint;
class QImage;
class KoColorSpace;
class KisPaintDevice;

class KRITAIMAGE_EXPORT KisPattern : public KisResource {
    typedef KisResource super;
    Q_OBJECT

public:
    KisPattern(const QString& file);
    KisPattern(KisPaintDevice* image, int x, int y, int w, int h);
    virtual ~KisPattern();

    virtual bool load();
    virtual bool save();
    virtual QImage img();

    /**
     * returns a KisPaintDeviceSP made with colorSpace as the ColorSpace strategy
     * for use in the fill painter.
     **/
    KisPaintDeviceSP image(KoColorSpace * colorSpace);

    qint32 width() const;
    qint32 height() const;

    void setImage(const QImage& img);

    KisPattern* clone() const;

protected:
    void setWidth(qint32 w);
    void setHeight(qint32 h);

private:
    bool init();

private:
    QByteArray m_data;
    QImage m_img;
    QMap<QString, KisPaintDeviceSP> m_colorspaces;
    bool m_hasFile;

    qint32 m_width;
    qint32 m_height;
};

#endif

