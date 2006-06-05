/*
 *  kis_gradient.h - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *                2004 Boudewijn Rempt <boud@valdyas.org>
 *                2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_GRADIENT_H
#define KIS_GRADIENT_H

#include <QList>
#include <QColor>

#include <kio/job.h>

#include "kis_resource.h"
#include "kis_global.h"
#include "KoColor.h"

class QImage;

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

class KRITAIMAGE_EXPORT KisGradientSegment {
    public:
        KisGradientSegment(int interpolationType, int colorInterpolationType, double startOffset, double middleOffset, double endOffset, const KoColor& startColor, const KoColor& endColor);

        // startOffset <= t <= endOffset
        KoColor colorAt(double t) const;

        const KoColor& startColor() const;
        const KoColor& endColor() const;

        void setStartColor(const KoColor& color) { m_startColor = color; }
        void setEndColor(const KoColor& color) { m_endColor = color; }

        double startOffset() const;
        double middleOffset() const;
        double endOffset() const;

        void setStartOffset(double t);
        void setMiddleOffset(double t);
        void setEndOffset(double t);

        double length() { return m_length; }

        int interpolation() const;
        int colorInterpolation() const;

        void setInterpolation(int interpolationType);
        void setColorInterpolation(int colorInterpolationType);

        bool isValid() const;
    protected:

        class ColorInterpolationStrategy {
        public:
            ColorInterpolationStrategy() {}
            virtual ~ColorInterpolationStrategy() {}

            virtual KoColor colorAt(double t, KoColor start, KoColor end) const = 0;
            virtual int type() const = 0;
        };

        class RGBColorInterpolationStrategy : public ColorInterpolationStrategy {
        public:
            static RGBColorInterpolationStrategy *instance();

            virtual KoColor colorAt(double t, KoColor start, KoColor end) const;
            virtual int type() const { return COLOR_INTERP_RGB; }

        private:
            RGBColorInterpolationStrategy();

            static RGBColorInterpolationStrategy *m_instance;
            KoColorSpace * m_colorSpace;
        };

        class HSVCWColorInterpolationStrategy : public ColorInterpolationStrategy {
        public:
            static HSVCWColorInterpolationStrategy *instance();

            virtual KoColor colorAt(double t, KoColor start, KoColor end) const;
            virtual int type() const { return COLOR_INTERP_HSV_CW; }
        private:
            HSVCWColorInterpolationStrategy();

            static HSVCWColorInterpolationStrategy *m_instance;
            KoColorSpace * m_colorSpace;
        };

        class HSVCCWColorInterpolationStrategy : public ColorInterpolationStrategy {
        public:
            static HSVCCWColorInterpolationStrategy *instance();

            virtual KoColor colorAt(double t, KoColor start, KoColor end) const;
            virtual int type() const { return COLOR_INTERP_HSV_CCW; }
        private:
            HSVCCWColorInterpolationStrategy();

            static HSVCCWColorInterpolationStrategy *m_instance;
            KoColorSpace * m_colorSpace;
        };

        class InterpolationStrategy {
        public:
            InterpolationStrategy() {}
            virtual ~InterpolationStrategy() {}

            virtual double valueAt(double t, double middle) const = 0;
            virtual int type() const = 0;
        };

        class LinearInterpolationStrategy : public InterpolationStrategy {
        public:
            static LinearInterpolationStrategy *instance();

            virtual double valueAt(double t, double middle) const;
            virtual int type() const { return INTERP_LINEAR; }

            // This does the actual calculation and is made
            // static as an optimisation for the other
            // strategies that need this for their own calculation.
            static double calcValueAt(double t, double middle);

        private:
            LinearInterpolationStrategy() {}

            static LinearInterpolationStrategy *m_instance;
        };

        class CurvedInterpolationStrategy : public InterpolationStrategy {
        public:
            static CurvedInterpolationStrategy *instance();

            virtual double valueAt(double t, double middle) const;
            virtual int type() const { return INTERP_CURVED; }
        private:
            CurvedInterpolationStrategy();

            static CurvedInterpolationStrategy *m_instance;
            double m_logHalf;
        };

        class SphereIncreasingInterpolationStrategy : public InterpolationStrategy {
        public:
            static SphereIncreasingInterpolationStrategy *instance();

            virtual double valueAt(double t, double middle) const;
            virtual int type() const { return INTERP_SPHERE_INCREASING; }
        private:
            SphereIncreasingInterpolationStrategy() {}

            static SphereIncreasingInterpolationStrategy *m_instance;
        };

        class SphereDecreasingInterpolationStrategy : public InterpolationStrategy {
        public:
            static SphereDecreasingInterpolationStrategy *instance();

            virtual double valueAt(double t, double middle) const;
            virtual int type() const { return INTERP_SPHERE_DECREASING; }
        private:
            SphereDecreasingInterpolationStrategy() {}

            static SphereDecreasingInterpolationStrategy *m_instance;
        };

        class SineInterpolationStrategy : public InterpolationStrategy {
        public:
            static SineInterpolationStrategy *instance();

            virtual double valueAt(double t, double middle) const;
            virtual int type() const { return INTERP_SINE; }
        private:
            SineInterpolationStrategy() {}

            static SineInterpolationStrategy *m_instance;
        };
    private:
        InterpolationStrategy *m_interpolator;
        ColorInterpolationStrategy *m_colorInterpolator;

        double m_startOffset;
        double m_middleOffset;
        double m_endOffset;
        double m_length;
        double m_middleT;

        KoColor m_startColor;
        KoColor m_endColor;
};

class KRITAIMAGE_EXPORT KisGradient : public KisResource {
    typedef KisResource super;
    Q_OBJECT

public:
    KisGradient(const QString& file);
    virtual ~KisGradient();

    virtual bool load();
    virtual bool save();
    virtual QImage img();
    virtual QImage generatePreview(int width, int height) const;

    KoColor colorAt(double t) const;
    KoColorSpace * colorSpace() const { return m_colorSpace; }

    KisGradientSegment *segmentAt(double t) const;

protected:
    inline void pushSegment( KisGradientSegment* segment ) { m_segments.push_back(segment); };
    void setImage(const QImage& img);

    QList<KisGradientSegment *> m_segments;

protected:
    KoColorSpace * m_colorSpace;

private:
    bool init();

private:
    QImage m_img;
};

#endif // KIS_GRADIENT_H

