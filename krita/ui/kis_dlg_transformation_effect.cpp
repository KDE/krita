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

#include <QGridLayout>
#include "kis_mask_widgets.h"
#include "kis_filter_strategy.h"

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

    QWidget * page = new QWidget( this );
    m_grid = new QGridLayout( page );

    m_maskSource = new WdgMaskSource( page );
    m_grid->addWidget( m_maskSource, 0, 0 );

    m_maskFromSelection = new WdgMaskFromSelection( page );
    m_grid->addWidget( m_maskFromSelection, 1, 0 );

    m_transformEffectWidget = new WdgTransformationEffect( page );
    m_transformEffectWidget->layout()->setMargin(0);
    m_grid->addWidget( m_transformEffectWidget, 0, 1, 2, 1 );

    setMainWidget( page );

    m_transformEffectWidget->Ui::WdgTransformationEffect::maskName->setText( maskName );
    m_transformEffectWidget->dblScaleX->setValue( xScale );
    m_transformEffectWidget->dblScaleY->setValue( yScale );
    m_transformEffectWidget->dblShearX->setValue( xShear );
    m_transformEffectWidget->dblShearY->setValue( yShear );
    m_transformEffectWidget->dblRotation->setValue( angle );
    m_transformEffectWidget->intMoveX->setValue( moveX );
    m_transformEffectWidget->intMoveY->setValue( moveY );

    m_transformEffectWidget->cmbFilter->clear();
    m_transformEffectWidget->cmbFilter->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_transformEffectWidget->cmbFilter->setCurrent(filterId);

}



#include "kis_dlg_transformation_effect.moc"
