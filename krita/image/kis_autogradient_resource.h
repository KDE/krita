/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
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

#ifndef _KIS_AUTOGRADIENT_RESOURCE_H_
#define _KIS_AUTOGRADIENT_RESOURCE_H_

#include <QList>

#include "kis_gradient.h"

class KRITAIMAGE_EXPORT KisAutogradientResource : public KisGradient
{

public:
    KisAutogradientResource() : KisGradient("") {}

public:

    void createSegment( int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, QColor left, QColor right );

    const QList<double> getHandlePositions() const;
    const QList<double> getMiddleHandlePositions() const;

    /**
     * Moves the StartOffset of the specified segment to the specified value
     * and corrects the endoffset of the previous segment.
     * If the segment is the first Segment the startoffset will be set to 0.0 .
     * The offset will maximally be moved till the middle of the current or the previous
     * segment
     */
    void moveSegmentStartOffset( KisGradientSegment* segment, double t);

    /**
     * Moves the endoffset of the specified segment to the specified value
     * and corrects the startoffset of the following segment.
     * If the segment is the last segment the endoffset will be set to 1.0 .
     * The offset will maximally be moved till the middle of the current or the following
     * segment
     */
    void moveSegmentEndOffset( KisGradientSegment* segment, double t);

    /**
     * Moves the Middle of the specified segment to the specified value
     * The offset will maximally be moved till the endoffset or startoffset of the segment
     */
    void moveSegmentMiddleOffset( KisGradientSegment* segment, double t);


    void splitSegment( KisGradientSegment* segment );
    void duplicateSegment( KisGradientSegment* segment );
    void mirrorSegment( KisGradientSegment* segment );

    /**
     * Removes the specific segment from the gradient.
     * @return The segment which will be at the place of the old segment.
     * 0 if the segment is not in the gradient or it is not possible to remove the segment.
     */
    KisGradientSegment* removeSegment( KisGradientSegment* segment );

    /**
     * Checks if it's possible to remove an segment(at least two segments in the gradient)
     * @return true if it's possible to remove an segment
     */
    bool removeSegmentPossible() const;

    /**
     * Recreates the preview of the gradient
     */
    void updatePreview();
public:
    virtual bool load() { return false; };
};

#endif // _KIS_AUTOGRADIENT_RESOURCE_H_
