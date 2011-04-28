/* This file is part of the KDE project
 * Copyright (C) 2007, 2009, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "AnchorStrategy.h"

#include "KoTextShapeContainerModel.h"
#include "KoTextDocumentLayout.h"
#include "KoTextLayoutRootArea.h"

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>

#include <kdebug.h>



AnchorStrategy::AnchorStrategy(KoTextAnchor *anchor)
        : m_model(0)
        , m_anchor(anchor)
{
}

AnchorStrategy::~AnchorStrategy()
{
    if (m_model)
        m_model->removeAnchor(m_anchor);
}

void AnchorStrategy::detachFromModel()
{
    m_model = 0;
}

/// as multiple shapes can hold 1 text flow; the anchored shape can be moved between containers and thus models
void AnchorStrategy::updatePosition(KoShape *shape, const QTextDocument *document, int posInDocument)
{
    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(document->documentLayout());
    Q_ASSERT(lay);
    KoTextLayoutRootArea *rootArea = lay->rootAreaForPosition(posInDocument);

    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(rootArea->associatedShape());
    if (container == 0) {
        if (m_model)
            m_model->removeAnchor(m_anchor);
        m_model = 0;
        shape->setParent(0);
        return;
    }
    KoTextShapeContainerModel *theModel = dynamic_cast<KoTextShapeContainerModel*>(container->model());
    if (theModel != m_model) {
        if (m_model)
            m_model->removeAnchor(m_anchor);
        if (shape->parent() != container) {
            if (shape->parent()) {
                shape->parent()->removeShape(shape);
            }
            container->addShape(shape);
        }
        m_model = theModel;
        m_model->addAnchor(m_anchor);
    }
    Q_ASSERT(m_model == theModel);
}
