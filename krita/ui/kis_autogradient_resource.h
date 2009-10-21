/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <sven.langkamp@gmail.com>
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

/**
 * @file this file is part of the Krita application in koffice.
 * Handles the gradient colour picker segments and their positions.
 * @author Cyrille Berger
 * @author Sven Langkamp
 * @author documentation by hscott
 * @since 1.4
 */

#ifndef _KIS_AUTOGRADIENT_RESOURCE_H_
#define _KIS_AUTOGRADIENT_RESOURCE_H_

#include <QList>

#include <KoSegmentGradient.h>
#include <krita_export.h>

class QColor;

/**
 * @class KisAutogradientResource
 * Is for creating custom gradient colour pickers. A gradient is
 * a field which shifts from one colour to one or more other
 * colours. The gradient colour picker defines what colours are
 * used in the image's gradient field. This class only handles
 * the gradient colour picker, _not_ the image's gradient field.
 */
class KRITAUI_EXPORT KisAutogradientResource
        : public KoSegmentGradient
{

public:
    KisAutogradientResource() : KoSegmentGradient("") {}

public:
    /**
     * a gradient colour picker can consist of one or more segments.
     * A segment has two end points - each colour in the gradient
     * colour picker represents a segment end point.
     * @param interpolation
     * @param colorInterpolation
     * @param startOffset
     * @param endOffset
     * @param middleOffset
     * @param left
     * @param right
     * @return void
     */
    void createSegment(int interpolation, int colorInterpolation, double startOffset, double endOffset, double middleOffset, const QColor & left, const QColor & right);

    /**
     * gets a list of end points of the segments in the gradient
     * colour picker. If two colours, one segment then two end
     * points, and if three colours, then two segments with four
     * endpoints.
     * @return a list of double values
     */
    const QList<double> getHandlePositions() const;

    /**
     * gets a list of middle points of the segments in the gradient
     * colour picker.
     * @return a list of double values
     */
    const QList<double> getMiddleHandlePositions() const;

    /**
     * Moves the StartOffset of the specified segment to the
     * specified value and corrects the endoffset of the previous
     * segment. If the segment is the first Segment the startoffset
     * will be set to 0.0 . The offset will maximally be moved till
     * the middle of the current or the previous segment. This is
     * useful if someone clicks to move the handler for a segment,
     * to set the half the segment to the right and half the segment
     * to the left of the handler.
     * @param segment the segment for which to move the relative
     * offset within the gradient colour picker.
     * @param t the new startoff position for the segment
     * @return void
     */
    void moveSegmentStartOffset(KoGradientSegment* segment, double t);

    /**
     * Moves the endoffset of the specified segment to the specified
     * value and corrects the startoffset of the following segment.
     * If the segment is the last segment the endoffset will be set
     * to 1.0 . The offset will maximally be moved till the middle
     * of the current or the following segment. This is useful if
     * someone moves the segment handler in the gradient colour
     * picker, and needs the segment to move with it. Sets the end
     * position of the segment to the correct new position.
     * @param segment the segment for which to move the relative
     * end position within the gradient colour picker.
     * @param t the new end position for the segment
     * @return void
     */
    void moveSegmentEndOffset(KoGradientSegment* segment, double t);

    /**
     * moves the Middle of the specified segment to the specified
     * value. The offset will maximally be moved till the endoffset
     * or startoffset of the segment. This sets the middle of the
     * segment to the same position as the handler of the gradient
     * colour picker.
     * @param segment the segment for which to move the relative
     * middle position within the gradient colour picker.
     * @param t the new middle position for the segment
     * @return void
     */
    void moveSegmentMiddleOffset(KoGradientSegment* segment, double t);

    /**
     * splits the specified segment into two equal parts
     * @param segment the segment to split
     * @return void
     */
    void splitSegment(KoGradientSegment* segment);

    /**
     * duplicate the specified segment
     * @param segment the segment to duplicate
     * @return void
     */
    void duplicateSegment(KoGradientSegment* segment);

    /**
     * create a segment horizontally reversed to the specified one.
     * @param segment the segment to reverse
     * @return void
     */
    void mirrorSegment(KoGradientSegment* segment);

    /**
     * removes the specific segment from the gradient colour picker.
     * @param segment the segment to remove
     * @return the segment which will be at the place of the old
     * segment. 0 if the segment is not in the gradient or it is
     * not possible to remove the segment.
     */
    KoGradientSegment* removeSegment(KoGradientSegment* segment);

    /**
     * checks if it's possible to remove a segment (at least two
     * segments in the gradient)
     * @return true if it's possible to remove an segment
     */
    bool removeSegmentPossible() const;

public:
    virtual bool load() {
        return false;
    }
};

#endif // _KIS_AUTOGRADIENT_RESOURCE_H_
