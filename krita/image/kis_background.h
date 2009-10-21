/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
class KRITAIMAGE_EXPORT KisBackground : public KisShared
{

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

