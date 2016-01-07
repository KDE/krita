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

#include "EllipseShapeConfigWidget.h"
#include "EllipseShapeConfigCommand.h"
#include <klocalizedstring.h>

EllipseShapeConfigWidget::EllipseShapeConfigWidget()
{
    widget.setupUi(this);

    widget.ellipseType->clear();
    widget.ellipseType->addItem(i18n("Arc"));
    widget.ellipseType->addItem(i18n("Pie"));
    widget.ellipseType->addItem(i18n("Chord"));

    widget.startAngle->setMinimum(0.0);
    widget.startAngle->setMaximum(360.0);

    widget.endAngle->setMinimum(0.0);
    widget.endAngle->setMaximum(360.0);

    connect(widget.ellipseType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(propertyChanged()));
    connect(widget.startAngle, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
    connect(widget.endAngle, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
    connect(widget.closeEllipse, SIGNAL(clicked(bool)), this, SLOT(closeEllipse()));
}

void EllipseShapeConfigWidget::open(KoShape *shape)
{
    m_ellipse = dynamic_cast<EllipseShape *>(shape);
    if (!m_ellipse) {
        return;
    }

    widget.ellipseType->blockSignals(true);
    widget.startAngle->blockSignals(true);
    widget.endAngle->blockSignals(true);

    widget.ellipseType->setCurrentIndex(m_ellipse->type());
    widget.startAngle->setValue(m_ellipse->startAngle());
    widget.endAngle->setValue(m_ellipse->endAngle());

    widget.ellipseType->blockSignals(false);
    widget.startAngle->blockSignals(false);
    widget.endAngle->blockSignals(false);
}

void EllipseShapeConfigWidget::save()
{
    if (!m_ellipse) {
        return;
    }

    m_ellipse->setType(static_cast<EllipseShape::EllipseType>(widget.ellipseType->currentIndex()));
    m_ellipse->setStartAngle(widget.startAngle->value());
    m_ellipse->setEndAngle(widget.endAngle->value());
}

KUndo2Command *EllipseShapeConfigWidget::createCommand()
{
    if (!m_ellipse) {
        return 0;
    } else {
        EllipseShape::EllipseType type = static_cast<EllipseShape::EllipseType>(widget.ellipseType->currentIndex());
        return new EllipseShapeConfigCommand(m_ellipse, type, widget.startAngle->value(), widget.endAngle->value());
    }
}

void EllipseShapeConfigWidget::closeEllipse()
{
    widget.startAngle->blockSignals(true);
    widget.endAngle->blockSignals(true);

    widget.startAngle->setValue(0.0);
    widget.endAngle->setValue(360.0);

    widget.startAngle->blockSignals(false);
    widget.endAngle->blockSignals(false);

    emit propertyChanged();
}
