/*
 *  Copyright (c) 2005 Michael Thaler
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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


#ifndef KIS_SELECTION_FILTERS_H
#define KIS_SELECTION_FILTERS_H

#include "kis_types.h"
#include "kritaimage_export.h"
#include "kis_default_bounds_base.h"

#include <QRect>
#include <QString>

class KUndo2MagicString;


class KRITAIMAGE_EXPORT KisSelectionFilter
{
public:
    virtual ~KisSelectionFilter();

    virtual void process(KisPixelSelectionSP pixelSelection,
                         const QRect &rect) = 0;

    virtual KUndo2MagicString name();
    virtual QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds);

protected:
    void computeBorder(qint32  *circ, qint32  xradius, qint32  yradius);

    void rotatePointers(quint8  **p, quint32 n);

    void computeTransition(quint8* transition, quint8** buf, qint32 width);
};

class KRITAIMAGE_EXPORT KisErodeSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

class KRITAIMAGE_EXPORT KisDilateSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

class KRITAIMAGE_EXPORT KisBorderSelectionFilter : public KisSelectionFilter
{
public:
    KisBorderSelectionFilter(qint32 xRadius, qint32 yRadius);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
};

class KRITAIMAGE_EXPORT KisFeatherSelectionFilter : public KisSelectionFilter
{
public:
    KisFeatherSelectionFilter(qint32 radius);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
private:
    qint32 m_radius;
};

class KRITAIMAGE_EXPORT KisGrowSelectionFilter : public KisSelectionFilter
{
public:
    KisGrowSelectionFilter(qint32 xRadius, qint32 yRadius);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
};

class KRITAIMAGE_EXPORT KisShrinkSelectionFilter : public KisSelectionFilter
{
public:
    KisShrinkSelectionFilter(qint32 xRadius, qint32 yRadius, bool edgeLock);

    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
    qint32 m_edgeLock;
};

class KRITAIMAGE_EXPORT KisSmoothSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

class KRITAIMAGE_EXPORT KisInvertSelectionFilter : public KisSelectionFilter
{
public:
    KUndo2MagicString name() override;

    QRect changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds) override;

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect) override;
};

#endif // KIS_SELECTION_FILTERS_H
