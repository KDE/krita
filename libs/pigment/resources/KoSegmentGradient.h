/*
   Copyright (c) 2000 Matthias Elter  <elter@kde.org>
                 2004 Boudewijn Rempt <boud@valdyas.org>
                 2004 Adrian Page <adrian@pagenet.plus.com>
                 2004, 2007 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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

#include <QtCore/QList>
#include <QtGui/QColor>

#include <kio/job.h>

#include <KoResource.h>
#include "KoAbstractGradient.h"
#include "KoColor.h"

#include <pigment_export.h>


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
class PIGMENTCMS_EXPORT KoGradientSegment
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

        virtual void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const;
        virtual int type() const {
            return COLOR_INTERP_RGB;
        }

    private:
        RGBColorInterpolationStrategy();

        static RGBColorInterpolationStrategy *m_instance;
        const KoColorSpace * const m_colorSpace;
        mutable KoColor buffer;
        mutable KoColor m_start;
        mutable KoColor m_end;
    };

    class HSVCWColorInterpolationStrategy : public ColorInterpolationStrategy
    {
    public:
        static HSVCWColorInterpolationStrategy *instance();

        virtual void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const;
        virtual int type() const {
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

        virtual void colorAt(KoColor& dst, qreal t, const KoColor& start, const KoColor& end) const;
        virtual int type() const {
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

        virtual qreal valueAt(qreal t, qreal middle) const;
        virtual int type() const {
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

        virtual qreal valueAt(qreal t, qreal middle) const;
        virtual int type() const {
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

        virtual qreal valueAt(qreal t, qreal middle) const;
        virtual int type() const {
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

        virtual qreal valueAt(qreal t, qreal middle) const;
        virtual int type() const {
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

        virtual qreal valueAt(qreal t, qreal middle) const;
        virtual int type() const {
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
class PIGMENTCMS_EXPORT KoSegmentGradient : public KoAbstractGradient
{

public:
    KoSegmentGradient(const QString& file);
    virtual ~KoSegmentGradient();

    /// reimplemented
    virtual bool load();

    /// not implemented
    virtual bool save();

    /// reimplemented
    void colorAt(KoColor& dst, qreal t) const;

    /**
     * Returns the segment at a given position
     * @param t position inside the gradient, with 0 <= t <= 1
     * @return the segment the position, 0 if no segment is found
     */
    KoGradientSegment *segmentAt(qreal t) const;

    /// reimplemented
    virtual QGradient* toQGradient() const;

    /// reimplemented
    QString defaultFileExtension() const;

protected:
    inline void pushSegment(KoGradientSegment* segment) {
        m_segments.push_back(segment);
    }

    QList<KoGradientSegment *> m_segments;

private:
    bool init();
};

#endif // KOSEGMENTGRADIENT_H

