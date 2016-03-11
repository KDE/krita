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

#include "kis_uniform_paintop_property_widget.h"

#include <QVBoxLayout>

#include "kis_slider_spin_box.h"
#include "kis_acyclic_signal_connector.h"

struct KisUniformPaintOpPropertyWidget::Private
{
    Private(KisUniformPaintOpProperty *_property)
        : property(_property),
          type(_property->type()){}

    typedef KisUniformPaintOpProperty::Type Type;
    KisUniformPaintOpProperty *property;
    Type type;
};

KisUniformPaintOpPropertyWidget::KisUniformPaintOpPropertyWidget(KisUniformPaintOpProperty *property, QWidget *parent)
    : QWidget(parent),
      m_d(new Private(property))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    KisAcyclicSignalConnector *conn = new KisAcyclicSignalConnector(this);

    const QString prefix = QString("%1: ").arg(property->name());

    switch (m_d->type) {
    case Private::Type::Int: {
        KisSliderSpinBox *slider = new KisSliderSpinBox(this);
        slider->setBlockUpdateSignalOnDrag(true);
        slider->setRange(0, 100);
        slider->setSingleStep(1);
        slider->setPageStep(10);
        slider->setPrefix(prefix);
        slider->setSuffix("px");
        slider->setValue(property->valueInt());

        conn->connectForwardInt(property, SIGNAL(sigValueIntChanged(int)),
                                slider, SLOT(setValue(int)));

        conn->connectBackwardInt(slider, SIGNAL(valueChanged(int)),
                                 property, SLOT(setValueInt(int)));

        layout->addWidget(slider);
        break;
    }
    case Private::Type::Double: {
        KisDoubleSliderSpinBox *slider = new KisDoubleSliderSpinBox(this);
        slider->setBlockUpdateSignalOnDrag(true);
        slider->setRange(0, 100, 2);
        slider->setSingleStep(0.1);
        slider->setPrefix(prefix);
        slider->setSuffix("%");
        slider->setValue(property->valueDouble());

        conn->connectForwardDouble(property, SIGNAL(sigValueDoubleChanged(qreal)),
                                   slider, SLOT(setValue(qreal)));
        conn->connectBackwardDouble(slider, SIGNAL(valueChanged(qreal)),
                                    property, SLOT(setValueDouble(qreal)));

        layout->addWidget(slider);
        break;
    }
    case Private::Type::Bool:
        break;
    case Private::Type::Combo:
        break;
    }

    setLayout(layout);
}

KisUniformPaintOpPropertyWidget::~KisUniformPaintOpPropertyWidget()
{
}
