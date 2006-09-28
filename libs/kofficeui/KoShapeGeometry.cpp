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
    mOriginalPosition = shape->absolutePosition();
    mOriginalSize = shape->size();
    widget.left->changeValue(mOriginalPosition.x());
    widget.top->changeValue(mOriginalPosition.y());
    widget.width->changeValue(mOriginalSize.width());
    widget.height->changeValue(mOriginalSize.height());

    connect(widget.protectSize, SIGNAL(stateChanged(int)),
            this, SLOT(protectSizeChanged(int)));

    if (shape->isLocked()) {
        widget.protectSize->setCheckState(Qt::Checked);
    }

    connect(widget.left, SIGNAL(valueChanged(double)),
            this, SLOT(updateShape()));
    connect(widget.top, SIGNAL(valueChanged(double)),
            this, SLOT(updateShape()));
    connect(widget.width, SIGNAL(valueChanged(double)),
            this, SLOT(updateShape()));
    connect(widget.height, SIGNAL(valueChanged(double)),
            this, SLOT(updateShape()));
}

void KoShapeGeometry::updateShape() {
    m_shape->repaint();
    QPointF pos(widget.left->value(), widget.top->value());
    m_shape->setAbsolutePosition(pos);
    QSizeF size(widget.width->value(), widget.height->value());
    m_shape->resize(size);
    m_shape->repaint();
}

void KoShapeGeometry::protectSizeChanged(int protectSizeState) {
    bool lock = (protectSizeState == Qt::Checked);
    widget.left->setDisabled(lock);
    widget.top->setDisabled(lock);
    widget.width->setDisabled(lock);
    widget.height->setDisabled(lock);
}

void KoShapeGeometry::save() {
    bool lock = (widget.protectSize->checkState() == Qt::Checked);
    m_shape->setLocked(lock);
}

void KoShapeGeometry::cancel() {
    m_shape->setAbsolutePosition(mOriginalPosition);
    m_shape->resize(mOriginalSize);
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
