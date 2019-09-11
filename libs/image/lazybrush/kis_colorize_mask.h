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
        int transparentIndex = -1;
    };

public:
    KisColorizeMask();
    ~KisColorizeMask() override;

    KisColorizeMask(const KisColorizeMask& rhs);

    void initializeCompositeOp();
    const KoColorSpace* colorSpace() const override;

    // assign color profile without conversion of pixel data
    void setProfile(const KoColorProfile *profile, KUndo2Command *parentCommand);

    KUndo2Command* setColorSpace(const KoColorSpace * dstColorSpace,
                                 KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::internalRenderingIntent(),
                                 KoColorConversionTransformation::ConversionFlags conversionFlags = KoColorConversionTransformation::internalConversionFlags());

    KisPaintDeviceSP paintDevice() const override;
    KisPaintDeviceSP coloringProjection() const;

    KisPaintDeviceSP colorPickSourceDevice() const override;

    KisNodeSP clone() const override {
        return KisNodeSP(new KisColorizeMask(*this));
    }

    QIcon icon() const override;

    void setImage(KisImageWSP image) override;
    bool accept(KisNodeVisitor &v) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    QRect decorateRect(KisPaintDeviceSP &src,
                       KisPaintDeviceSP &dst,
                       const QRect & rc,
                       PositionToFilthy maskPos) const override;

    void setCurrentColor(const KoColor &color) override;
    void mergeToLayer(KisNodeSP layer, KisPostExecutionUndoAdapter *undoAdapter, const KUndo2MagicString &transactionText,int timedID) override;
    void writeMergeData(KisPainter *painter, KisPaintDeviceSP src) override;
    bool supportsNonIndirectPainting() const override;

    QRect exactBounds() const override;
    QRect extent() const override;

    /**
     * Colorize mask has its own "projection", so it should report it
     * to the parent layer using non-dependent-extent property
     */
    QRect nonDependentExtent() const override;

    void setSectionModelProperties(const KisBaseNode::PropertyList &properties) override;
    KisBaseNode::PropertyList sectionModelProperties() const override;

    KeyStrokeColors keyStrokesColors() const;
    void setKeyStrokesColors(KeyStrokeColors colors);

    void removeKeyStroke(const KoColor &color);

    QVector<KisPaintDeviceSP> allPaintDevices() const;
    void resetCache();

    void setUseEdgeDetection(bool value);
    bool useEdgeDetection() const;

    void setEdgeDetectionSize(qreal value);
    qreal edgeDetectionSize() const;

    void setFuzzyRadius(qreal value);
    qreal fuzzyRadius() const;

    void setCleanUpAmount(qreal value);
    qreal cleanUpAmount() const;

    void setLimitToDeviceBounds(bool value);
    bool limitToDeviceBounds() const;

    void testingAddKeyStroke(KisPaintDeviceSP dev, const KoColor &color, bool isTransparent = false);
    void testingRegenerateMask();
    KisPaintDeviceSP testingFilteredSource() const;

    QList<KisLazyFillTools::KeyStroke> fetchKeyStrokesDirect() const;
    void setKeyStrokesDirect(const QList<KisLazyFillTools::KeyStroke> &strokes);

    qint32 x() const override;
    qint32 y() const override;
    void setX(qint32 x) override;
    void setY(qint32 y) override;

    KisPaintDeviceList getLodCapableDevices() const override;

    void regeneratePrefilteredDeviceIfNeeded();

private Q_SLOTS:
    void slotUpdateRegenerateFilling(bool prefilterOnly = false);
    void slotRegenerationFinished(bool prefilterOnly);
    void slotRegenerationCancelled();

    void slotUpdateOnDirtyParent();
    void slotRecalculatePrefilteredImage();

Q_SIGNALS:
    void sigKeyStrokesListChanged();
    void sigUpdateOnDirtyParent() const;

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

    template <class DeviceMetricPolicy>
    QRect calculateMaskBounds(DeviceMetricPolicy policy) const;

    friend struct SetKeyStrokesColorSpaceCommand;
    friend struct KeyStrokeAddRemoveCommand;
    friend struct SetKeyStrokeColorsCommand;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLORIZE_MASK_H */
