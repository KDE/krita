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

#include <QApplication>
#include <QLayout>
#include <QTabWidget>
#include <QFrame>
#include <QWidget>
#include <QEvent>
#include <QTimer>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>

#include <ktoolbar.h>
#include <kmainwindow.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <KoDualColorButton.h>

#include "kis_resourceserver.h"
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
#include "kis_autobrush.h"
#include "kis_autogradient.h"
#include "kis_config.h"
#include "kis_paintop_box.h"
#include "kis_custom_brush.h"
#include "kis_custom_pattern.h"
#ifdef HAVE_TEXT_BRUSH
#include "kis_text_brush.h"
#endif
KisPopupFrame::KisPopupFrame(QWidget * parent, const char* name)
    : QMenu(parent)
{
    setObjectName(name);
    setFocusPolicy(Qt::StrongFocus);
}

void KisPopupFrame::keyPressEvent(QKeyEvent * e)
{
    if (e->key()== Qt::Key_Escape)
    {
        hide();
        e->accept();
    }
    else
    {
        e->ignore();
    }
}


KisControlFrame::KisControlFrame( KMainWindow * /*window*/, KisView * view, const char* name )
    : QObject(view)
    //: KToolBar ( window, Qt::DockTop, false, name, true, true )
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
    setObjectName(name);
    KisConfig cfg;
    m_font  = KGlobalSettings::generalFont();
    m_font.setPointSize((int)cfg.dockerFontSize());

    m_brushWidget = new KisIconWidget(view, "brushes");
    m_brushWidget->setText( i18n("Brush Shapes") );
    m_brushWidget->setToolTip( i18n("Brush Shapes") );
    // XXX: An action without a slot -- that's silly, what kind of action could we use here?
    KAction *action = new KAction( i18n("&Brush"), view->actionCollection(), "brushes");

    action->setDefaultWidget( m_brushWidget );

    m_patternWidget = new KisIconWidget(view, "patterns");
    m_patternWidget->setText( i18n("Fill Patterns") );
    m_patternWidget->setToolTip( i18n("Fill Patterns") );
    action = new KAction(i18n("&Patterns"), view->actionCollection(), "patterns");
    action->setDefaultWidget( m_patternWidget );

    m_gradientWidget = new KisIconWidget(view, "gradients");
    m_gradientWidget->setText( i18n("Gradients") );
    m_gradientWidget->setToolTip( i18n("Gradients") );
    action = new KAction( i18n("&Gradients"), view->actionCollection(), "gradients");
    action->setDefaultWidget( m_gradientWidget );

    m_paintopBox = new KisPaintopBox( view, view, "paintopbox" );
    action = new KAction(i18n("&Painter's Tools"), view->actionCollection(), "paintops");
    action->setDefaultWidget( m_paintopBox );

/**** Temporary hack to test the KoDualColorButton ***/
   KoDualColorButton * dual = new KoDualColorButton(view->canvasSubject()->fgColor(), view->canvasSubject()->fgColor(), view, view);
    action = new KAction(i18n("&Painter's Tools"), view->actionCollection(), "dual");
    action->setDefaultWidget( dual );
    connect(dual, SIGNAL(foregroundColorChanged(const KoColor &)), view, SLOT(slotSetFGColor(const KoColor &)));
    connect(dual, SIGNAL(backgroundColorChanged(const KoColor &)), view, SLOT(slotSetBGColor(const KoColor &)));
    connect(view, SIGNAL(sigFGColorChanged(const KoColor &)), dual, SLOT(setForegroundColor(const KoColor &)));
    dual->setFixedSize( 26, 26 );
/*******/
    m_brushWidget->setFixedSize( 26, 26 );
    m_patternWidget->setFixedSize( 26, 26 );
    m_gradientWidget->setFixedSize( 26, 26 );

    createBrushesChooser(m_view);
    createPatternsChooser(m_view);
    createGradientsChooser(m_view);

    m_brushWidget->setMenu(m_brushChooserPopup);
    m_brushWidget->setPopupMode(QToolButton::InstantPopup);
    m_patternWidget->setMenu(m_patternChooserPopup);
    m_patternWidget->setPopupMode(QToolButton::InstantPopup);
    m_gradientWidget->setMenu(m_gradientChooserPopup);
    m_gradientWidget->setPopupMode(QToolButton::InstantPopup);
}


