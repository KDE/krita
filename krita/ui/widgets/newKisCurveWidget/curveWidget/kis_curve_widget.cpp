/* This file is part of the KDE project
 * Copyright (C) 2010 Adam Celarek <kdedev at xibo dot at>
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

#include "kis_curve_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QButtonGroup>

#include "kis_spline_curve_widget.h"
#include "kis_line_curve_widget.h"
#include "kis_freehand_curve_widget.h"



KisCurveWidget::KisCurveWidget(QWidget *parent)
    : QWidget(parent),
      m_functionLikeWidget(new KisSplineCurveWidget),
      m_splineWidget(new KisSplineCurveWidget),
      m_lineWidget(new KisLineCurveWidget),
      m_freehandWidget(new KisFreehandCurveWidget)
{
    m_functionLikeWidget->hide();
    m_splineWidget->hide();
    m_lineWidget->hide();
    m_freehandWidget->hide();

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addWidget(m_functionLikeWidget);
    layout->addWidget(m_splineWidget);
    layout->addWidget(m_lineWidget);
    layout->addWidget(m_freehandWidget);

    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(0);
    buttonLayout->setMargin(0);

    QPushButton* functionlikeButton = new QPushButton("fun");
    QPushButton* splineButton = new QPushButton("spl");
    QPushButton* lineButton = new QPushButton("lin");
    QPushButton* freehandButton = new QPushButton("fre");
    QPushButton* resetButton = new QPushButton("res");

    buttonLayout->addWidget(functionlikeButton);
    buttonLayout->addWidget(splineButton);
    buttonLayout->addWidget(lineButton);
    buttonLayout->addWidget(freehandButton);

    QButtonGroup* buttonGroup = new QButtonGroup(this);

    for(int i=0; i<buttonLayout->count(); i++) {
        QPushButton* b = dynamic_cast<QPushButton*>(buttonLayout->itemAt(i)->widget());
        Q_ASSERT(b);

        b->setMaximumSize(30, 30);
        b->setMinimumSize(30, 30);
        b->setCheckable(true);
        buttonGroup->addButton(b);
    }

    buttonLayout->addStretch();
    buttonLayout->addWidget(resetButton);
    layout->addLayout(buttonLayout);

    resetButton->setMinimumSize(30, 30);
    resetButton->setMaximumSize(30, 30);

    //default is cubic curve
    m_currentCurve = m_splineWidget;
    m_currentCurve->show();
    splineButton->click();

    connect(functionlikeButton, SIGNAL(clicked()), SLOT(switchToFunction()));
    connect(splineButton,        SIGNAL(clicked()), SLOT(switchToCubic()));
    connect(lineButton,       SIGNAL(clicked()), SLOT(switchToLinear()));
    connect(freehandButton,     SIGNAL(clicked()), SLOT(switchToFreehand()));
    connect(resetButton,        SIGNAL(clicked()), SLOT(reset()));
}

KisCurveWidget::~KisCurveWidget()
{}

void KisCurveWidget::reset()
{
    m_currentCurve->reset();
}

void KisCurveWidget::switchTo(KisCurveWidgetBase* newWidget)
{
    if(m_currentCurve == newWidget) return;

    newWidget->setControlPoints(m_currentCurve->controlPoints());
    m_currentCurve->hide();
    m_currentCurve = newWidget;
    m_currentCurve->show();

}

