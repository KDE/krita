/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_brush_selection_widget.h"
#include <QLayout>
#include <QTabWidget>
#include <QFrame>
#include <QImage>
#include <QPainter>
#include <QBrush>
#include <QColor>

#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <klocale.h>

#include <widgets/kis_preset_chooser.h>
#include <kis_image.h>

#include "kis_brush.h"
#include "kis_auto_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_brush_chooser.h"
#include "kis_auto_brush_widget.h"
#include "kis_custom_brush_widget.h"
#include "kis_text_brush_chooser.h"

KisBrushSelectionWidget::KisBrushSelectionWidget(QWidget * parent)
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

    m_autoBrushWidget = new KisAutoBrushWidget(0, "autobrush", i18n("Autobrush"));
    connect(m_autoBrushWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
    m_brushesTab->addTab(m_autoBrushWidget, i18n("Autobrush"));

    m_brushChooser = new KisBrushChooser(0);
    connect(m_brushChooser, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
    m_brushesTab->addTab(m_brushChooser, i18n("Predefined Brushes"));

    // XXX: pass image!
//  TODO custom brush doesn't work correctly
//    m_customBrushWidget = new KisCustomBrushWidget(0, i18n("Custom Brush"), 0);
//    connect(m_customBrushWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
//    m_brushesTab->addTab(m_customBrushWidget, i18n("Custom Brush"));

    m_textBrushWidget = new KisTextBrushChooser(0, "textbrush", i18n("Text Brush"));
    connect(m_textBrushWidget, SIGNAL(sigBrushChanged()), SIGNAL(sigBrushChanged()));
    m_brushesTab->addTab(m_textBrushWidget, i18n("Text Brush"));

    setLayout(l);

    // m_brushChooser->itemChooser()->setCurrent(0);
    m_autoBrushWidget->activate();
}


KisBrushSelectionWidget::~KisBrushSelectionWidget()
{
}

KisBrushSP KisBrushSelectionWidget::brush()
{
    KisBrushSP theBrush;
    switch (m_brushesTab->currentIndex()) {
    case 0:
        theBrush = m_autoBrushWidget->brush();
        break;
    case 1:
        theBrush = m_brushChooser->brush();
        break;
    case 2:
//  TODO custom brush doesn't work correctly
//        theBrush = m_customBrushWidget->brush();
//        break;
//    case 3:
        theBrush = m_textBrushWidget->brush();
        break;
    default:
        ;
    }
    // Fallback to auto brush if no brush selected
    // Can happen if there is no predefined brush found
    if (!theBrush)
        theBrush = m_autoBrushWidget->brush();

    return theBrush;

}


void KisBrushSelectionWidget::setAutoBrush(bool on)
{
    m_autoBrushWidget->setVisible(on);
}

void KisBrushSelectionWidget::setPredefinedBrushes(bool on)
{
    m_brushChooser->setVisible(on);
}

void KisBrushSelectionWidget::setCustomBrush(bool on)
{
    Q_UNUSED(on);
//    m_customBrushWidget->setVisible( on );
}

void KisBrushSelectionWidget::setTextBrush(bool on)
{
    m_textBrushWidget->setVisible(on);
}

void KisBrushSelectionWidget::setImage(KisImageWSP image)
{
    Q_UNUSED(image);
//    m_customBrushWidget->setImage(image);
}

void KisBrushSelectionWidget::setCurrentBrush(KisBrushSP brush)
{
    // XXX: clever code have brush plugins know their configuration
    //      pane, so we don't have to have this if statement and
    //      have an extensible set of brush types
    if (dynamic_cast<KisAutoBrush*>(brush.data())) {
        m_brushesTab->setCurrentWidget(m_autoBrushWidget);
        m_autoBrushWidget->setBrush(brush);
    } else if (dynamic_cast<KisTextBrush*>(brush.data())) {
        m_brushesTab->setCurrentWidget(m_textBrushWidget);
        m_textBrushWidget->setBrush(brush);
    } else {
        m_brushesTab->setCurrentWidget(m_brushChooser);
        m_brushChooser->setBrush(brush);
    }

}



void KisBrushSelectionWidget::setAutoBrushDiameter(qreal diameter)
{
    m_autoBrushWidget->setAutoBrushDiameter(diameter);
}


qreal KisBrushSelectionWidget::autoBrushDiameter()
{
    return m_autoBrushWidget->autoBrushDiameter();
}


#include "kis_brush_selection_widget.moc"
