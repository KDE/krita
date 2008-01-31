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
#include "SnapGuide.h"
#include "SnapStrategy.h"

SnapGuideConfigWidget::SnapGuideConfigWidget( SnapGuide * snapGuide, QWidget * parent )
    :QWidget(parent), m_snapGuide(snapGuide)
{
    widget.setupUi(this);

    if( snapGuide->enabledSnapStrategies() & SnapStrategy::Orthogonal )
        widget.orthogonalSnapGuide->setCheckState( Qt::Checked );
    if( snapGuide->enabledSnapStrategies() & SnapStrategy::Node )
        widget.nodeSnapGuide->setCheckState( Qt::Checked );
    if( snapGuide->enabledSnapStrategies() & SnapStrategy::Extension )
        widget.extensionSnapGuide->setCheckState( Qt::Checked );

    widget.snapDistance->setValue( m_snapGuide->snapDistance() );

    connect( widget.useSnapGuides, SIGNAL(stateChanged(int)), this, SLOT(snappingEnabled(int)));
    connect( widget.orthogonalSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.nodeSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
    connect( widget.extensionSnapGuide, SIGNAL(stateChanged(int)), this, SLOT(strategyChanged()));
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
        strategies |= SnapStrategy::Orthogonal;
    if( widget.nodeSnapGuide->checkState() == Qt::Checked )
        strategies |= SnapStrategy::Node;
    if( widget.extensionSnapGuide->checkState() == Qt::Checked )
        strategies |= SnapStrategy::Extension;

    m_snapGuide->enableSnapStrategies( strategies );
}

void SnapGuideConfigWidget::distanceChanged( int distance )
{
    m_snapGuide->setSnapDistance( distance );
}

#include "SnapGuideConfigWidget.moc"
