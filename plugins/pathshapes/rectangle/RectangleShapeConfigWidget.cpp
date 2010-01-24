/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "RectangleShapeConfigWidget.h"
#include "RectangleShape.h"
#include "RectangleShapeConfigCommand.h"

RectangleShapeConfigWidget::RectangleShapeConfigWidget()
{
    widget.setupUi(this);

    connect(widget.cornerRadiusX, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
    connect(widget.cornerRadiusY, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
}

void RectangleShapeConfigWidget::setUnit(KoUnit unit)
{
    widget.cornerRadiusX->setUnit(unit);
    widget.cornerRadiusY->setUnit(unit);
}

void RectangleShapeConfigWidget::open(KoShape *shape)
{
    m_rectangle = dynamic_cast<RectangleShape*>(shape);
    if (! m_rectangle)
        return;

    widget.cornerRadiusX->blockSignals(true);
    widget.cornerRadiusY->blockSignals(true);

    QSizeF size = m_rectangle->size();

    widget.cornerRadiusX->setMaximum(0.5 * size.width());
    widget.cornerRadiusX->setValue(0.01 * m_rectangle->cornerRadiusX() * 0.5 * size.width());
    widget.cornerRadiusY->setMaximum(0.5 * size.height());
    widget.cornerRadiusY->setValue(0.01 * m_rectangle->cornerRadiusY() * 0.5 * size.height());

    widget.cornerRadiusX->blockSignals(false);
    widget.cornerRadiusY->blockSignals(false);
}

void RectangleShapeConfigWidget::save()
{
    if (! m_rectangle)
        return;

    QSizeF size = m_rectangle->size();

    m_rectangle->setCornerRadiusX(100.0 * widget.cornerRadiusX->value() / (0.5 * size.width()));
    m_rectangle->setCornerRadiusY(100.0 * widget.cornerRadiusY->value() / (0.5 * size.height()));
}

QUndoCommand * RectangleShapeConfigWidget::createCommand()
{
    if (! m_rectangle)
        return 0;
    QSizeF size = m_rectangle->size();

    qreal cornerRadiusX = 100.0 * widget.cornerRadiusX->value() / (0.5 * size.width());
    qreal cornerRadiusY = 100.0 * widget.cornerRadiusY->value() / (0.5 * size.height());

    return new RectangleShapeConfigCommand(m_rectangle, cornerRadiusX, cornerRadiusY);
}

#include <RectangleShapeConfigWidget.moc>
