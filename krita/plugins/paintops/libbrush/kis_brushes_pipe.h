/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_BRUSHES_PIPE_H
#define __KIS_BRUSHES_PIPE_H

template<class BrushType>
class KisBrushesPipe
{
public:
    KisBrushesPipe() {
        m_currentBrushIndex = 0;
    }

    KisBrushesPipe(const KisBrushesPipe &rhs) {
        qDeleteAll(m_brushes);
        m_brushes.clear();
        foreach(BrushType *brush, rhs.m_brushes) {
            m_brushes.append(brush->clone());
        }

        m_currentBrushIndex = rhs.m_currentBrushIndex;
    }

    virtual ~KisBrushesPipe() {
        qDeleteAll(m_brushes);
    }

    virtual void clear() {
        qDeleteAll(m_brushes);
        m_brushes.clear();
        m_currentBrushIndex = 0;
    }

    BrushType* firstBrush() const {
        return m_brushes.first();
    }

    BrushType* lastBrush() const {
        return m_brushes.last();
    }

    BrushType* currentBrush() const {
        return !m_brushes.isEmpty() ? m_brushes.at(m_currentBrushIndex) : 0;
    }

    qint32 maskWidth(double scale, double angle) const {
        BrushType *brush = currentBrush();
        return brush ? brush->maskWidth(scale, angle) : 0;
    }

    qint32 maskHeight(double scale, double angle) const {
        BrushType *brush = currentBrush();
        return brush ? brush->maskHeight(scale, angle) : 0;
    }

    void setAngle(qreal angle) {
        foreach (BrushType *brush, m_brushes) {
            brush->setAngle(angle);
        }
    }

    void setScale(qreal scale) {
        foreach (BrushType *brush, m_brushes) {
            brush->setScale(scale);
        }
    }

    void setSpacing(double spacing) {
        foreach (BrushType *brush, m_brushes) {
            brush->setSpacing(spacing);
        }
    }

    bool hasColor() const {
        foreach (BrushType *brush, m_brushes) {
            if (brush->hasColor()) return true;
        }
        return false;
    }

    void generateMaskAndApplyMaskOrCreateDab(KisFixedPaintDeviceSP dst, KisBrush::ColoringInformation* coloringInformation,
                                             double scaleX, double scaleY, double angle, const KisPaintInformation& info,
                                             double subPixelX , double subPixelY,
                                             qreal softnessFactor) {

        BrushType *brush = currentBrush();
        if (!brush) return;

        brush->generateMaskAndApplyMaskOrCreateDab(dst, coloringInformation, scaleX, scaleY, angle, info, subPixelX, subPixelY, softnessFactor);
        selectNextBrush(info);
    }

    KisFixedPaintDeviceSP paintDevice(const KoColorSpace * colorSpace,
                                      double scale, double angle,
                                      const KisPaintInformation& info,
                                      double subPixelX, double subPixelY) {

        BrushType *brush = currentBrush();
        if (!brush) return 0;

        KisFixedPaintDeviceSP device = brush->paintDevice(colorSpace, scale, angle, info, subPixelX, subPixelY);
        selectNextBrush(info);
        return device;
    }

    QVector<BrushType*> testingGetBrushes() {
        return m_brushes;
    }

    void testingSelectNextBrush(const KisPaintInformation& info) {
        selectNextBrush(info);
    }

protected:
    void addBrush(BrushType *brush) {
        m_brushes.append(brush);
    }

    virtual void selectNextBrush(const KisPaintInformation& info) = 0;

protected:
    QVector<BrushType*> m_brushes;
    int m_currentBrushIndex;
};

#endif /* __KIS_BRUSHES_PIPE_H */
