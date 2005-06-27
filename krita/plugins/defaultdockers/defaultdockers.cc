/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdockwindow.h>
#include <qwidget.h>

#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <koPaletteManager.h>

#include <kis_config.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_tool.h>
#include <kis_doc.h>
#include <kis_global.h>
#include <kis_id.h>
#include <kis_resourceserver.h>
#include <kis_resource_mediator.h>

#include "kis_gray_widget.h"
#include "kis_hsv_widget.h"
#include "kis_icon_item.h"
#include "kis_palette_widget.h"
#include "kis_rgb_widget.h"
#include "kis_text_brush.h"
#include "kis_autobrush.h"
#include "kis_autogradient.h"
#include "kis_birdeye_box.h"
#include "kis_channelview.h"
#include "kis_color.h"
//#include "kis_controlframe.h"

#include "defaultdockers.h"

typedef KGenericFactory<KritaDefaultDockers> KritaDefaultDockersFactory;
K_EXPORT_COMPONENT_FACTORY( kritadefaultdockers, KritaDefaultDockersFactory( "krita" ) )

KritaDefaultDockers::KritaDefaultDockers(QObject *parent, const char *name, const QStringList &)
		: KParts::Plugin(parent, name)
{
	setInstance(KritaDefaultDockersFactory::instance());


	kdDebug(DBG_AREA_PLUGINS) << "DefaultDockers plugin. Class: "
		  << className()
		  << ", Parent: "
		  << parent -> className()
		  << "\n";

	if ( !parent->inherits("KisView") )
	{
		return;
	}

	m_view = (KisView*) parent;

	m_paletteManager = m_view->paletteManager();
	Q_ASSERT(m_paletteManager);

	m_resourceServer = new KisResourceServer;

 	createBirdEyeBox(m_view);
	
	createChannelView(m_view);

	createHSVWidget(m_view);
	createRGBWidget(m_view);
	createGrayWidget(m_view);
	createPaletteWidget(m_view);

	createBrushesWidget(m_view);
	createAutoBrush(m_view);
	createTextBrush(m_view);

	createAutoGradient(m_view);
	createGradientsWidget(m_view);

	createPatternWidget(m_view);

	m_paletteManager->showWidget("brushes");
	m_paletteManager->showWidget("hsvwidget");
	m_paletteManager->showWidget("layerbox");
	m_paletteManager->showWidget(krita::TOOL_OPTION_WIDGET);

}

KritaDefaultDockers::~KritaDefaultDockers()
{
}

void KritaDefaultDockers::createBirdEyeBox(KisView * view)
{
	m_birdEyeBox = new KisBirdEyeBox(view);
        m_birdEyeBox -> setCaption(i18n("Overview"));
	m_paletteManager->addWidget(actionCollection(), m_birdEyeBox, "birdeyebox", krita::CONTROL_PALETTE);

}

void KritaDefaultDockers::createChannelView(KisView * view)
{
        m_channelView = new KisChannelView((KisDoc*)m_view->getCanvasSubject()->document(), view);
        m_channelView -> setCaption(i18n("Channels"));
	m_paletteManager->addWidget(actionCollection(), m_channelView, "channelview", "layerpalette");
}

void KritaDefaultDockers::createAutoBrush(KisView * view)
{
	m_autobrush = new KisAutobrush(view, "autobrush", i18n("Autobrush"));
	m_paletteManager->addWidget(actionCollection(), m_autobrush, "autobrush", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);
        connect(m_autobrush, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated(KisResource*)));
}

void KritaDefaultDockers::createTextBrush(KisView * view)
{
	m_textBrush = new KisTextBrush(view, "textbrush", i18n("Text Brush"));
	m_paletteManager->addWidget(actionCollection(), m_textBrush, "textbrush", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);
	connect(m_textBrush, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(brushActivated(KisResource*)));

}

void KritaDefaultDockers::createAutoGradient(KisView * view)
{
	m_autogradient = new KisAutogradient(view, "autogradient", i18n("Autogradient"));
	m_paletteManager->addWidget(actionCollection(), m_autogradient, "autogradient", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);
	connect(m_autogradient, SIGNAL(activatedResource(KisResource*)), m_view, SLOT(gradientActivated(KisResource*)));

}

void KritaDefaultDockers::createHSVWidget(KisView * view)
{
	m_hsvwidget = new KisHSVWidget(view, "hsv");
        m_hsvwidget -> setCaption(i18n("HSV"));
	m_paletteManager->addWidget(actionCollection(), m_hsvwidget, "hsvwidget", krita::COLORBOX);
        view->getCanvasSubject()->attach(m_hsvwidget);

}

