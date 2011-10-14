/* This file is part of the KDE project
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
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

#include "InitialsCommentShape.h"
#include "Globals.h"

#include <QPainter>

InitialsCommentShape::InitialsCommentShape()
: KoShape()
, m_active(true)
{
}

InitialsCommentShape::~InitialsCommentShape()
{
}

void InitialsCommentShape::saveOdf(KoShapeSavingContext& /*context*/) const
{
}

bool InitialsCommentShape::loadOdf(const KoXmlElement& /*element*/, KoShapeLoadingContext& /*context*/)
{
    return false;
}

void InitialsCommentShape::paint(QPainter& painter, const KoViewConverter& converter, KoShapePaintingContext &)
{
    applyConversion(painter, converter);

    QLinearGradient gradient(initialsBoxPoint, QPointF(0, initialsBoxSize.height()));
    qreal lighterPos = 0.0;
    qreal darkerPos = 0.0;
    if(!m_active){
        darkerPos = 1.0;
    }
    else {
        lighterPos = 1.0;
    }
    gradient.setColorAt(lighterPos, QColor(Qt::yellow));
    gradient.setColorAt(darkerPos, QColor(254, 201, 7));
    const QBrush brush(gradient);
    painter.setBrush(brush);

    painter.setPen(Qt::black);

    painter.drawRect(QRectF(initialsBoxPoint, initialsBoxSize));

    painter.drawText(QRectF(initialsBoxPoint, initialsBoxSize), Qt::AlignCenter, m_initials);
}

void InitialsCommentShape::setInitials(QString initials)
{
    m_initials = initials;
}

QString InitialsCommentShape::initials()
{
    return m_initials;

}

bool InitialsCommentShape::isActive() const
{
    return m_active;
}

void InitialsCommentShape::setActive(bool activate)
{
    m_active = activate;
}

void InitialsCommentShape::toogleActive()
{
    setActive(!m_active);
    update();
}

