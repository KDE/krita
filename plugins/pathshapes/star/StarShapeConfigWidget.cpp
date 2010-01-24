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

#include "StarShapeConfigWidget.h"
#include "StarShape.h"
#include "StarShapeConfigCommand.h"

StarShapeConfigWidget::StarShapeConfigWidget()
{
    widget.setupUi(this);

    connect(widget.corners, SIGNAL(valueChanged(int)), this, SIGNAL(propertyChanged()));
    connect(widget.innerRadius, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
    connect(widget.outerRadius, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
    connect(widget.convex, SIGNAL(stateChanged(int)), this, SIGNAL(propertyChanged()));
    connect(widget.convex, SIGNAL(clicked()), this, SLOT(typeChanged()));
}

void StarShapeConfigWidget::setUnit(KoUnit unit)
{
    widget.innerRadius->setUnit(unit);
    widget.outerRadius->setUnit(unit);
}

void StarShapeConfigWidget::open(KoShape *shape)
{
    m_star = dynamic_cast<StarShape*>(shape);
    if (! m_star)
        return;

    widget.corners->blockSignals(true);
    widget.innerRadius->blockSignals(true);
    widget.outerRadius->blockSignals(true);
    widget.convex->blockSignals(true);

    widget.corners->setValue(m_star->cornerCount());
    widget.innerRadius->changeValue(m_star->baseRadius());
    widget.outerRadius->changeValue(m_star->tipRadius());
    widget.convex->setCheckState(m_star->convex() ? Qt::Checked : Qt::Unchecked);
    typeChanged();

    widget.corners->blockSignals(false);
    widget.innerRadius->blockSignals(false);
    widget.outerRadius->blockSignals(false);
    widget.convex->blockSignals(false);
}

void StarShapeConfigWidget::save()
{
    if (! m_star)
        return;

    m_star->setCornerCount(widget.corners->value());
    m_star->setBaseRadius(widget.innerRadius->value());
    m_star->setTipRadius(widget.outerRadius->value());
    m_star->setConvex(widget.convex->checkState() == Qt::Checked);
}

QUndoCommand * StarShapeConfigWidget::createCommand()
{
    if (! m_star)
        return 0;
    else
        return new StarShapeConfigCommand(m_star, widget.corners->value(), widget.innerRadius->value(),
                widget.outerRadius->value(), widget.convex->checkState() == Qt::Checked);
}

void StarShapeConfigWidget::typeChanged()
{
    if (widget.convex->checkState() == Qt::Checked) {
        widget.innerRadius->setEnabled(false);
    } else {
        widget.innerRadius->setEnabled(true);
    }
}

#include <StarShapeConfigWidget.moc>
