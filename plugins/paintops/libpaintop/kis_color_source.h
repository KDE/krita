/*
 *  SPDX-FileCopyrightText: 2006-2007, 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KIS_DYNAMIC_COLORING_H_
#define _KIS_DYNAMIC_COLORING_H_

#include "kis_paintop_option.h"

#include <QRect>

#include <KoColor.h>
#include <KoAbstractGradient.h>

#include <kis_types.h>
#include <kritapaintop_export.h>

class KoColorTransformation;
class KisPaintInformation;

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
     * @param pi paint information
     */
    virtual void selectColor(double mix, const KisPaintInformation &pi) = 0;
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
    ~KisUniformColorSource() override;
    virtual void rotate(double);
    virtual void resize(double , double);
    void applyColorTransformation(const KoColorTransformation* transfo) override;
    const KoColorSpace* colorSpace() const override;
    void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& offset) const override;
    bool isUniformColor() const override;
    const KoColor& uniformColor() const override;
protected:
    KoColor m_color;
};

class PAINTOP_EXPORT KisPlainColorSource : public KisUniformColorSource
{
public:
    KisPlainColorSource(const KoColor& backGroundColor, const KoColor& foreGroundColor);
    ~KisPlainColorSource() override;
    void selectColor(double mix, const KisPaintInformation &pi) override;
private:
    KoColor m_backGroundColor;
    KoColor m_cachedBackGroundColor;
    KoColor m_foreGroundColor;
};

class PAINTOP_EXPORT KisGradientColorSource : public KisUniformColorSource
{
public:
    KisGradientColorSource(const KoAbstractGradientSP gradient, const KoColorSpace* workingCS);
    ~KisGradientColorSource() override;
    void selectColor(double mix, const KisPaintInformation &pi) override;
private:
    const KoAbstractGradientSP m_gradient;
};

class PAINTOP_EXPORT KisUniformRandomColorSource : public KisUniformColorSource
{
public:
    KisUniformRandomColorSource();
    ~KisUniformRandomColorSource() override;
    void selectColor(double mix, const KisPaintInformation &pi) override;
};

class PAINTOP_EXPORT KisTotalRandomColorSource : public KisColorSource
{
public:
    KisTotalRandomColorSource();
    ~KisTotalRandomColorSource() override;
public:
    void selectColor(double mix, const KisPaintInformation &pi) override;
    void applyColorTransformation(const KoColorTransformation* transfo) override;
    const KoColorSpace* colorSpace() const override;
    void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& offset) const override;
    virtual void rotate(double r);
    virtual void resize(double xs, double ys);
    bool isUniformColor() const override;
private:
    const KoColorSpace* m_colorSpace;
};

class PAINTOP_EXPORT KoPatternColorSource : public KisColorSource
{
public:
    KoPatternColorSource(KisPaintDeviceSP _pattern, int _width, int _height, bool _locked);
    ~KoPatternColorSource() override;
public:
    void selectColor(double mix, const KisPaintInformation &pi) override;
    void applyColorTransformation(const KoColorTransformation* transfo) override;
    const KoColorSpace* colorSpace() const override;
    void colorize(KisPaintDeviceSP, const QRect& rect, const QPoint& _offset) const override;
    virtual void rotate(double r);
    virtual void resize(double xs, double ys);
    bool isUniformColor() const override;
private:
    const KisPaintDeviceSP m_device;
    QRect m_bounds;
    bool m_locked;
};

#endif
