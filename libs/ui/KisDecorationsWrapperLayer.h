/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
