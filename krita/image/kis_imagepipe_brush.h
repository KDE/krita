/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <QList>
#include <QMap>
#include <QString>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_brush.h"
#include "kis_global.h"

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
    /// Set some default values
    KisPipeBrushParasite() : ncells(0), dim(0), needsMovement(false) {
        for (int i = 0; i < MaxDim; i++) {
            rank[i] = index[i] = brushesCount[i] = 0;
            selection[i] = Constant;
        }
    }
    /// Initializes the brushesCount helper
    void setBrushesCount();
    /// Load the parasite from the source string
    KisPipeBrushParasite(const QString& source);
    /**
     * Saves a GIMP-compatible representation of this parasite to the device. Also writes the
     * number of brushes (== ncells) (no trailing '\n') */
    bool saveToDevice(QIODevice* dev) const;

    /** Velocity won't be supported, atm Angular and Tilt aren't either, but have chances of implementation */
    enum SelectionMode {
        Constant, Incremental, Angular, Velocity, Random, Pressure, TiltX, TiltY
    };
    enum Placement { DefaultPlacement, ConstantPlacement, RandomPlacement };
    static int const MaxDim = 4;
    //qint32 step;
    qint32 ncells;
    qint32 dim;
    // Apparantly only used for editing a pipe brush, which we won't at the moment
    // qint32 cols, rows;
    // qint32 cellwidth, cellheight;
    // Aparantly the gimp doesn't use this anymore? Anyway it is a bit weird to
    // paint at someplace else than where your cursor displays it will...
    //Placement placement;
    qint32 rank[MaxDim];
    SelectionMode selection[MaxDim];
    /// The total count of brushes in each dimension (helper)
    qint32 brushesCount[MaxDim];
    /// The current index in each dimension, so that the selection modes know where to start
    qint32 index[MaxDim];
    /// If true, the brush won't be painted when there is no motion
    bool needsMovement;
};


class KRITAIMAGE_EXPORT KisImagePipeBrush : public KisBrush {
    typedef KisBrush super;
    Q_OBJECT

public:
    KisImagePipeBrush(const QString& filename);
    /**
     * Specialized constructor that makes a new pipe brush from a sequence of samesize
     * devices. The fact that it's a vector of a vector, is to support multidimensional
     * brushes (not yet supported!) */
    KisImagePipeBrush(const QString& name, int w, int h,
                      QVector< QVector<KisPaintDevice*> > devices,
                      QVector<KisPipeBrushParasite::SelectionMode> modes);
    virtual ~KisImagePipeBrush();

    virtual bool load();
    virtual bool save();
    /// Will call KisBrush's saveToDevice as well
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
      @return the next image in the pipe.
      */
    virtual QImage img();

    /**
       @return the next mask in the pipe.
    */
    virtual KisAlphaMaskSP mask(const KisPaintInformation& info,
                                double subPixelX = 0, double subPixelY = 0) const;
    virtual KisPaintDeviceSP image(KoColorSpace * colorSpace, const KisPaintInformation& info,
                             double subPixelX = 0, double subPixelY = 0) const;

    virtual bool useColorAsMask() const;
    virtual void setUseColorAsMask(bool useColorAsMask);
    virtual bool hasColor() const;

    virtual enumBrushType brushType() const;

    virtual KisBoundary boundary();

    KisPipeBrushParasite const& parasite() const { return m_parasite; }

    virtual bool canPaintFor(const KisPaintInformation& info);

    virtual void makeMaskImage();

    virtual KisImagePipeBrush* clone() const;

private:
    bool init();
    void setParasiteString(const QString& parasite);
    void selectNextBrush(const KisPaintInformation& info) const;

    QString m_name;
    QString m_parasiteString; // Contains instructions on how to use the brush
    mutable KisPipeBrushParasite m_parasite;
    qint32 m_numOfBrushes;
    mutable quint32 m_currentBrush;

    QByteArray m_data;
    mutable QList<KisBrush *> m_brushes;

    enumBrushType m_brushType;

};

#endif // KIS_IMAGEPIPE_BRUSH_
