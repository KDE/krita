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
#ifndef KIS_IMAGE_PIPE_BRUSH_P_H
#define KIS_IMAGE_PIPE_BRUSH_P_H

#include "kis_imagepipe_brush.h"
#include <math.h>
#include <limits.h>

#include <QImage>
#include <QPoint>
#include <QVector>
#include <QFile>
#include <QRegExp>
#include <QStringList>
#include <QTextStream>

#include <kis_debug.h>
#include <klocale.h>
#include <krandom.h>

#include "KoColorSpaceRegistry.h"

#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_brush.h"
#include "kis_layer.h"
#include "kis_boundary.h"



/**
 * The parasite info that gets loaded from the terribly documented gimp pipe brush parasite.
 * We only store data we actually use.
 * BC: How it seems the dimension stuff interacts with rank, selectionMode and the actual
 * selection of a brush to be drawn. So apparently you can have at most 4 'dimensions'.
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
class KisPipeBrushParasite
{
public:
    /// Set some default values
    KisPipeBrushParasite()
            : ncells(0)
            , dim(0)
            , needsMovement(false) {
        init();
    }

    void init();
    /// Initializes the brushesCount helper
    void setBrushesCount();

    /// Load the parasite from the source string
    KisPipeBrushParasite(const QString& source);

    /**
     * Saves a GIMP-compatible representation of this parasite to the device. Also writes the
     * number of brushes (== ncells) (no trailing '\n') */
    bool saveToDevice(QIODevice* dev) const;


    enum Placement { DefaultPlacement, ConstantPlacement, RandomPlacement };

    static int const MaxDim = 4;

    //qint32 step;
    qint32 ncells;
    qint32 dim;

    // Apparently only used for editing a pipe brush, which we won't at the moment
    // qint32 cols, rows;
    // qint32 cellwidth, cellheight;
    // Apparently the gimp doesn't use this anymore? Anyway it is a bit weird to
    // paint at someplace else than where your cursor displays it will...
    //Placement placement;
    qint32 rank[MaxDim];

    KisParasite::SelectionMode selection[MaxDim];

    /// The total count of brushes in each dimension (helper)
    qint32 brushesCount[MaxDim];

    /// The current index in each dimension, so that the selection modes know where to start
    qint32 index[MaxDim];

    /// If true, the brush won't be painted when there is no motion
    bool needsMovement;
};
#endif
