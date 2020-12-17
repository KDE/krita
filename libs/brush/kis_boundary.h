/*
 *  SPDX-FileCopyrightText: 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_BOUNDARY_H_
#define _KIS_BOUNDARY_H_

#include <QList>
#include <QPair>
#include <QPainter>

#include <kritabrush_export.h>

#include "kis_types.h"

/**
 * Generates an 'outline' for a paint device. It should look a bit like the outline of a
 * marching ants selection.
 *
 * It's not really optimized, so it's not recommended to do big things with it and expect
 * it to be fast.
 *
 * Usage: construct a KisBoundary, and then run a generateBoundary(w, h) on it. After that,
 * you can use the KisBoundaryPainter::paint method to let it paint the outline, or get a pixmap.
 *
 * If you are debugging the brush outlines, be aware that the pipeline for this
 * data is somewhat complex, involving such user classes:
 * KisBoundary, KisBrush, KisBrushBasedPaintOpSettings, KisTool, KisCurrentOutlineFetcher
 **/
class BRUSH_EXPORT KisBoundary
{
public:
    KisBoundary(KisFixedPaintDeviceSP dev);
    ~KisBoundary();
    void generateBoundary();

    void paint(QPainter& painter) const;

    /// returns the outline saved in QPainterPath
    QPainterPath path() const;

private:
    struct Private;
    Private* const d;
};

#endif // _KIS_BOUNDARY_H_
