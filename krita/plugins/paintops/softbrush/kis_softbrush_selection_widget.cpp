/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_softbrush_selection_widget.h"
#include <QLayout>
#include <QTabWidget>
#include <QFrame>
#include <QImage>
#include <QPainter>
#include <QBrush>
#include <QColor>

#include <klocale.h>

KisSoftBrushSelectionWidget::KisSoftBrushSelectionWidget(QWidget * parent)
        : QWidget(parent)
{
    QHBoxLayout * l = new QHBoxLayout(this);
    l->setObjectName("brushpopuplayout");
    l->setMargin(2);
    l->setSpacing(2);

    m_brushesTab = new QTabWidget(this);
    m_brushesTab->setObjectName("brushestab");
    m_brushesTab->setFocusPolicy(Qt::StrongFocus);
    m_brushesTab->setContentsMargins(1, 1, 1, 1);

    l->addWidget(m_brushesTab);

    m_curveBrushTip = new KisSoftCurveOptionsWidget();
    m_brushesTab->addTab(m_curveBrushTip, i18n("Curve"));
    
    m_gaussBrushTip = new KisSoftOpOptionsWidget();
    m_brushesTab->addTab(m_gaussBrushTip, i18n("Gaussian"));

    setLayout(l);
}


KisSoftBrushSelectionWidget::~KisSoftBrushSelectionWidget()
{
    delete m_curveBrushTip;
    delete m_gaussBrushTip;
}



void KisSoftBrushSelectionWidget::setCurveBrush(bool on)
{
    m_curveBrushTip->setVisible(on);
}

void KisSoftBrushSelectionWidget::setGaussianBrush(bool on)
{
    m_gaussBrushTip->setVisible(on);
}

qreal KisSoftBrushSelectionWidget::currentBrushSize()
{
    //TODO:
}


void KisSoftBrushSelectionWidget::setCurrentBrushSize(qreal size)
{
    //TODO:
    Q_UNUSED(size);
}


#include "kis_softbrush_selection_widget.moc"
