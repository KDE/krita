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
    virtual void selectColor(double mix) = 0;
    virtual void darken(qint32 v) = 0;
    virtual void applyColorTransformation(const KoColorTransformation* transfo) = 0;
    virtual const KoColorSpace* colorSpace() const = 0;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect) = 0;
    virtual void colorAt(int x, int y, KoColor*) = 0;
    virtual bool isUniformColor() const = 0;
    virtual const KoColor& uniformColor() const;
};

class PAINTOP_EXPORT KisUniformColorSource : public KisColorSource
{
public:
    KisUniformColorSource();
    virtual ~KisUniformColorSource();
    virtual void darken(qint32 v);
    virtual void rotate(double);
    virtual void resize(double , double);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect);
    virtual void colorAt(int x, int y, KoColor*);
    virtual bool isUniformColor() const;
    virtual const KoColor& uniformColor() const;
protected:
    KoColor* m_color;
private:
    KoColor* m_cachedColor;
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
    virtual void darken(qint32 v);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect);
    virtual void colorAt(int x, int y, KoColor*);
    virtual void rotate(double r);
    virtual void resize(double xs, double ys);
    virtual bool isUniformColor() const;
private:
    const KoColorSpace* m_colorSpace;
};

#endif
