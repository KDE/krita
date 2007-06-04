/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_dlg_transformation_effect.h"

#include "kis_filter_strategy.h"
#include <knuminput.h>

KisDlgTransformationEffect::KisDlgTransformationEffect(const QString & maskName,
                                                       double xScale,
                                                       double yScale,
                                                       double xShear,
                                                       double yShear,
                                                       double angle,
                                                       int moveX,
                                                       int moveY,
                                                       const KoID & filterId,
                                                       QWidget *parent,
                                                       const char *name)
    : KDialog( parent )
{
    setCaption( "New Transformation Mask" );
    setButtons( Ok| Cancel );
    setDefaultButton( Ok );
    setObjectName(name);
    m_page = new WdgTransformationEffect(this);
    m_page->layout()->setMargin(0);
    setMainWidget(m_page);

    m_page->maskName->setText( maskName );
    m_page->dblScaleX->setValue( xScale );
    m_page->dblScaleY->setValue( yScale );
    m_page->dblShearX->setValue( xShear );
    m_page->dblShearY->setValue( yShear );
    m_page->dblRotation->setValue( angle );
    m_page->intMoveX->setValue( moveX );
    m_page->intMoveY->setValue( moveY );

    m_page->cmbFilter->clear();
    m_page->cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->cmbFilter->setCurrent(filterId);


}

QString KisDlgTransformationEffect::maskName() const
{
    return m_page->maskName->text();
}

double KisDlgTransformationEffect::xScale() const
{
    return m_page->dblScaleX->value();
}

double KisDlgTransformationEffect::yScale() const
{
    return m_page->dblScaleY->value();
}

double KisDlgTransformationEffect::xShear() const
{
    return m_page->dblShearX->value();
}

double KisDlgTransformationEffect::yShear() const
{
    return m_page->dblShearY->value();
}

double KisDlgTransformationEffect::rotation() const
{
    return m_page->dblRotation->value();
}
int KisDlgTransformationEffect::moveX() const
{
    return m_page->intMoveX->value();
}

int KisDlgTransformationEffect::moveY() const
{
    return m_page->intMoveY->value();
}

KisFilterStrategy * KisDlgTransformationEffect::filterStrategy()
{
    KoID filterID = m_page->cmbFilter->currentItem();
    return KisFilterStrategyRegistry::instance()->value(filterID.id());
}


#include "kis_dlg_transformation_effect.moc"
