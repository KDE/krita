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

#include "kis_brush.h"
#include "kis_types.h"
#include "kis_shared.h"
#include "kis_paint_information.h"

#include "krita_export.h"

class KisQImagemask;
typedef KisSharedPtr<KisQImagemask> KisQImagemaskSP;

class QString;
class QPoint;
class QIODevice;

class KoColor;
class KoColorSpace;

class BRUSH_EXPORT KisAbrBrush : public KisBrush
{

public:

    /// Construct brush to load filename later as brush
    KisAbrBrush(const QString& filename);

    virtual bool load();

    virtual bool save();

    /**
     * save the content of this brush to an IO device
     */
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * @return default file extension for saving the brush
     */
    virtual QString defaultFileExtension() const;

protected:

    friend class KisAbrBrushCollection;

    virtual void setImage(const QImage& image);

    void toXML(QDomDocument& d, QDomElement& e) const;

private:


};

#endif // KIS_ABR_BRUSH_

