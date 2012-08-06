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

#include <KoIcon.h>

SnapGuideConfigWidget::SnapGuideConfigWidget(KoSnapGuide * snapGuide, QWidget * parent)
        : QWidget(parent), m_snapGuide(snapGuide)
{
    widget.setupUi(this);

    widget.orthogonalSnapGuide->setIcon(koIcon("snap-orto"));
    widget.nodeSnapGuide->setIcon(koIcon("snap-node"));
    widget.extensionSnapGuide->setIcon(koIcon("snap-extension"));
    widget.intersectionSnapGuide->setIcon(koIcon("snap-intersection"));
    widget.boundingBoxSnapGuide->setIcon(koIcon("snap-boundingbox"));
    widget.lineGuideSnapGuide->setIcon(koIcon("snap-guideline"));

    updateControls();

    connect(widget.useSnapGuides, SIGNAL(toggled(bool)), this, SLOT(snappingEnabled(bool)));
    connect(widget.orthogonalSnapGuide, SIGNAL(toggled(bool)), this, SLOT(strategyChanged()));
    connect(widget.nodeSnapGuide, SIGNAL(toggled(bool)), this, SLOT(strategyChanged()));
    connect(widget.extensionSnapGuide, SIGNAL(toggled(bool)), this, SLOT(strategyChanged()));
    connect(widget.intersectionSnapGuide, SIGNAL(toggled(bool)), this, SLOT(strategyChanged()));
    connect(widget.boundingBoxSnapGuide, SIGNAL(toggled(bool)), this, SLOT(strategyChanged()));
    connect(widget.lineGuideSnapGuide, SIGNAL(toggled(bool)), this, SLOT(strategyChanged()));
    connect(widget.snapDistance, SIGNAL(valueChanged(int)), this, SLOT(distanceChanged(int)));

    widget.useSnapGuides->setChecked(snapGuide->isSnapping());
}

SnapGuideConfigWidget::~SnapGuideConfigWidget()
{
}

void SnapGuideConfigWidget::snappingEnabled(bool isEnabled)
{
    widget.orthogonalSnapGuide->setEnabled(isEnabled);
    widget.nodeSnapGuide->setEnabled(isEnabled);
    widget.extensionSnapGuide->setEnabled(isEnabled);
    widget.intersectionSnapGuide->setEnabled(isEnabled);
    widget.boundingBoxSnapGuide->setEnabled(isEnabled);
    widget.lineGuideSnapGuide->setEnabled(isEnabled);
    widget.snapDistance->setEnabled(isEnabled);

    m_snapGuide->enableSnapping(isEnabled);
}

void SnapGuideConfigWidget::strategyChanged()
{
    KoSnapGuide::Strategies strategies;
    if (widget.orthogonalSnapGuide->isChecked())
        strategies |= KoSnapGuide::OrthogonalSnapping;
    if (widget.nodeSnapGuide->isChecked())
        strategies |= KoSnapGuide::NodeSnapping;
    if (widget.extensionSnapGuide->isChecked())
        strategies |= KoSnapGuide::ExtensionSnapping;
    if (widget.intersectionSnapGuide->isChecked())
        strategies |= KoSnapGuide::IntersectionSnapping;
    if (widget.boundingBoxSnapGuide->isChecked())
        strategies |= KoSnapGuide::BoundingBoxSnapping;
    if (widget.lineGuideSnapGuide->isChecked())
        strategies |= KoSnapGuide::GuideLineSnapping;

    m_snapGuide->enableSnapStrategies(strategies);
}

void SnapGuideConfigWidget::distanceChanged(int distance)
{
    m_snapGuide->setSnapDistance(distance);
}

void SnapGuideConfigWidget::updateControls()
{
    const KoSnapGuide::Strategies enabledSnapStrategies = m_snapGuide->enabledSnapStrategies();

    widget.orthogonalSnapGuide->setChecked(enabledSnapStrategies & KoSnapGuide::OrthogonalSnapping);
    widget.nodeSnapGuide->setChecked(enabledSnapStrategies & KoSnapGuide::NodeSnapping);
    widget.extensionSnapGuide->setChecked(enabledSnapStrategies & KoSnapGuide::ExtensionSnapping);
    widget.intersectionSnapGuide->setChecked(enabledSnapStrategies & KoSnapGuide::IntersectionSnapping);
    widget.boundingBoxSnapGuide->setChecked(enabledSnapStrategies & KoSnapGuide::BoundingBoxSnapping);
    widget.lineGuideSnapGuide->setChecked(enabledSnapStrategies & KoSnapGuide::GuideLineSnapping);

    widget.snapDistance->setValue(m_snapGuide->snapDistance());
}

void SnapGuideConfigWidget::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    updateControls();
}

#include <SnapGuideConfigWidget.moc>
