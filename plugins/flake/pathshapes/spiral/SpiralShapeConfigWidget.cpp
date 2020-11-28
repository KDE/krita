/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Rob Buis <buis@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SpiralShapeConfigWidget.h"
#include "SpiralShapeConfigCommand.h"
#include <klocalizedstring.h>

SpiralShapeConfigWidget::SpiralShapeConfigWidget()
{
    widget.setupUi(this);

    widget.spiralType->clear();
    widget.spiralType->addItem(i18n("Curve"));
    widget.spiralType->addItem(i18n("Line"));

    widget.fade->setMinimum(0.0);
    widget.fade->setMaximum(1.0);

    widget.clockWise->clear();
    widget.clockWise->addItem(i18n("Clockwise"));
    widget.clockWise->addItem(i18n("Anticlockwise"));

    connect(widget.spiralType, SIGNAL(currentIndexChanged(int)), this, SIGNAL(propertyChanged()));
    connect(widget.clockWise, SIGNAL(currentIndexChanged(int)), this, SIGNAL(propertyChanged()));
    connect(widget.fade, SIGNAL(editingFinished()), this, SIGNAL(propertyChanged()));
}

void SpiralShapeConfigWidget::open(KoShape *shape)
{
    m_spiral = dynamic_cast<SpiralShape *>(shape);
    if (!m_spiral) {
        return;
    }

    widget.spiralType->blockSignals(true);
    widget.clockWise->blockSignals(true);
    widget.fade->blockSignals(true);

    widget.spiralType->setCurrentIndex(m_spiral->type());
    widget.clockWise->setCurrentIndex(m_spiral->clockWise() ? 0 : 1);
    widget.fade->setValue(m_spiral->fade());

    widget.spiralType->blockSignals(false);
    widget.clockWise->blockSignals(false);
    widget.fade->blockSignals(false);
}

void SpiralShapeConfigWidget::save()
{
    if (!m_spiral) {
        return;
    }

    m_spiral->setType(static_cast<SpiralShape::SpiralType>(widget.spiralType->currentIndex()));
    m_spiral->setClockWise(widget.clockWise->currentIndex() == 0);
    m_spiral->setFade(widget.fade->value());
}

KUndo2Command *SpiralShapeConfigWidget::createCommand()
{
    if (!m_spiral) {
        return 0;
    }
    SpiralShape::SpiralType type = static_cast<SpiralShape::SpiralType>(widget.spiralType->currentIndex());
    return new SpiralShapeConfigCommand(m_spiral, type, (widget.clockWise->currentIndex() == 0), widget.fade->value());
}
