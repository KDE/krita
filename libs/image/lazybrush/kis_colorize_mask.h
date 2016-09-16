/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_COLORIZE_MASK_H
#define __KIS_COLORIZE_MASK_H

#include <QScopedPointer>

#include "kis_types.h"
#include "kis_effect_mask.h"
#include "kritaimage_export.h"

class KoColor;
class KUndo2Command;

namespace KisLazyFillTools
{
    struct KeyStroke;
}


class KRITAIMAGE_EXPORT KisColorizeMask : public KisEffectMask
{
    Q_OBJECT
public:
    struct KeyStrokeColors {
        QVector<KoColor> colors;
        int transparentIndex;
    };

public:
    KisColorizeMask();
    ~KisColorizeMask();

    KisColorizeMask(const KisColorizeMask& rhs);

    void initializeCompositeOp();
    const KoColorSpace* colorSpace() const;

    // assign color profile without conversion of pixel data
    void setProfile(const KoColorProfile *profile);

    KUndo2Command* setColorSpace(const KoColorSpace * dstColorSpace,
                                 KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags());

    KisPaintDeviceSP paintDevice() const;
    KisPaintDeviceSP coloringProjection() const;

    KisNodeSP clone() const {
        return KisNodeSP(new KisColorizeMask(*this));
    }

    QIcon icon() const;

    void setImage(KisImageWSP image) Q_DECL_OVERRIDE;
    bool accept(KisNodeVisitor &v);
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter);

    QRect decorateRect(KisPaintDeviceSP &src,
                       KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const;

    void setCurrentColor(const KoColor &color);
    void mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID);
    void writeMergeData(KisPainter *painter, KisPaintDeviceSP src);

    QRect exactBounds() const;
    QRect extent() const;

    void setSectionModelProperties(const KisBaseNode::PropertyList &properties);
    KisBaseNode::PropertyList sectionModelProperties() const;

    KeyStrokeColors keyStrokesColors() const;
    void setKeyStrokesColors(KeyStrokeColors colors);

    void removeKeyStroke(const KoColor &color);

    QVector<KisPaintDeviceSP> allPaintDevices() const;
    void resetCache();

    void testingAddKeyStroke(KisPaintDeviceSP dev, const KoColor &color, bool isTransparent = false);
    void testingRegenerateMask();
    KisPaintDeviceSP testingFilteredSource() const;

    QList<KisLazyFillTools::KeyStroke> fetchKeyStrokesDirect() const;
    void setKeyStrokesDirect(const QList<KisLazyFillTools::KeyStroke> &strokes);

    qint32 x() const;
    qint32 y() const;
    void setX(qint32 x);
    void setY(qint32 y);

    KisPaintDeviceList getLodCapableDevices() const;

private Q_SLOTS:
    void slotUpdateRegenerateFilling();
    void slotRegenerationFinished();

Q_SIGNALS:
    void sigKeyStrokesListChanged();

private:
    // NOTE: please access this methods using model properties only!

    bool needsUpdate() const;
    void setNeedsUpdate(bool value);

    bool showColoring() const;
    void setShowColoring(bool value);

    bool showKeyStrokes() const;
    void setShowKeyStrokes(bool value);


private:
    void rerenderFakePaintDevice();
    KisImageSP fetchImage() const;
    void moveAllInternalDevices(const QPoint &diff);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLORIZE_MASK_H */
