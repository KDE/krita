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

#include "kis_brush.h"
#include "kis_auto_brush.h"
#include "kis_imagepipe_brush.h"
#include "kis_brush_chooser.h"
#include "kis_auto_brush_widget.h"
#include "kis_custom_brush.h"
#include "kis_text_brush.h"
#include <kis_image.h>

KisBrushSelectionWidget::KisBrushSelectionWidget( QWidget * parent )
    : QWidget( parent )
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

    KisAutoBrushWidget * m_autoBrushWidget = new KisAutoBrushWidget(0, "autobrush", i18n("Autobrush"));
    m_brushesTab->addTab( m_autoBrushWidget, i18n("Autobrush"));

    m_brushChooser = new KisBrushChooser(m_brushesTab);
    m_brushesTab->addTab( m_brushChooser, i18n("Predefined Brushes"));

    // XXX: pass image!
    KisCustomBrush* customBrushes = new KisCustomBrush(0, i18n("Custom Brush"), 0);
    m_brushesTab->addTab( customBrushes, i18n("Custom Brush"));

    KisTextBrush* textBrushes = new KisTextBrush(0, "textbrush",
            i18n("Text Brush")/*, m_view*/);
    m_brushesTab->addTab( textBrushes, i18n("Text Brush"));

    setLayout(l);

    m_brushChooser->setCurrent( 0 );
    m_autoBrushWidget->activate();

    QImage img( 100, 100, QImage::Format_ARGB32 );
    QPainter p( &img );
    p.setRenderHint( QPainter::Antialiasing );
    p.fillRect( 0, 0, 100, 100, QBrush(QColor( 255, 255, 255, 0) ) );
    p.setBrush( QBrush( QColor( 0, 0, 0, 255 ) ) );
    p.drawEllipse( 0, 0, 100, 100 );
    p.end();

    m_activeBrush = new KisBrush( img );

}


KisBrushSelectionWidget::~KisBrushSelectionWidget()
{
}

KisBrush * KisBrushSelectionWidget::brush()
{
    return m_activeBrush;
}
