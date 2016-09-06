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

    KisPaintDeviceSP paintDevice() const;
    KisPaintDeviceSP coloringProjection() const;

    KisNodeSP clone() const {
        return KisNodeSP(new KisColorizeMask(*this));
    }

    QIcon icon() const;

    void setImage(KisImageWSP image);
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
    KisImageSP fetchImage() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_COLORIZE_MASK_H */
