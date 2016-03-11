/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_brush_hud.h"

#include <QVBoxLayout>
#include <QPointer>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollArea>
#include <QEvent>

#include "kis_uniform_paintop_property.h"
#include "kis_uniform_paintop_property_widget.h"

#include "kis_debug.h"


struct KisBrushHud::Private
{
    QPointer<QLabel> lblPresetName;
    QPointer<QWidget> wdgProperties;
    QPointer<QScrollArea> wdgPropertiesArea;
};

KisBrushHud::KisBrushHud(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint),
      m_d(new Private)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    m_d->lblPresetName = new QLabel("<Preset Name>", this);
    layout->addWidget(m_d->lblPresetName);

    m_d->wdgPropertiesArea = new QScrollArea(this);
    m_d->wdgPropertiesArea->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_d->wdgPropertiesArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_d->wdgProperties = new QWidget(this);
    QVBoxLayout *propsLayout = new QVBoxLayout(this);
    propsLayout->setSpacing(0);
    propsLayout->setContentsMargins(0, 0, 22, 0);
    propsLayout->setSizeConstraint(QLayout::SetMinAndMaxSize);

    for (int i = 0; i < 10; i++) {
        const QString name = QString("Property %1").arg(i);
        const KisUniformPaintOpProperty::Type type =
            KisUniformPaintOpProperty::Type(i%2);

        KisUniformPaintOpProperty *prop = new KisUniformPaintOpProperty(type, name, name, this);
        KisUniformPaintOpPropertyWidget *w = new KisUniformPaintOpPropertyWidget(prop, m_d->wdgProperties);

        w->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        propsLayout->addWidget(w);
    }

    m_d->wdgProperties->setLayout(propsLayout);
    m_d->wdgPropertiesArea->setWidget(m_d->wdgProperties);
    layout->addWidget(m_d->wdgPropertiesArea);

    setLayout(layout);
    setCursor(Qt::ArrowCursor);
}

KisBrushHud::~KisBrushHud()
{
}

void KisBrushHud::paintEvent(QPaintEvent *event)
{
    QColor bgColor = palette().color(QPalette::Window);

    QPainter painter(this);
    painter.fillRect(rect() & event->rect(), bgColor);
    painter.end();

    QWidget::paintEvent(event);
}

bool KisBrushHud::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TabletPress:
    case QEvent::TabletMove:
    case QEvent::TabletRelease:
    case QEvent::MouseButtonPress:
    case QEvent::MouseMove:
    case QEvent::MouseButtonRelease:
    case QEvent::Wheel:
        event->accept();
        return true;
    }

    return QWidget::event(event);
}
