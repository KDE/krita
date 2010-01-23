/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "Intro.h"

#include "DivineProportionShape.h"

Intro::Intro()
{
    widget.setupUi(this);
}

void Intro::open(KoShape *shape)
{
    m_shape = dynamic_cast<DivineProportionShape*> (shape);
    widget.topRight->setChecked(true);
}

void Intro::save()
{
    if (m_shape == 0)
        return;
    Q_ASSERT(m_resourceManager);
    if (m_resourceManager->hasResource(KoCanvasResource::PageSize)) {
        QSizeF size = m_resourceManager->sizeResource(KoCanvasResource::PageSize);
        if (size.height() > size.width()) {
            m_shape->setSize(QSizeF(size.height(), size.width()));
            m_shape->rotate(-90);
            m_shape->setAbsolutePosition(QPointF(size.width() / 2, size.height() / 2));
        }
        else {
            m_shape->setPosition(QPointF(0, 0));
            m_shape->setSize(size);
        }
    }

    DivineProportionShape::Orientation orientation;
    if (widget.topLeft->isChecked())
        orientation = DivineProportionShape::TopLeft;
    else if (widget.topRight->isChecked())
        orientation = DivineProportionShape::TopRight;
    else if (widget.bottomLeft->isChecked())
        orientation = DivineProportionShape::BottomLeft;
    else
        orientation = DivineProportionShape::BottomRight;
    m_shape->setOrientation(orientation);
    m_shape->setPrintable(widget.printable->isChecked());
}

KAction *Intro::createAction()
{
    return 0;
}
