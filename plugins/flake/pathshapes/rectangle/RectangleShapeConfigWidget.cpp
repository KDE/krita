/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "RectangleShapeConfigWidget.h"
#include "RectangleShape.h"
#include "RectangleShapeConfigCommand.h"
#include "kis_signals_blocker.h"
#include "kis_assert.h"

#include "kis_document_aware_spin_box_unit_manager.h"

RectangleShapeConfigWidget::RectangleShapeConfigWidget()
    : m_rectangle(0)
{
    widget.setupUi(this);

    connect(widget.cornerRadiusX, SIGNAL(valueChangedPt(qreal)), this, SIGNAL(propertyChanged()));
    connect(widget.cornerRadiusY, SIGNAL(valueChangedPt(qreal)), this, SIGNAL(propertyChanged()));
}

void RectangleShapeConfigWidget::setUnit(const KoUnit &unit)
{
    widget.cornerRadiusX->setUnit(unit);
    widget.cornerRadiusY->setUnit(unit);
}

void RectangleShapeConfigWidget::open(KoShape *shape)
{
    if (m_rectangle) {
        m_rectangle->removeShapeChangeListener(this);
    }

    m_rectangle = dynamic_cast<RectangleShape *>(shape);
    if (!m_rectangle) return;

    loadPropertiesFromShape(m_rectangle);

    m_rectangle->addShapeChangeListener(this);
}

void RectangleShapeConfigWidget::loadPropertiesFromShape(RectangleShape *shape)
{
    KisSignalsBlocker b(widget.cornerRadiusX, widget.cornerRadiusY);

    QSizeF size = shape->size();

    widget.cornerRadiusX->setMaximum(0.5 * size.width());
    widget.cornerRadiusX->changeValue(0.01 * shape->cornerRadiusX() * 0.5 * size.width());
    widget.cornerRadiusY->setMaximum(0.5 * size.height());
    widget.cornerRadiusY->changeValue(0.01 * shape->cornerRadiusY() * 0.5 * size.height());
}

void RectangleShapeConfigWidget::save()
{
    if (!m_rectangle) {
        return;
    }

    QSizeF size = m_rectangle->size();

    m_rectangle->setCornerRadiusX(100.0 * widget.cornerRadiusX->value() / (0.5 * size.width()));
    m_rectangle->setCornerRadiusY(100.0 * widget.cornerRadiusY->value() / (0.5 * size.height()));
}

KUndo2Command *RectangleShapeConfigWidget::createCommand()
{
    if (!m_rectangle) {
        return 0;
    }
    QSizeF size = m_rectangle->size();

    qreal cornerRadiusX = 100.0 * widget.cornerRadiusX->value() / (0.5 * size.width());
    qreal cornerRadiusY = 100.0 * widget.cornerRadiusY->value() / (0.5 * size.height());

    return new RectangleShapeConfigCommand(m_rectangle, cornerRadiusX, cornerRadiusY);
}

void RectangleShapeConfigWidget::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_rectangle && shape == m_rectangle);

    if (type == KoShape::ParameterChanged) {
        loadPropertiesFromShape(m_rectangle);
    }
}
