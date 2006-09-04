/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include "KoShapeGeometry.h"

#include <KoShape.h>
#include <klocale.h>

KoShapeGeometry::KoShapeGeometry()
    : m_shape(0)
{
    widget.setupUi(this);
    setObjectName(i18n("Geometry"));
    widget.left->setMinimum(0.0);
    widget.top->setMinimum(0.0);
    widget.width->setMinimum(0.0);
    widget.height->setMinimum(0.0);
}

KoShapeGeometry::~KoShapeGeometry() {
}

void KoShapeGeometry::open(KoShape *shape) {
    m_shape = shape;
    QPointF absolutePosition = shape->absolutePosition();
    widget.left->changeValue(absolutePosition.x());
    widget.top->changeValue(absolutePosition.y());
    widget.width->changeValue(shape->size().width());
    widget.height->changeValue(shape->size().height());
}

void KoShapeGeometry::save() {
    QPointF pos(widget.left->value(), widget.top->value());
    m_shape->setAbsolutePosition(pos);
    m_shape->resize(QSizeF(widget.width->value(), widget.height->value()));
}

KAction *KoShapeGeometry::createAction() {
    return 0; // TODO
}

void KoShapeGeometry::setUnit(KoUnit::Unit unit) {
    widget.left->setUnit(unit);
    widget.top->setUnit(unit);
    widget.width->setUnit(unit);
    widget.height->setUnit(unit);
}

#include "KoShapeGeometry.moc"
