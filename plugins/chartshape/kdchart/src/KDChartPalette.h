/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2001-2007 Klaralvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/
#ifndef __KDCHART_PALETTE_H__
#define __KDCHART_PALETTE_H__

#include <QObject>
#include <QBrush>
#include "KDChartGlobal.h"

namespace KDChart {

  /**
   * \brief A Palette is a set of brushes (or colors) to be used
   * for painting data sets.
   *
   * The palette class encapsulates a colletion of brushes, which in
   * the simplest case are colors, to be used for painting a series of
   * data sets. When asked for the m-th color, a palette of size n will
   * wrap around and thus cycle through the available colors.
   *
   * Three builtin palettes are provided for convenience, one with a default
   * set of colors, one with a subdued color selection, one with rainbow
   * colors.
   *
   * When a palette changes, it emits a changed() signal. Hook up to it, if
   * you want to repaint when the color selection changes.
   */

class KDCHART_EXPORT Palette: public QObject
{
    Q_OBJECT
public:
    explicit Palette( QObject *parent  = 0 );
    Palette( const Palette& );
    Palette &operator= ( const Palette & );

    ~Palette();

    /** Provide access to the three builtin palettes, one with standard bright
     * colors, one with more subdued colors, and one with rainbow colors.  */
    static const Palette& defaultPalette();
    static const Palette& subduedPalette();
    static const Palette& rainbowPalette();

    /** @return whether this represents a valid palette. For a palette to be
     * valid it needs to have at least one brush associated. */
    bool isValid() const;

    /** @return the number of brushed in the palette.  */
    int size() const;

    /** Adds \a brush to the palette. If no \a position is specified, the
     * brush is appended. 
     */
    void addBrush( const QBrush & brush, int position = -1 );

    /**
     * Query the palette for a brush at the specified position. If the
     * position exceeds the size of the palette, it wraps around.
     */
    QBrush getBrush( int position ) const;

    /** Remove the brush at position \a position, if there is one.  */
    void removeBrush( int position );

Q_SIGNALS:
    /**  Emitted whenever the palette changes. Views listen to this and
     * repaing.  */
    void changed();

private:
    KDCHART_DECLARE_PRIVATE_BASE_VALUE( Palette )
};

}
KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::Palette )
#endif
