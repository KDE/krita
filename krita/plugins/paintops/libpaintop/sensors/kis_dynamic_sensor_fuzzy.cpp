/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
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


#include "kis_dynamic_sensor_fuzzy.h"

#include <QDomElement>
#include <kis_paint_information.h>

#include <QCheckBox>
#include <QHBoxLayout>


KisDynamicSensorFuzzy::KisDynamicSensorFuzzy() : KisDynamicSensor(FuzzyId)
{
    setMinimumLabel(QString());
    setMaximumLabel(QString());
}

QWidget* KisDynamicSensorFuzzy::createConfigurationWidget(QWidget* parent, QWidget *ss)
{
    QWidget *w = new QWidget(parent);

    QCheckBox *rotationModeEnabled = new QCheckBox(i18n("Positive and\nNegative\nRotation"), w);

    connect(rotationModeEnabled, SIGNAL(stateChanged(int)), SLOT(setRotationModeEnabled(int)));
    connect(rotationModeEnabled, SIGNAL(stateChanged(int)), ss, SIGNAL(parametersChanged()));

    rotationModeEnabled->setChecked(m_rotationModeEnabled);

    QVBoxLayout* l = new QVBoxLayout(w);
    l->addWidget(rotationModeEnabled);

    w->setLayout(l);
    return w;
}
bool KisDynamicSensorFuzzy::rotationModeEnabled() const
{
    return m_rotationModeEnabled;
}

void KisDynamicSensorFuzzy::setRotationModeEnabled(int state)
{
    m_rotationModeEnabled = state;
}
void KisDynamicSensorFuzzy::toXML(QDomDocument &doc, QDomElement &e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("rotationModeEnabled", m_rotationModeEnabled);

}

void KisDynamicSensorFuzzy::fromXML(const QDomElement &e)
{
    KisDynamicSensor::fromXML(e);
    m_rotationModeEnabled = e.attribute("rotationModeEnabled", "0").toInt();

}
bool KisDynamicSensorFuzzy::dependsOnCanvasRotation() const
{
    return false;
}


