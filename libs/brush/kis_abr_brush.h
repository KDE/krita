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
#ifndef KIS_ABR_BRUSH_
#define KIS_ABR_BRUSH_

#include <QImage>
#include <QVector>

#include <kis_scaling_size_brush.h>
#include <kis_types.h>
#include <kis_shared.h>

#include "kritabrush_export.h"

class KisQImagemask;
class KisAbrBrushCollection;
typedef KisSharedPtr<KisQImagemask> KisQImagemaskSP;

class QString;
class QIODevice;


class BRUSH_EXPORT KisAbrBrush : public KisScalingSizeBrush
{

public:

    /// Construct brush to load filename later as brush
    KisAbrBrush(const QString& filename, KisAbrBrushCollection *parent);
    KisAbrBrush(const KisAbrBrush& rhs);
    KisAbrBrush(const KisAbrBrush& rhs, KisAbrBrushCollection *parent);
    KisBrushSP clone() const override;

    bool load() override;

    bool loadFromDevice(QIODevice *dev) override;

    bool save() override;

    bool saveToDevice(QIODevice* dev) const override;

    /**
     * @return default file extension for saving the brush
     */
    QString defaultFileExtension() const override;

    QImage brushTipImage() const override;

    friend class KisAbrBrushCollection;

    void setBrushTipImage(const QImage& image) override;

    void toXML(QDomDocument& d, QDomElement& e) const override;

private:
    KisAbrBrushCollection *m_parent;
};

typedef QSharedPointer<KisAbrBrush> KisAbrBrushSP;

#endif // KIS_ABR_BRUSH_

