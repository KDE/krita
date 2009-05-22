/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include <QImage>

#include <kis_shared.h>
#include <krita_export.h>

/**
 * KisBackground paints the background pattern into the image,
 * behind the image contents.
 *
 * Note: this is for things like fixed color, paper patterns,
 * not for transparency checks.
 */
class KRITAIMAGE_EXPORT KisBackground : public KisShared {

public:

    KisBackground(const QImage& patternTile);
    virtual ~KisBackground();

    // Paint the background pattern into the image, 'behind' the image
    // contents. The coordinates are for the image's top-left corner
    // in image space.
    void paintBackground(QImage& image, const QRect& rc);
    void paintBackground(QImage& img, const QRect& scaledImageRect, const QSize& scaledImageSize, const QSize& imageSize);

    // Returns the pattern tile.
    const QImage& patternTile() const;

protected:

    KisBackground(KisBackground& rhs);
    KisBackground& operator=(KisBackground& rhs);

    static const int PATTERN_WIDTH = 64;
    static const int PATTERN_HEIGHT = 64;

    QImage m_patternTile;
};

#endif // KIS_BACKGROUND_H_

