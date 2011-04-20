/*
 *  Copyright (c) 2006-2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_DYNAMIC_COLORING_H_
#define _KIS_DYNAMIC_COLORING_H_

#include "kis_paintop_option.h"

#include <QRect>

#include <KoColor.h>

#include <kis_types.h>

class KoAbstractGradient;
class KoColorTransformation;

/**
 * A color source allow to abstract how a brush is colorized,
 * and to apply transformation.
 *
 * The first function to call is @ref selectColor , then any of the transformation.
 */
class PAINTOP_EXPORT KisColorSource
{
public:
    virtual ~KisColorSource();
public:
    /**
     * This is function is called to initialize the color that will be used for the dab.
     * @param mix is a parameter between 0.0 and 1.0
     */
    virtual void selectColor(double mix) = 0;
    /**
     * Apply a color transformation on the selected color
     */
    virtual void applyColorTransformation(const KoColorTransformation* transfo) = 0;
    virtual const KoColorSpace* colorSpace() const = 0;
    /**
     * Apply the color on a paint device
     */
    virtual void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& _offset) const = 0;
    /**
     * @return true if the color is an uniform color
     */
    virtual bool isUniformColor() const = 0;
    /**
     * @return the color if the color is uniformed
     */
    virtual const KoColor& uniformColor() const;
};

class PAINTOP_EXPORT KisUniformColorSource : public KisColorSource
{
public:
    KisUniformColorSource();
    virtual ~KisUniformColorSource();
    virtual void rotate(double);
    virtual void resize(double , double);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& offset) const;
    virtual bool isUniformColor() const;
    virtual const KoColor& uniformColor() const;
protected:
    KoColor* m_color;
private:
    mutable KoColor* m_cachedColor;
};

class PAINTOP_EXPORT KisPlainColorSource : public KisUniformColorSource
{
public:
    KisPlainColorSource(const KoColor& backGroundColor, const KoColor& foreGroundColor);
    virtual ~KisPlainColorSource();
    virtual void selectColor(double mix);
private:
    KoColor m_backGroundColor, m_foreGroundColor;
    KoColor* m_cachedBackGroundColor;
};

class PAINTOP_EXPORT KisGradientColorSource : public KisUniformColorSource
{
public:
    KisGradientColorSource(const KoAbstractGradient* gradient, const KoColorSpace* workingCS);
    virtual ~KisGradientColorSource();
    virtual void selectColor(double mix);
private:
    const KoAbstractGradient* m_gradient;
    const KoColorSpace* m_colorSpace;
};

class PAINTOP_EXPORT KisUniformRandomColorSource : public KisUniformColorSource
{
public:
    KisUniformRandomColorSource();
    virtual ~KisUniformRandomColorSource();
    virtual void selectColor(double mix);
};

class PAINTOP_EXPORT KisTotalRandomColorSource : public KisColorSource
{
public:
    KisTotalRandomColorSource();
    virtual ~KisTotalRandomColorSource();
public:
    virtual void selectColor(double mix);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& offset) const;
    virtual void rotate(double r);
    virtual void resize(double xs, double ys);
    virtual bool isUniformColor() const;
private:
    const KoColorSpace* m_colorSpace;
};

class PAINTOP_EXPORT KisPatternColorSource : public KisColorSource
{
public:
    KisPatternColorSource(KisPaintDeviceSP _pattern, int _width, int _height, bool _locked);
    virtual ~KisPatternColorSource();
public:
    virtual void selectColor(double mix);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& _offset) const;
    virtual void rotate(double r);
    virtual void resize(double xs, double ys);
    virtual bool isUniformColor() const;
private:
    const KisPaintDeviceSP m_device;
    QRect m_bounds;
    bool m_locked;
};

#endif
