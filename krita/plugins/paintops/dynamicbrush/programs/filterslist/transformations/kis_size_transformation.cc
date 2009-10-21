/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_size_transformation.h"
#include <KoIntegerMaths.h>

#include "kis_dynamic_shape.h"
#include "kis_dynamic_sensor.h"

#include "ui_SizeTransformationEditor.h"

KisSizeTransformation::KisSizeTransformation(KisDynamicSensor* hTransfoParameter, KisDynamicSensor* vTransfoParameter)
        : KisDynamicTransformation(KisDynamicTransformation::SizeTransformationID),
        m_horizTransfoParameter(hTransfoParameter), m_vertiTransfoParameter(vTransfoParameter),
        m_hmax(2.0), m_hmin(0.5), m_vmax(2.0), m_vmin(0.5)
{
}
KisSizeTransformation::~KisSizeTransformation()
{
    if (m_horizTransfoParameter != m_vertiTransfoParameter)
        delete m_vertiTransfoParameter;
    delete m_horizTransfoParameter;
}

void KisSizeTransformation::transformBrush(KisDynamicShape* dabsrc, const KisPaintInformation& info)
{
    dabsrc->resize(m_horizTransfoParameter->parameter(info) *(m_hmax - m_hmin) + m_hmin,
                   m_vertiTransfoParameter->parameter(info) *(m_vmax - m_vmin) + m_vmin);
}

void KisSizeTransformation::transformColoring(KisDynamicColoring* coloringsrc, const KisPaintInformation& info)
{
    Q_UNUSED(coloringsrc);
    Q_UNUSED(info);
    // TODO: implement it
}

void KisSizeTransformation::setHSensor(KisDynamicSensor* sensor)
{
    delete m_horizTransfoParameter;
    m_horizTransfoParameter = sensor;
}

void KisSizeTransformation::setVSensor(KisDynamicSensor* sensor)
{
    delete m_vertiTransfoParameter;
    m_vertiTransfoParameter = sensor;
}

QWidget* KisSizeTransformation::createConfigWidget(QWidget* parent)
{
    QWidget* editorWidget = new QWidget(parent);
    Ui_SizeTransformationEditor ste;
    ste.setupUi(editorWidget);
    // Setup the horizontal parameter
    // horizontal sensor
    connect(ste.comboBoxHorizontalSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), this, SLOT(setHSensor(KisDynamicSensor*)));
    ste.comboBoxHorizontalSensor->setCurrent(m_horizTransfoParameter);
    // horizontal maximum
    ste.spinBoxHorizontalMaximum->setValue(m_hmax);
    connect(ste.spinBoxHorizontalMaximum, SIGNAL(valueChanged(double)), this, SLOT(setHMaximum(double)));
    // horizontal minimum
    ste.spinBoxHorizontalMinimum->setValue(m_hmin);
    connect(ste.spinBoxHorizontalMinimum, SIGNAL(valueChanged(double)), this, SLOT(setHMinimum(double)));
    // Setup the vertical parameter
    // vertical sensor
    connect(ste.comboBoxVerticalSensor, SIGNAL(sensorChanged(KisDynamicSensor*)), this, SLOT(setVSensor(KisDynamicSensor*)));
    ste.comboBoxVerticalSensor->setCurrent(m_vertiTransfoParameter);
    // vertical maximum
    ste.spinBoxVerticalMaximum->setValue(m_hmax);
    connect(ste.spinBoxVerticalMaximum, SIGNAL(valueChanged(double)), this, SLOT(setHMaximum(double)));
    // Vertical minimum
    ste.spinBoxVerticalMinimum->setValue(m_hmin);
    connect(ste.spinBoxVerticalMinimum, SIGNAL(valueChanged(double)), this, SLOT(setHMinimum(double)));
    return editorWidget;
}

void KisSizeTransformation::setHMaximum(double v)
{
    m_hmax = v;
}
void KisSizeTransformation::setVMaximum(double v)
{
    m_vmax = v;
}
void KisSizeTransformation::setHMinimum(double v)
{
    m_hmin = v;
}
void KisSizeTransformation::setVMinimum(double v)
{
    m_vmin = v;
}

#include "kis_size_transformation.moc"