void KisControlFrame::slotSetBrush(QTableWidgetItem *item)
{
    if (item)
        m_brushWidget->slotSetItem(*item);
}

void KisControlFrame::slotSetPattern(QTableWidgetItem *item)
{
    if (item)
        m_patternWidget->slotSetItem(*item);
}

void KisControlFrame::slotSetGradient(QTableWidgetItem *item)
{
    if (item)
        m_gradientWidget->slotSetItem(*item);
}

void KisControlFrame::slotBrushChanged(KisBrush * brush)
{
        KisIconItem *item;

        if((item = m_brushMediator->itemFor(brush)))
        {
                slotSetBrush(item);
        } else {
                slotSetBrush( new KisIconItem(brush) );
        }

        m_brushChooserPopup->hide();
}

void KisControlFrame::slotPatternChanged(KisPattern * pattern)
{
        KisIconItem *item;
        if (!pattern)
                return;

        if ( (item = m_patternMediator->itemFor(pattern)) )
                slotSetPattern(item);
        else
                slotSetPattern( new KisIconItem(pattern) );

        m_patternChooserPopup->hide();
}


void KisControlFrame::slotGradientChanged(KisGradient * gradient)
{
        KisIconItem *item;
        if (!gradient)
                return;

        if ( (item = m_gradientMediator->itemFor(gradient)) )
                slotSetGradient(item);
        else
                slotSetGradient( new KisIconItem(gradient) );

        m_gradientChooserPopup->hide();
}

void KisControlFrame::createBrushesChooser(KisView * view)
{

    m_brushChooserPopup = new KisPopupFrame(m_brushWidget, "brush_chooser_popup");

    QHBoxLayout * l = new QHBoxLayout(m_brushChooserPopup);
    l->setObjectName("brushpopuplayout");
    l->setMargin(2);
    l->setSpacing(2);

    m_brushesTab = new QTabWidget(m_brushChooserPopup);
    m_brushesTab->setObjectName("brushestab");
    m_brushesTab->setTabShape(QTabWidget::Triangular);
    m_brushesTab->setFocusPolicy(Qt::NoFocus);
    m_brushesTab->setFont(m_font);
    m_brushesTab->setContentsMargins(1, 1, 1, 1);

    l->addWidget(m_brushesTab);

    KisBrushChooser * m_brushChooser = new KisBrushChooser(0, "brush_chooser");
    m_brushesTab->addTab( m_brushChooser, i18n("Predefined Brushes"));

    KisAutobrush * m_autobrush = new KisAutobrush(0, "autobrush", i18n("Autobrush"));
    m_brushesTab->addTab( m_autobrush, i18n("Autobrush"));
    connect(m_autobrush, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated( KisResource* )));

    KisCustomBrush* customBrushes = new KisCustomBrush(0, "custombrush",
            i18n("Custom Brush"), m_view);
    m_brushesTab->addTab( customBrushes, i18n("Custom Brush"));
    connect(customBrushes, SIGNAL(activatedResource(KisResource*)),
            m_view, SLOT(brushActivated(KisResource*)));
#ifdef HAVE_TEXT_BRUSH
    KisTextBrush* textBrushes = new KisTextBrush(0, "textbrush",
            i18n("Text Brush")/*, m_view*/);
    m_brushesTab->addTab( textBrushes, i18n("Text Brush"));
    connect(textBrushes, SIGNAL(activatedResource(KisResource*)),
            m_view, SLOT(brushActivated(KisResource*)));
