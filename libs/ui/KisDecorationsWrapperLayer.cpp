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

#include "KisDecorationsWrapperLayer.h"

#include "KisDocument.h"
#include "kis_node_visitor.h"
#include "kis_processing_visitor.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include "kis_painting_assistant.h"
#include "kis_default_bounds.h"

struct KisDecorationsWrapperLayer::Private
{
    KisDocument *document = 0;
    KisPaintDeviceSP fakeOriginalDevice;
};

KisDecorationsWrapperLayer::KisDecorationsWrapperLayer(KisDocument *document)
    : KisExternalLayer(document->image(), "decorations-wrapper-layer", OPACITY_OPAQUE_U8),
      m_d(new Private)
{
    m_d->document = document;
    m_d->fakeOriginalDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    m_d->fakeOriginalDevice->setDefaultBounds(new KisDefaultBounds(document->image()));
}


KisDecorationsWrapperLayer::KisDecorationsWrapperLayer(const KisDecorationsWrapperLayer &rhs)
    : KisExternalLayer(rhs.image(), "decorations-wrapper-layer", OPACITY_OPAQUE_U8),
      m_d(new Private)
{
    m_d->document = rhs.m_d->document;
    m_d->fakeOriginalDevice = new KisPaintDevice(*rhs.m_d->fakeOriginalDevice);
}

KisDecorationsWrapperLayer::~KisDecorationsWrapperLayer()
{
}

void KisDecorationsWrapperLayer::setDocument(KisDocument *document)
{
    m_d->document = document;
    KIS_SAFE_ASSERT_RECOVER(image() == document->image()) {
        setImage(document->image());
    }
}

KisDocument *KisDecorationsWrapperLayer::document() const
{
    return m_d->document;
}

bool KisDecorationsWrapperLayer::allowAsChild(KisNodeSP) const
{
    return false;
}

bool KisDecorationsWrapperLayer::accept(KisNodeVisitor &visitor)
{
    return visitor.visit(this);
}

void KisDecorationsWrapperLayer::accept(KisProcessingVisitor &visitor, KisUndoAdapter *undoAdapter)
{
    visitor.visit(this, undoAdapter);
}

KisNodeSP KisDecorationsWrapperLayer::clone() const {
    return new KisDecorationsWrapperLayer(*this);
}

KisPaintDeviceSP KisDecorationsWrapperLayer::original() const
{
    return m_d->fakeOriginalDevice;
}

KisPaintDeviceSP KisDecorationsWrapperLayer::paintDevice() const
{
    return 0;
}

bool KisDecorationsWrapperLayer::isFakeNode() const
{
    return true;
}

bool KisDecorationsWrapperLayer::supportsPerspectiveTransform() const
{
    return false;
}

void KisDecorationsWrapperLayer::setImage(KisImageWSP image)
{
    m_d->fakeOriginalDevice->setDefaultBounds(new KisDefaultBounds(image));
    KisExternalLayer::setImage(image);
}

KUndo2Command *KisDecorationsWrapperLayer::crop(const QRect &rect)
{
    return transform(QTransform::fromTranslate(-rect.x(), -rect.y()));
}

KUndo2Command *KisDecorationsWrapperLayer::transform(const QTransform &transform)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->document, 0);

    struct UndoCommand : public KUndo2Command
    {
        UndoCommand(KisDocument *document, const QTransform &transform)
            : m_document(document),
              m_transform(transform)
        {}

        void undo() override {
            doTransform(m_transform.inverted());
        }

        void redo() override {
            doTransform(m_transform);
        }

    private:
        void doTransform(const QTransform &transform) {
            const QTransform imageToDocument =
                QTransform::fromScale(1 / m_document->image()->xRes(),
                                      1 / m_document->image()->yRes());

            KisGridConfig gridConfig = m_document->gridConfig();
            if (gridConfig.showGrid()) {
                gridConfig.transform(transform);
                m_document->setGridConfig(gridConfig);
            }

            KisGuidesConfig guidesConfig = m_document->guidesConfig();
            if (guidesConfig.hasGuides()) {
                guidesConfig.transform(imageToDocument.inverted() * transform * imageToDocument);
                m_document->setGuidesConfig(guidesConfig);
            }

            QList<KisPaintingAssistantSP> assistants = m_document->assistants();
            Q_FOREACH(KisPaintingAssistantSP assistant, assistants) {
                assistant->transform(imageToDocument.inverted() * transform * imageToDocument);
            }
            m_document->setAssistants(assistants);
        }

    private:
        KisDocument *m_document;
        QTransform m_transform;
    };

    return new UndoCommand(m_d->document, transform);
}
