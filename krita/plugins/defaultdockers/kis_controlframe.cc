/*
 *  kis_controlframe.cc - part of Krita
 *
 *  Copyright (c) 1999 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Sven Langkamp  <longamp@reallygood.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>

#include <qapplication.h>
#include <qlayout.h>
#include <qtabwidget.h>
#include <qframe.h>
#include <qwidget.h>
#include <qevent.h>
#include <qtimer.h>

#include <kmainwindow.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <koFrameButton.h>

#include "kis_factory.h"
#include "kis_controlframe.h"
#include "kis_resource_mediator.h"
#include "kis_itemchooser.h"
#include "kis_pattern_chooser.h"
#include "kis_gradient_chooser.h"
#include "kis_icon_item.h"
#include "kis_iconwidget.h"
#include "kis_brush.h"
#include "kis_pattern.h"
#include "kis_gradient.h"
#include "kis_brush_chooser.h"
#include "kis_view.h"
#include "kis_toolbox.h"
#include "kis_autobrush.h"
#include "kis_autogradient.h"
#include "kis_paintop_box.h"

KisPopupFrame::KisPopupFrame(QWidget * parent, const char* name)
    : QPopupMenu(parent, name)
{
    setFocusPolicy(StrongFocus);
}

void KisPopupFrame::keyPressEvent(QKeyEvent * e)
{
    if (e->key()== Qt::Key_Escape)
    {
        hide();
        e->accept();
    }
    else {
        e->ignore();
    }
}


KisControlFrame::KisControlFrame( KisView * view, QWidget* parent, const char* name )
    : QFrame( parent, name )
    , m_view(view)
    , m_brushWidget(0)
    , m_patternWidget(0)
    , m_gradientWidget(0)
    , m_brushChooserPopup(0)
    , m_patternChooserPopup(0)
    , m_gradientChooserPopup(0)
    , m_brushMediator(0)
    , m_patternMediator(0)
    , m_gradientMediator(0)
    , m_paintopBox(0)
{
    setFrameStyle(Panel | Raised);
    setLineWidth(1);

    m_font  = KGlobalSettings::generalFont();
    float ps = m_font.pointSize() * 0.8;
    m_font.setPointSize((int)ps);

    m_brushWidget = new KisIconWidget(this, "brushes");
    m_brushWidget->setTextLabel( i18n("Brush shapes") );
    m_brushWidget->show();

    m_patternWidget = new KisIconWidget(this, "patterns");
    m_patternWidget->setTextLabel( i18n("Fill patterns") );
    m_patternWidget->show();

    m_gradientWidget = new KisIconWidget(this, "gradients");
    m_gradientWidget->setTextLabel( i18n("Gradients") );
    m_gradientWidget->show();

    m_paintopBox = new KisPaintopBox( m_view, this, "paintopbox" );

    QVBoxLayout * vl = new QVBoxLayout( this, 1, 2, "controldocker main layout" );
    QHBoxLayout * hl = new QHBoxLayout( vl, 2, "controldocker button frame layout" );
    //hl->addItem( new QSpacerItem(10, 0, QSizePolicy::Expanding));
    hl->addWidget( m_brushWidget );
    hl->addWidget( m_patternWidget );
    hl->addWidget( m_gradientWidget );
    hl->addItem( new QSpacerItem(10, 0, QSizePolicy::Expanding));
    vl->addLayout( hl );
    vl->addWidget( m_paintopBox );
    vl->addItem( new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Expanding));

    m_brushWidget -> setFixedSize( 32, 32 );
    m_patternWidget -> setFixedSize( 32, 32 );
    m_gradientWidget -> setFixedSize( 32, 32 );




    createBrushesChooser(m_view);
    createPatternsChooser(m_view);
    createGradientsChooser(m_view);

    m_brushWidget->setPopup(m_brushChooserPopup);
    m_brushWidget->setPopupDelay(1);
    m_patternWidget->setPopup(m_patternChooserPopup);
    m_patternWidget->setPopupDelay(1);
    m_gradientWidget->setPopup(m_gradientChooserPopup);
    m_gradientWidget->setPopupDelay(1);
}


void KisControlFrame::slotSetBrush(KoIconItem *item)
{
    if (item)
        m_brushWidget -> slotSetItem(*item);
}

void KisControlFrame::slotSetPattern(KoIconItem *item)
{
    if (item)
        m_patternWidget -> slotSetItem(*item);
}

void KisControlFrame::slotSetGradient(KoIconItem *item)
{
    if (item)
        m_gradientWidget -> slotSetItem(*item);
}

void KisControlFrame::slotBrushChanged(KisBrush * brush)
{
        KisIconItem *item;

        if((item = m_brushMediator -> itemFor(brush)))
        {
                slotSetBrush(item);
        } else {
                slotSetBrush( new KisIconItem(brush) );
        }

}

void KisControlFrame::slotPatternChanged(KisPattern * pattern)
{
        KisIconItem *item;
        if (!pattern)
                return;

        if ( (item = m_patternMediator -> itemFor(pattern)) )
                slotSetPattern(item);
        else
                slotSetPattern( new KisIconItem(pattern) );
}


void KisControlFrame::slotGradientChanged(KisGradient * gradient)
{
        KisIconItem *item;
        if (!gradient)
                return;

        if ( (item = m_gradientMediator -> itemFor(gradient)) )
                slotSetGradient(item);
        else
                slotSetGradient( new KisIconItem(gradient) );
}

void KisControlFrame::createBrushesChooser(KisView * view)
{

    m_brushChooserPopup = new KisPopupFrame(m_brushWidget, "brush_chooser_popup");

    QHBoxLayout * l = new QHBoxLayout(m_brushChooserPopup, 2, 2, "brushpopuplayout");

    QTabWidget * m_brushesTab = new QTabWidget(m_brushChooserPopup, "brushestab");
    m_brushesTab->setTabShape(QTabWidget::Triangular);
    m_brushesTab->setFocusPolicy(QWidget::NoFocus);
    m_brushesTab->setFont(m_font);
    m_brushesTab->setMargin(1);

    l->add(m_brushesTab);

    KisBrushChooser * m_brushChooser = new KisBrushChooser(m_brushesTab, "brush_chooser");
    m_brushesTab->addTab( m_brushChooser, i18n("Predefined Brushes"));

    KisAutobrush * m_autobrush = new KisAutobrush(m_brushesTab, "autobrush", i18n("Autobrush"));
    m_brushesTab->addTab( m_autobrush, i18n("Autobrush"));
    connect(m_autobrush, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated( KisResource* )));

    m_brushChooser->setFont(m_font);
    m_brushMediator = new KisResourceMediator( m_brushChooser, this);
    connect(m_brushMediator, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisFactory::rServerRegistry() -> get("ImagePipeBrushServer");
    m_brushMediator -> connectServer(rServer);
    rServer = KisFactory::rServerRegistry() -> get("BrushServer");
    m_brushMediator -> connectServer(rServer);

    KisControlFrame::connect(view, SIGNAL(brushChanged(KisBrush *)), this, SLOT(slotBrushChanged( KisBrush *)));
    m_brushChooser->setCurrent( 0 );
    m_brushMediator->setActiveItem( m_brushChooser->currentItem() );

}

void KisControlFrame::createPatternsChooser(KisView * view)
{
    m_patternChooserPopup = new KisPopupFrame(m_patternWidget, "pattern_chooser_popup");

    QHBoxLayout * l2 = new QHBoxLayout(m_patternChooserPopup, 2, 2, "patternpopuplayout");

    QTabWidget * m_patternsTab = new QTabWidget(m_patternChooserPopup, "patternstab");
    m_patternsTab->setTabShape(QTabWidget::Triangular);
    m_patternsTab->setFocusPolicy(QWidget::NoFocus);
    m_patternsTab->setFont(m_font);
    m_patternsTab->setMargin(1);
    l2->add( m_patternsTab );

    KisPatternChooser * chooser = new KisPatternChooser(m_patternChooserPopup, "pattern_chooser");
    chooser->setFont(m_font);
    chooser->setMinimumSize(200, 150);
    m_patternsTab->addTab(chooser, i18n("Patterns"));

    m_patternMediator = new KisResourceMediator( chooser, view);
    connect( m_patternMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(patternActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisFactory::rServerRegistry() -> get("PatternServer");
    m_patternMediator -> connectServer(rServer);

    KisControlFrame::connect(view, SIGNAL(patternChanged(KisPattern *)), this, SLOT(slotPatternChanged( KisPattern *)));
    chooser->setCurrent( 0 );
    m_patternMediator->setActiveItem( chooser->currentItem() );
}


void KisControlFrame::createGradientsChooser(KisView * view)
{
    m_gradientChooserPopup = new KisPopupFrame(m_gradientWidget, "gradient_chooser_popup");

    QHBoxLayout * l2 = new QHBoxLayout(m_gradientChooserPopup, 2, 2, "gradientpopuplayout");

    QTabWidget * m_gradientTab = new QTabWidget(m_gradientChooserPopup, "gradientstab");
    m_gradientTab->setTabShape(QTabWidget::Triangular);
    m_gradientTab->setFocusPolicy(QWidget::NoFocus);
    m_gradientTab->setFont(m_font);
    m_gradientTab->setMargin(1);

    l2->add( m_gradientTab);

    KisGradientChooser * m_gradientChooser = new KisGradientChooser(m_view, m_gradientChooserPopup, "gradient_chooser");
    m_gradientChooser->setFont(m_font);
    m_gradientChooser->setMinimumSize(200, 150);
    m_gradientTab->addTab( m_gradientChooser, i18n("Gradients"));

    m_gradientMediator = new KisResourceMediator( m_gradientChooser, view);
    connect(m_gradientMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(gradientActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisFactory::rServerRegistry()->get("GradientServer");
    m_gradientMediator -> connectServer(rServer);

    connect(view, SIGNAL(gradientChanged(KisGradient *)), this, SLOT(slotGradientChanged( KisGradient *)));
    m_gradientChooser->setCurrent( 0 );
    m_gradientMediator->setActiveItem( m_gradientChooser->currentItem() );
}


#include "kis_controlframe.moc"

