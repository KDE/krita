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
#include <qmap.h>
#include <qstring.h>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_brush.h"
#include "kis_global.h"

class QCString;
class QImage;
class QPoint;
class QSize;

/**
 * The parasite info that gets loaded from the terribly documented gimp pipe brush parasite.
 * We only store data we actually use.
 * BC: How it seems the dimension stuff interacts with rank, selectionMode and the actual
 * selection of a brush to be drawn. So apparantly you can have at most 4 'dimensions'.
 * Each dimension has a number of brushes, the rank. Each dimension has an associated selection
 * mode and placement mode (which we don't use). The selection mode says us in which way
 * which of the brushes or brush sets will be selected. In the case of a 1-dimensional pipe
 * brush it is easy.
 * However, when there are more dimensions it is a bit harder. You can according to the gimp
 * source maximally use 4 dimensions. When you want to select a brush, you first go to the
 * first dimension. Say it has a rank of 2. The code chooses one of the 2 according to the
 * selection mode. Say we choose 2. Then the currentBrush will skip over all the brushes
 * from the first element in dimension 1. Then in dimension we pick again from the choices
 * we have in dimension 2. We again add the appropriate amount to currentBrush. And so on,
 * until we have reached dimension dim. Or at least, that is how it looks like, we'll know
 * for sure when we can test it better with >1 dim brushes and Angular selectionMode.
 **/
class KisPipeBrushParasite {
public:
    KisPipeBrushParasite() {}
    KisPipeBrushParasite(const QString& source);
    /** Velocity won't be supported, atm Angular and Tilt aren't either, but have chances of implementation */
    enum SelectionMode {
        Constant, Incremental, Angular, Velocity, Random, Pressure, TiltX, TiltY
    };
    enum Placement { DefaultPlacement, ConstantPlacement, RandomPlacement };
    static int const MaxDim = 4;
    //Q_INT32 step;
    Q_INT32 ncells;
    Q_INT32 dim;
    // Apparantly only used for editing a pipe brush, which we won't at the moment
    // Q_INT32 cols, rows;
    // Q_INT32 cellwidth, cellheight;
    // Aparantly the gimp doesn't use this anymore? Anyway it is a bit weird to
    // paint at someplace else than where your cursor displays it will...
    //Placement placement;
    Q_INT32 rank[MaxDim];
    SelectionMode selection[MaxDim];
    /// The total count of brushes in each dimension (helper)
    Q_INT32 brushesCount[MaxDim];
    /// The current index in each dimension, so that the selection modes know where to start
    Q_INT32 index[MaxDim];
    /// If true, the brush won't be painted when there is no motion
    bool needsMovement;
};


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
    virtual KisAlphaMaskSP mask(const KisPaintInformation& info,
                                double subPixelX = 0, double subPixelY = 0) const;
    virtual KisLayerSP image(KisColorSpace * colorSpace, const KisPaintInformation& info,
                             double subPixelX = 0, double subPixelY = 0) const;

    virtual bool useColorAsMask() const;
    virtual void setUseColorAsMask(bool useColorAsMask);
    virtual bool hasColor() const;

    virtual enumBrushType brushType() const;
    
    virtual KisBoundary boundary();
    
    KisPipeBrushParasite parasite() { return m_parasite; }
    
    virtual bool canPaintFor(const KisPaintInformation& info);

private:
    bool init();
    void setParasiteString(const QString& parasite);
    void selectNextBrush(const KisPaintInformation& info) const;

    QString m_name;
    QString m_parasiteString; // Contains instructions on how to use the brush
    mutable KisPipeBrushParasite m_parasite;
    Q_UINT32 m_numOfBrushes;
    mutable Q_UINT32 m_currentBrush;

    QByteArray m_data;
    mutable QPtrList<KisBrush> m_brushes;

    enumBrushType m_brushType;
    
};

#endif // KIS_IMAGEPIPE_BRUSH_
