/*
   Copyright (c) 2000 Matthias Elter  <elter@kde.org>
                 2004 Boudewijn Rempt <boud@valdyas.org>
                 2004 Adrian Page <adrian@pagenet.plus.com>
                 2004, 2007 Sven Langkamp <sven.langkamp@gmail.com>
                 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KOSEGMENTGRADIENT_H
#define KOSEGMENTGRADIENT_H

#include <QList>
#include <QColor>

#include <KoResource.h>
#include <resources/KoAbstractGradient.h>
#include "KoColor.h"

#include <kritapigment_export.h>


enum {
    INTERP_LINEAR = 0,
    INTERP_CURVED,
    INTERP_SINE,
    INTERP_SPHERE_INCREASING,
    INTERP_SPHERE_DECREASING
};

enum {
    COLOR_INTERP_RGB,
    COLOR_INTERP_HSV_CCW,
    COLOR_INTERP_HSV_CW
};

/// Write API docs here
class KRITAPIGMENT_EXPORT KoGradientSegment
{
public:
    KoGradientSegment(int interpolationType, int colorInterpolationType, qreal startOffset, qreal middleOffset, qreal endOffset, const KoColor& startColor, const KoColor& endColor);

    // startOffset <= t <= endOffset
    void colorAt(KoColor&, qreal t) const;

    const KoColor& startColor() const;
    const KoColor& endColor() const;

    void setStartColor(const KoColor& color) {
        m_startColor = color;
    }
    void setEndColor(const KoColor& color) {
        m_endColor = color;
    }

    qreal startOffset() const;
    qreal middleOffset() const;
    qreal endOffset() const;

    void setStartOffset(qreal t);
    void setMiddleOffset(qreal t);
    void setEndOffset(qreal t);

    qreal length() {
        return m_length;
    }

    int interpolation() const;
    int colorInterpolation() const;

    void setInterpolation(int interpolationType);
    void setColorInterpolation(int colorInterpolationType);

    bool isValid() const;
protected:

    class ColorInterpolationStrategy
    {
    public:
        ColorInterpolationStrategy() {}
        virtual ~ColorInterpolationStrategy() {}

        virtual void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const = 0;
        virtual int type() const = 0;
    };

    class RGBColorInterpolationStrategy : public ColorInterpolationStrategy
    {
    public:
        static RGBColorInterpolationStrategy *instance();

        void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const override;
        int type() const override {
            return COLOR_INTERP_RGB;
        }

    private:
        RGBColorInterpolationStrategy();

        static RGBColorInterpolationStrategy *m_instance;
        const KoColorSpace * const m_colorSpace;
    };

    class HSVCWColorInterpolationStrategy : public ColorInterpolationStrategy
    {
    public:
        static HSVCWColorInterpolationStrategy *instance();

        void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const override;
        int type() const override {
            return COLOR_INTERP_HSV_CW;
        }
    private:
        HSVCWColorInterpolationStrategy();

        static HSVCWColorInterpolationStrategy *m_instance;
        const KoColorSpace * const m_colorSpace;
    };

    class HSVCCWColorInterpolationStrategy : public ColorInterpolationStrategy
    {
    public:
        static HSVCCWColorInterpolationStrategy *instance();

        void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const override;
        int type() const override {
            return COLOR_INTERP_HSV_CCW;
        }
    private:
        HSVCCWColorInterpolationStrategy();

        static HSVCCWColorInterpolationStrategy *m_instance;
        const KoColorSpace * const m_colorSpace;
    };

    class InterpolationStrategy
    {
    public:
        InterpolationStrategy() {}
        virtual ~InterpolationStrategy() {}

        virtual qreal valueAt(qreal t, qreal middle) const = 0;
        virtual int type() const = 0;
    };

    class LinearInterpolationStrategy : public InterpolationStrategy
    {
    public:
        static LinearInterpolationStrategy *instance();

        qreal valueAt(qreal t, qreal middle) const override;
        int type() const override {
            return INTERP_LINEAR;
        }

        // This does the actual calculation and is made
        // static as an optimization for the other
        // strategies that need this for their own calculation.
        static qreal calcValueAt(qreal t, qreal middle);

    private:
        LinearInterpolationStrategy() {}

        static LinearInterpolationStrategy *m_instance;
    };

    class CurvedInterpolationStrategy : public InterpolationStrategy
    {
    public:
        static CurvedInterpolationStrategy *instance();

        qreal valueAt(qreal t, qreal middle) const override;
        int type() const override {
            return INTERP_CURVED;
        }
    private:
        CurvedInterpolationStrategy();

        static CurvedInterpolationStrategy *m_instance;
        qreal m_logHalf;
    };

    class SphereIncreasingInterpolationStrategy : public InterpolationStrategy
    {
    public:
        static SphereIncreasingInterpolationStrategy *instance();

        qreal valueAt(qreal t, qreal middle) const override;
        int type() const override {
            return INTERP_SPHERE_INCREASING;
        }
    private:
        SphereIncreasingInterpolationStrategy() {}

        static SphereIncreasingInterpolationStrategy *m_instance;
    };

    class SphereDecreasingInterpolationStrategy : public InterpolationStrategy
    {
    public:
        static SphereDecreasingInterpolationStrategy *instance();

        qreal valueAt(qreal t, qreal middle) const override;
        int type() const override {
            return INTERP_SPHERE_DECREASING;
        }
    private:
        SphereDecreasingInterpolationStrategy() {}

        static SphereDecreasingInterpolationStrategy *m_instance;
    };

    class SineInterpolationStrategy : public InterpolationStrategy
    {
    public:
        static SineInterpolationStrategy *instance();

        qreal valueAt(qreal t, qreal middle) const override;
        int type() const override {
            return INTERP_SINE;
        }
    private:
        SineInterpolationStrategy() {}

        static SineInterpolationStrategy *m_instance;
    };
private:
    InterpolationStrategy *m_interpolator;
    ColorInterpolationStrategy *m_colorInterpolator;

    qreal m_startOffset;
    qreal m_middleOffset;
    qreal m_endOffset;
    qreal m_length;
    qreal m_middleT;

    KoColor m_startColor;
    KoColor m_endColor;
};

/**
  * KoSegmentGradient stores a segment based gradients like Gimp gradients
  */
