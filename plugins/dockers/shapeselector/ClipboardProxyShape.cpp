/*
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#include "ClipboardProxyShape.h"
#include "ZoomHandler.h"

#include <KoShapeContainer.h>
#include <KoShapeBorderModel.h>

#include <QPainter>
#include <QSizeF>

ClipboardProxyShape::ClipboardProxyShape(KoShape*clipboardItem, const QByteArray &clipboardData)
    : m_child(clipboardItem),
    m_clipboardData(clipboardData)
{
    Q_ASSERT(clipboardItem);
}

static void deleteShape(KoShape *shape)
{
    KoShapeContainer *container = dynamic_cast<KoShapeContainer*>(shape);
    if (container) {
        foreach(KoShape *shape, container->shapes())
            deleteShape(shape);
    }
    delete shape;
}

ClipboardProxyShape::~ClipboardProxyShape()
{
    deleteShape(m_child);
}

void ClipboardProxyShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    painter.setClipRect(converter.documentToView(QRectF(QPointF(), size())));
    QSizeF nestedSize = m_child->size();
    QSizeF ourSize = size();
    const qreal scale = qMin(ourSize.width() / nestedSize.width(), ourSize.height() / nestedSize.height());
    if (scale != 1.0) {
        ZoomHandler zh;
        qreal x1;
        converter.zoom(&x1, &x1);
        zh.setAbsoluteZoom(x1 * scale);
        painter.save();
        m_child->paint(painter, zh);
        painter.restore();
        if (m_child->border())
            m_child->border()->paint(m_child, painter, zh);
    }
    else {
        painter.save();
        m_child->paint(painter, converter);
        painter.restore();
        if (m_child->border())
            m_child->border()->paint(m_child, painter, converter);
    }
}