void KritaDefaultDockers::createRGBWidget(KisView * view)
{
	m_rgbwidget = new KisRGBWidget(view, "rgb");
        m_rgbwidget -> setCaption(i18n("RGB"));
	m_paletteManager->addWidget(actionCollection(), m_rgbwidget, "rgbwidget", krita::COLORBOX);
	view->getCanvasSubject()->attach(m_rgbwidget);
}

void KritaDefaultDockers::createGrayWidget(KisView * view)
{
	m_graywidget = new KisGrayWidget(view, "gray");
        m_graywidget -> setCaption(i18n("Gray"));
	m_paletteManager->addWidget(actionCollection(), m_graywidget, "graywidget", krita::COLORBOX);
        view->getCanvasSubject()->attach(m_graywidget);
}

void KritaDefaultDockers::createPaletteWidget(KisView * view)
{
	m_palettewidget = new KisPaletteWidget(view);
        m_palettewidget -> setCaption(i18n("Palettes"));

        connect(m_resourceServer, SIGNAL(loadedPalette(KisResource*)), m_palettewidget, SLOT(slotAddPalette(KisResource*)));
        m_resourceServer->palettes();
        connect(m_palettewidget, SIGNAL(colorSelected(const KisColor &)), view, SLOT(slotSetFGColor(const KisColor &)));

	m_paletteManager->addWidget(actionCollection(), m_palettewidget, "palettewidget", krita::COLORBOX);
}

void KritaDefaultDockers::createPatternWidget(KisView * view)
{
	m_patternMediator = new KisResourceMediator(MEDIATE_PATTERNS, m_resourceServer, i18n("Patterns"),
                                                    view, "pattern chooser", view);

	connect(m_patternMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(patternActivated(KisResource*)));
	
	m_paletteManager->addWidget(actionCollection(), m_patternMediator->chooserWidget(), "patterns", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);

        view -> patternActivated(dynamic_cast<KisPattern*>(m_patternMediator -> currentResource()));

	//KritaDefaultDockers::connect(view, SIGNAL(patternChanged(KisPattern *)), this, SLOT(slotPatternChanged( KisPattern *)));

}

void KritaDefaultDockers::createBrushesWidget(KisView * view)
{

	m_brushMediator = new KisResourceMediator(MEDIATE_BRUSHES, m_resourceServer, i18n("Brushes"),
                                                  view, "brush_chooser", view);

	m_paletteManager->addWidget(actionCollection(), m_brushMediator->chooserWidget(), "brushes", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);

        view -> brushActivated(dynamic_cast<KisBrush*>(m_brushMediator -> currentResource()));

	//KritaDefaultDockers::connect(view, SIGNAL(brushChanged(KisBrush *)), this, SLOT(slotBrushChanged( KisBrush *)));
        KritaDefaultDockers::connect(m_brushMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(brushActivated(KisResource*)));
}

void KritaDefaultDockers::createGradientsWidget(KisView * view)
{
	m_gradientMediator = new KisResourceMediator(MEDIATE_GRADIENTS, m_resourceServer, i18n("Gradients"),
                                                             view, "gradient chooser", view);

        m_paletteManager->addWidget(actionCollection(), m_gradientMediator->chooserWidget(), "gradients", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);

        view -> gradientActivated(dynamic_cast<KisGradient*>(m_gradientMediator -> currentResource()));
	//KritaDefaultDockers::connect(view, SIGNAL(gradientChanged(KisGradient *)), this, SLOT(slotGradientChanged( KisGradient *)));
        KritaDefaultDockers::connect(m_gradientMediator, SIGNAL(activatedResource(KisResource*)), view, SLOT(gradientActivated(KisResource*)));

}
#if 0
void KritaDefaultDockers::slotBrushChanged(KisBrush * brush)
{
        KisIconItem *item;

        if((item = m_brushMediator -> itemFor(brush)))
        {
                if (m_controlWidget) m_controlWidget -> slotSetBrush(item);
        } else {
                if (m_controlWidget) m_controlWidget -> slotSetBrush( new KisIconItem(brush) );
        }

}


void KritaDefaultDockers::slotGradientChanged(KisGradient * gradient)
{
        KisIconItem *item;
        if (!gradient || ! m_controlWidget)
                return;

        if ( (item = m_gradientMediator -> itemFor(gradient)) )
                m_controlWidget -> slotSetGradient(item);
        else
                m_controlWidget -> slotSetGradient( new KisIconItem(gradient) );
}

void KritaDefaultDockers::slotPatternChanged(KisPattern * pattern)
{
        KisIconItem *item;
        if (!pattern || ! m_controlWidget)
                return;

        if ( (item = m_gradientMediator -> itemFor(pattern)) )
                m_controlWidget -> slotSetGradient(item);
        else
                m_controlWidget -> slotSetGradient( new KisIconItem(pattern) );
}

#endif

#include "defaultdockers.moc"
