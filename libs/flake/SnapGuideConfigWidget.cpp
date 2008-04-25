/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "SnapGuideConfigWidget.h"
#include "KoSnapGuide.h"
#include "KoSnapStrategy.h"

SnapGuideConfigWidget::SnapGuideConfigWidget( KoSnapGuide * snapGuide, QWidget * parent )
    :QWidget(parent), m_snapGuide(snapGuide)
{
    widget.setupUi(this);

    updateControls();

    connect( widget.useSnapGuides, SIGNAL(stateChanged(int)), this, SLOT(snappingEnabled(int)));
    connect( widget.orthogonalSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.nodeSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.extensionSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.intersectionSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.boundingBoxSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.snapDistance, SIGNAL(valueChanged(int)), this, SLOT(distanceChanged(int)));

    widget.useSnapGuides->setCheckState( snapGuide->isSnapping() ? Qt::Checked : Qt::Unchecked );
}

SnapGuideConfigWidget::~SnapGuideConfigWidget()
{
}

void SnapGuideConfigWidget::snappingEnabled( int state )
{
    widget.strategyPanel->setEnabled( state == QCheckBox::On );
    m_snapGuide->enableSnapping( state == QCheckBox::On );
}

void SnapGuideConfigWidget::strategyChanged()
{
    int strategies = 0;
    if( widget.orthogonalSnapGuide->checkState() == Qt::Checked )
        strategies |= KoSnapStrategy::Orthogonal;
    if( widget.nodeSnapGuide->checkState() == Qt::Checked )
        strategies |= KoSnapStrategy::Node;
    if( widget.extensionSnapGuide->checkState() == Qt::Checked )
        strategies |= KoSnapStrategy::Extension;
    if( widget.intersectionSnapGuide->checkState() == Qt::Checked )
        strategies |= KoSnapStrategy::Intersection;
    if( widget.boundingBoxSnapGuide->checkState() == Qt::Checked )
        strategies |= KoSnapStrategy::BoundingBox;

    m_snapGuide->enableSnapStrategies( strategies );
}

void SnapGuideConfigWidget::distanceChanged( int distance )
{
    m_snapGuide->setSnapDistance( distance );
}

void SnapGuideConfigWidget::updateControls()
{
    if( m_snapGuide->enabledSnapStrategies() & KoSnapStrategy::Orthogonal )
        widget.orthogonalSnapGuide->setCheckState( Qt::Checked );
    else
        widget.orthogonalSnapGuide->setCheckState( Qt::Unchecked );
    if( m_snapGuide->enabledSnapStrategies() & KoSnapStrategy::Node )
        widget.nodeSnapGuide->setCheckState( Qt::Checked );
    else
        widget.nodeSnapGuide->setCheckState( Qt::Unchecked );
    if( m_snapGuide->enabledSnapStrategies() & KoSnapStrategy::Extension )
        widget.extensionSnapGuide->setCheckState( Qt::Checked );
    else
        widget.extensionSnapGuide->setCheckState( Qt::Unchecked );
    if( m_snapGuide->enabledSnapStrategies() & KoSnapStrategy::Intersection )
        widget.intersectionSnapGuide->setCheckState( Qt::Checked );
    else
        widget.intersectionSnapGuide->setCheckState( Qt::Unchecked );
    if( m_snapGuide->enabledSnapStrategies() & KoSnapStrategy::BoundingBox )
        widget.boundingBoxSnapGuide->setCheckState( Qt::Checked );
    else
        widget.boundingBoxSnapGuide->setCheckState( Qt::Unchecked );

    widget.snapDistance->setValue( m_snapGuide->snapDistance() );
}

void SnapGuideConfigWidget::showEvent( QShowEvent * event )
{
    Q_UNUSED(event);
    updateControls();
}

#include "SnapGuideConfigWidget.moc"