class KRITAPIGMENT_EXPORT KoSegmentGradient : public KoAbstractGradient
{

public:
    explicit KoSegmentGradient(const QString &file = QString());
    ~KoSegmentGradient() override;

    KoAbstractGradientSP clone() const override;

    /// reimplemented
    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;

    /// not implemented
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    /// reimplemented
    void colorAt(KoColor& dst, qreal t) const override;

    /**
     * Returns the segment at a given position
     * @param t position inside the gradient, with 0 <= t <= 1
     * @return the segment the position, 0 if no segment is found
     */
    KoGradientSegment *segmentAt(qreal t) const;

    /// reimplemented
    QGradient* toQGradient() const override;

    /// reimplemented
    QString defaultFileExtension() const override;

    /**
     * @brief toXML
     * convert the gradient to xml.
     */
    void toXML(QDomDocument& doc, QDomElement& gradientElt) const;
    /**
     * @brief fromXML
     * get a segment gradient from xml.
     * @return gradient
     */
    static KoSegmentGradient fromXML(const QDomElement& elt);

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

    const QList<KoGradientSegment *>& segments() const;

protected:
    KoSegmentGradient(const KoSegmentGradient &rhs);

    inline void pushSegment(KoGradientSegment* segment) {
        m_segments.push_back(segment);
    }

    QList<KoGradientSegment *> m_segments;

    private:
    bool init();
};

typedef QSharedPointer<KoSegmentGradient> KoSegmentGradientSP;

#endif // KOSEGMENTGRADIENT_H

