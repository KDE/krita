/*
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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

#include "dynamicbrush_export.h"

#include <KoColor.h>

#include <kis_types.h>

#include "kis_dynamic_transformable.h"

class KoAbstractGradient;
class KoColorTransformation;

class DYNAMIC_BRUSH_EXPORT KisDynamicColoring : public KisDynamicTransformable
{
public:
    virtual ~KisDynamicColoring();
public:
    virtual void selectColor(double mix) = 0;
    virtual KisDynamicColoring* clone() const = 0;
    virtual void darken(qint32 v) = 0;
    virtual void applyColorTransformation(const KoColorTransformation* transfo) = 0;
    virtual const KoColorSpace* colorSpace() const = 0;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect) = 0;
    virtual void colorAt(int x, int y, KoColor*) = 0;
};

class KisUniformColoring : public KisDynamicColoring
{
public:
    KisUniformColoring();
    virtual ~KisUniformColoring();
    virtual void darken(qint32 v);
    virtual void rotate(double);
    virtual void resize(double , double);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect);
    virtual void colorAt(int x, int y, KoColor*);
protected:
    KoColor* m_color;
private:
    KoColor* m_cachedColor;
};

class DYNAMIC_BRUSH_EXPORT KisPlainColoring : public KisUniformColoring
{
public:
    KisPlainColoring(const KoColor& backGroundColor, const KoColor& foreGroundColor);
    virtual ~KisPlainColoring();
    virtual KisDynamicColoring* clone() const;
    virtual void selectColor(double mix);
private:
    KoColor m_backGroundColor, m_foreGroundColor;
    KoColor* m_cachedBackGroundColor;
};

class DYNAMIC_BRUSH_EXPORT KisGradientColoring : public KisUniformColoring
{
public:
    KisGradientColoring(const KoAbstractGradient* gradient, const KoColorSpace* workingCS);
    virtual ~KisGradientColoring();
    virtual KisDynamicColoring* clone() const;
    virtual void selectColor(double mix);
private:
    const KoAbstractGradient* m_gradient;
    const KoColorSpace* m_colorSpace;
};

class DYNAMIC_BRUSH_EXPORT KisUniformRandomColoring : public KisUniformColoring
{
public:
    KisUniformRandomColoring();
    virtual ~KisUniformRandomColoring();
    virtual KisDynamicColoring* clone() const;
    virtual void selectColor(double mix);
};

class DYNAMIC_BRUSH_EXPORT KisTotalRandomColoring : public KisDynamicColoring
{
public:
    KisTotalRandomColoring();
    virtual ~KisTotalRandomColoring();
public:
    virtual void selectColor(double mix);
    virtual KisDynamicColoring* clone() const;
    virtual void darken(qint32 v);
    virtual void applyColorTransformation(const KoColorTransformation* transfo);
    virtual const KoColorSpace* colorSpace() const;
    virtual void colorize(KisPaintDeviceSP, const QRect& rect);
    virtual void colorAt(int x, int y, KoColor*);
    virtual void rotate(double r);
    virtual void resize(double xs, double ys);
private:
    const KoColorSpace* m_colorSpace;
};

#endif
