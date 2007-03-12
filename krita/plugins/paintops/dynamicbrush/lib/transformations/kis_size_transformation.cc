/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
    : KisDynamicTransformation(KoID("size",i18n("Resize"))),
      m_horizTransfoParameter(hTransfoParameter), m_vertiTransfoParameter(vTransfoParameter),
      m_hmax(2.0), m_hmin(0.5), m_vmax(2.0), m_vmin(0.5)
{
}
KisSizeTransformation::~KisSizeTransformation()
{
    if(m_horizTransfoParameter != m_vertiTransfoParameter)
        delete m_vertiTransfoParameter;
    delete m_horizTransfoParameter;
}

void KisSizeTransformation::transformBrush(KisDynamicShape* dabsrc, const KisPaintInformation& info)
{
    dabsrc->resize(
        CLAMP(m_horizTransfoParameter->parameter(info), m_hmin, m_hmax),
        CLAMP( m_vertiTransfoParameter->parameter(info), m_vmin, m_vmax) );
}

void KisSizeTransformation::transformColoring(KisDynamicColoring* coloringsrc, const KisPaintInformation& info)
{
    Q_UNUSED(coloringsrc);
    Q_UNUSED(info);
    // TODO: implement it
}

void KisSizeTransformation::setHSensor(const KoID& id)
{
    kDebug() << "H: " << id.id() << endl;
    if(id != m_horizTransfoParameter->id() )
    {
        delete m_horizTransfoParameter;
        m_horizTransfoParameter = KisDynamicSensor::id2Sensor(id);
    }
}

void KisSizeTransformation::setVSensor(const KoID& id)
{
    kDebug() << "V: " << id.id() << endl;
    if(id != m_vertiTransfoParameter->id() )
    {
        delete m_vertiTransfoParameter;
        m_vertiTransfoParameter = KisDynamicSensor::id2Sensor(id);
    }
}

QWidget* KisSizeTransformation::createConfigWidget(QWidget* parent)
{
    QWidget* editorWidget = new QWidget(parent);
    Ui_SizeTransformationEditor ste;
    ste.setupUi(editorWidget);
    // Setup the horizontal parameter
    // horizontal sensor
    ste.comboBoxHorizontalSensor->setIDList( KisDynamicSensor::sensorsIds() );
    connect(ste.comboBoxHorizontalSensor, SIGNAL(activated(const KoID &)), this, SLOT(setHSensor(const KoID& )));
    ste.comboBoxHorizontalSensor->setCurrent( m_horizTransfoParameter->id() );
    // horizontal maximum
    ste.spinBoxHorizontalMaximum->setValue( m_hmax);
    connect(ste.spinBoxHorizontalMaximum, SIGNAL(valueChanged(double)), this, SLOT(setHMaximum(double)));
    // horizontal minimum
    ste.spinBoxHorizontalMinimum->setValue( m_hmin);
    connect(ste.spinBoxHorizontalMinimum, SIGNAL(valueChanged(double)), this, SLOT(setHMinimum(double)));
    // Setup the vertical parameter
    // vertical sensor
    ste.comboBoxVerticalSensor->setIDList( KisDynamicSensor::sensorsIds() );
    connect(ste.comboBoxVerticalSensor, SIGNAL(activated(const KoID &)), this, SLOT(setVSensor(const KoID& )));
    ste.comboBoxVerticalSensor->setCurrent( m_vertiTransfoParameter->id() );
    // vertical maximum
    ste.spinBoxVerticalMaximum->setValue( m_hmax);
    connect(ste.spinBoxVerticalMaximum, SIGNAL(valueChanged(double)), this, SLOT(setHMaximum(double)));
    // Vertical minimum
    ste.spinBoxVerticalMinimum->setValue( m_hmin);
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
