/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISDECORATIONSWRAPPERLAYER_H
#define KISDECORATIONSWRAPPERLAYER_H

#include "kis_types.h"
#include "kis_external_layer_iface.h"
#include <QScopedPointer>

class KisDocument;

/**
 * KisDecorationsWrapperLayer is a fake node for connecting Grids, Guides and Assistants
 * the KisNodeVisitor system. This allows things like crops and transformations to be
 * applied to these decoration style items.
 */
class KisDecorationsWrapperLayer : public KisExternalLayer
{
    Q_OBJECT
public:
    KisDecorationsWrapperLayer(KisDocument *document);
    KisDecorationsWrapperLayer(const KisDecorationsWrapperLayer &rhs);
    ~KisDecorationsWrapperLayer();

    void setDocument(KisDocument *document);
    KisDocument* document() const;


public:
    // reimplemented from KisLayer

    bool allowAsChild(KisNodeSP) const override;

    bool accept(KisNodeVisitor&) override;
    void accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter) override;

    KisNodeSP clone() const override;

    KisPaintDeviceSP original() const override;
    KisPaintDeviceSP paintDevice() const override;
    bool isFakeNode() const override;

    KUndo2Command* crop(const QRect & rect) override;

    KUndo2Command* transform(const QTransform &transform) override;

    bool supportsPerspectiveTransform() const override;

    void setImage(KisImageWSP image) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef KisSharedPtr<KisDecorationsWrapperLayer> KisDecorationsWrapperLayerSP;
typedef KisWeakSharedPtr<KisDecorationsWrapperLayer> KisDecorationsWrapperLayerWSP;


#endif // KISDECORATIONSWRAPPERLAYER_H
