/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_IMAGEPIPE_BRUSH_
#define KIS_IMAGEPIPE_BRUSH_

#include <qptrlist.h>
#include <qvaluelist.h>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_brush.h"
#include "kis_global.h"

class QCString;
class QImage;
class QPoint;
class QSize;

class KisImagePipeBrush : public KisBrush {
    typedef KisBrush super;
    Q_OBJECT

public:
    KisImagePipeBrush(const QString& filename);
    virtual ~KisImagePipeBrush();

    virtual bool load();
    virtual bool save();

    /**
      @return the next image in the pipe.
      */
    virtual QImage img();

    /**
       @return the next mask in the pipe.
    */
    virtual KisAlphaMaskSP mask(double pressure = PRESSURE_DEFAULT, double subPixelX = 0, double subPixelY = 0) const;
    virtual KisLayerSP image(KisAbstractColorSpace * colorSpace, double pressure = PRESSURE_DEFAULT, double subPixelX = 0, double subPixelY = 0) const;

    virtual bool useColorAsMask() const;
    virtual void setUseColorAsMask(bool useColorAsMask);
    virtual bool hasColor() const;

    virtual enumBrushType brushType() const;
    
    virtual KisBoundary boundary();

private:
    bool init();
    void setParasite(const QString& parasite);

    QString m_name;
    QString m_parasite; // This contains some kind of instructions on how to use the brush
              // That I haven't decoded yet.
    Q_UINT32 m_numOfBrushes;
    mutable Q_UINT32 m_currentBrush;

    QByteArray m_data;
    mutable QPtrList<KisBrush> m_brushes;

    enumBrushType m_brushType;
    
};

#endif // KIS_IMAGEPIPE_BRUSH_
