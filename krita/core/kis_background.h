/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_BACKGROUND_H_
#define KIS_BACKGROUND_H_

#include <qimage.h>

#include <ksharedptr.h>

class KisBackground : public KShared {

public:
    KisBackground();
    virtual ~KisBackground();

    // Paint the background pattern into the image, 'behind' the image
    // contents. The coordinates are for the image's top-left corner
    // in image space.
    void paintBackground(QImage image, int leftX, int topY);

    void paintBackground(QImage image, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize);

    // Returns the pattern tile.
    const QImage& patternTile() const;

protected:
    static const int PATTERN_WIDTH = 32;
    static const int PATTERN_HEIGHT = 32;

    QImage m_patternTile;
};

#endif // KIS_BACKGROUND_H_

