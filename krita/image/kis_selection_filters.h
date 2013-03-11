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
#include "krita_export.h"

class KRITAIMAGE_EXPORT KisSelectionFilter
{
public:
    virtual ~KisSelectionFilter();

    virtual void process(KisPixelSelectionSP pixelSelection,
                         const QRect &rect) = 0;

    virtual QString name();
    virtual QRect changeRect(const QRect &rect);

protected:
    void computeBorder(qint32  *circ, qint32  xradius, qint32  yradius);

    void rotatePointers(quint8  **p, quint32 n);

    void computeTransition(quint8* transition, quint8** buf, qint32 width);
};

class KRITAIMAGE_EXPORT KisErodeSelectionFilter : public KisSelectionFilter
{
public:
    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);
};

class KRITAIMAGE_EXPORT KisDilateSelectionFilter : public KisSelectionFilter
{
public:
    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);
};

class KRITAIMAGE_EXPORT KisBorderSelectionFilter : public KisSelectionFilter
{
public:
    KisBorderSelectionFilter(qint32 xRadius, qint32 yRadius);

    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
};

class KRITAIMAGE_EXPORT KisFeatherSelectionFilter : public KisSelectionFilter
{
public:
    KisFeatherSelectionFilter(qint32 radius);

    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);
private:
    qint32 m_radius;
};

class KRITAIMAGE_EXPORT KisGrowSelectionFilter : public KisSelectionFilter
{
public:
    KisGrowSelectionFilter(qint32 xRadius, qint32 yRadius);

    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
};

class KRITAIMAGE_EXPORT KisShrinkSelectionFilter : public KisSelectionFilter
{
public:
    KisShrinkSelectionFilter(qint32 xRadius, qint32 yRadius, bool edgeLock);

    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);

private:
    qint32 m_xRadius;
    qint32 m_yRadius;
    qint32 m_edgeLock;
};

class KRITAIMAGE_EXPORT KisSmoothSelectionFilter : public KisSelectionFilter
{
public:
    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);
};

class KRITAIMAGE_EXPORT KisInvertSelectionFilter : public KisSelectionFilter
{
    QString name();

    QRect changeRect(const QRect &rect);

    void process(KisPixelSelectionSP pixelSelection, const QRect &rect);
};

#endif // KIS_SELECTION_FILTERS_H