#endif

    m_brushChooserPopup->setLayout(l);
    m_brushChooser->setFont(m_font);
    m_brushMediator = new KisResourceMediator( m_brushChooser, this);
    connect(m_brushMediator, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("ImagePipeBrushServer");
    m_brushMediator->connectServer(rServer);
    rServer = KisResourceServerRegistry::instance()->get("BrushServer");
    m_brushMediator->connectServer(rServer);

    KisControlFrame::connect(view, SIGNAL(brushChanged(KisBrush *)), this, SLOT(slotBrushChanged( KisBrush *)));
    m_brushChooser->setCurrent( 0 );
    m_brushMediator->setActiveItem( m_brushChooser->currentItem() );
    customBrushes->setResourceServer(rServer);

    m_autobrush->activate();
}

void KisControlFrame::createPatternsChooser(KisView * view)
{
    m_patternChooserPopup = new KisPopupFrame(m_patternWidget, "pattern_chooser_popup");

    QHBoxLayout * l2 = new QHBoxLayout(m_patternChooserPopup);
    l2->setObjectName("patternpopuplayout");
    l2->setMargin(2);
    l2->setSpacing(2);

    m_patternsTab = new QTabWidget(m_patternChooserPopup);
    m_patternsTab->setObjectName("patternstab");
    m_patternsTab->setTabShape(QTabWidget::Triangular);
    m_patternsTab->setFocusPolicy(Qt::NoFocus);
    m_patternsTab->setFont(m_font);
    m_patternsTab->setContentsMargins(1, 1, 1, 1);
    l2->addWidget( m_patternsTab );

    KisPatternChooser * chooser = new KisPatternChooser(0, "pattern_chooser");
    chooser->setFont(m_font);
    m_patternsTab->addTab(chooser, i18n("Patterns"));

    KisCustomPattern* customPatterns = new KisCustomPattern(0, "custompatterns",
            i18n("Custom Pattern"), m_view);
    customPatterns->setFont(m_font);
    m_patternsTab->addTab( customPatterns, i18n("Custom Pattern"));


    m_patternMediator = new KisResourceMediator( chooser, view);
    connect( m_patternMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(patternActivated(KisResource*)));
    connect(customPatterns, SIGNAL(activatedResource(KisResource*)),
            view, SLOT(patternActivated(KisResource*)));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("PatternServer");
    m_patternMediator->connectServer(rServer);

    KisControlFrame::connect(view, SIGNAL(patternChanged(KisPattern *)), this, SLOT(slotPatternChanged( KisPattern *)));
    chooser->setCurrent( 0 );
    m_patternMediator->setActiveItem( chooser->currentItem() );

    customPatterns->setResourceServer(rServer);
}


void KisControlFrame::createGradientsChooser(KisView * view)
{
    m_gradientChooserPopup = new KisPopupFrame(m_gradientWidget, "gradient_chooser_popup");

    QHBoxLayout * l2 = new QHBoxLayout(m_gradientChooserPopup);
    l2->setObjectName("gradientpopuplayout");
    l2->setMargin(2);
    l2->setSpacing(2);

    m_gradientTab = new QTabWidget(m_gradientChooserPopup);
    m_gradientTab->setObjectName("gradientstab");
    m_gradientTab->setTabShape(QTabWidget::Triangular);
    m_gradientTab->setFocusPolicy(Qt::NoFocus);
    m_gradientTab->setFont(m_font);
    m_gradientTab->setContentsMargins(1, 1, 1, 1);
    l2->addWidget( m_gradientTab);

    m_gradientChooser = new KisGradientChooser(m_view, 0, "gradient_chooser");
    m_gradientChooser->setFont(m_font);
    m_gradientTab->addTab( m_gradientChooser, i18n("Gradients"));

    m_gradientMediator = new KisResourceMediator( m_gradientChooser, view);
    connect(m_gradientMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(gradientActivated(KisResource*)));


    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("GradientServer");
    m_gradientMediator->connectServer(rServer);

    connect(m_gradientChooser, SIGNAL(selected(QTableWidgetItem*)), this, SLOT(slotGradientChanged( )));

    connect(view, SIGNAL(gradientChanged(KisGradient *)), this, SLOT(slotGradientChanged( KisGradient *)));
    m_gradientChooser->setCurrent( 0 );
    m_gradientMediator->setActiveItem( m_gradientChooser->currentItem() );
}


#include "kis_controlframe.moc"

