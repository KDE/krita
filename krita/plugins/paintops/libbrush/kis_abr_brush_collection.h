/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  Copyright (c) 2007 Eric Lamarque <eric.lamarque@free.fr>
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
#ifndef KIS_ABR_BRUSH_COLLECTION_H
#define KIS_ABR_BRUSH_COLLECTION_H

#include <QImage>
#include <QVector>
#include <QDataStream>
#include <QString>

#include <kis_brush.h>

#include "kis_types.h"
#include "kis_shared.h"
#include "kis_paint_information.h"

class QString;
class QPoint;
class QIODevice;

class KoColor;
class KoColorSpace;

class KisAbrBrush;
struct AbrInfo;

/**
 * load a collection of brushes from an abr file
 */
class BRUSH_EXPORT KisAbrBrushCollection : public KisBrush
{

protected:

public:

    /// Construct brush to load filename later as brush
    KisAbrBrushCollection(const QString& filename);

    virtual ~KisAbrBrushCollection() {}

    virtual bool load();

    virtual bool save();

    /**
     * @return a preview of the brush
     */
    virtual QImage image() const;

    /**
     * save the content of this brush to an IO device
     */
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * @return default file extension for saving the brush
     */
    virtual QString defaultFileExtension() const;

    QVector<KisAbrBrush*> brushes() { return m_abrBrushes; }
    
protected:

    void toXML(QDomDocument& d, QDomElement& e) const;

private:

    qint32 abr_brush_load (QDataStream & abr, AbrInfo *abr_hdr,const QString filename, qint32 image_ID, qint32 id);
    qint32 abr_brush_load_v12 (QDataStream & abr, AbrInfo *abr_hdr,const QString filename, qint32 image_ID, qint32 id);
    quint32 abr_brush_load_v6 (QDataStream & abr, AbrInfo *abr_hdr, const QString filename, qint32 image_ID, qint32 id);
    QVector<KisAbrBrush*> m_abrBrushes;
};

#endif

