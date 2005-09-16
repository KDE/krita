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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdockwindow.h>
#include <qwidget.h>
#include <qcombobox.h>

#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <ktoolbar.h>
#include <kopalettemanager.h>

#include <kis_toolbox.h>
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
#include "kis_birdeye_box.h"
#include "kis_color.h"
#include "kis_controlframe.h"
#include "kis_factory.h"

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

     createBirdEyeBox(m_view);

    createHSVWidget(m_view);
    createRGBWidget(m_view);
    createGrayWidget(m_view);
    createPaletteWidget(m_view);
    createControlFrame(m_view);

    m_paletteManager->showWidget("hsvwidget");
    m_paletteManager->showWidget("layerbox");
    m_paletteManager->showWidget(krita::TOOL_OPTION_WIDGET);

}

KritaDefaultDockers::~KritaDefaultDockers()
{
}

void KritaDefaultDockers::createControlFrame(KisView * view)
{
    m_controlWidget = new KisControlFrame(view, view, "control frame");
    m_controlWidget->setCaption(i18n("Brushes and stuff"));
    //m_paletteManager->addWidget( m_controlWidget, i18n("Brushes and stuff"), krita::PAINTBOX );
}


void KritaDefaultDockers::createBirdEyeBox(KisView * view)
{
    m_birdEyeBox = new KisBirdEyeBox(view);
    m_birdEyeBox -> setCaption(i18n("Overview"));
    m_paletteManager->addWidget( m_birdEyeBox, "birdeyebox", krita::CONTROL_PALETTE);
    connect(m_birdEyeBox, SIGNAL(exposureChanged(float)), view, SLOT(setHDRExposure(float)));
}


void KritaDefaultDockers::createHSVWidget(KisView * view)
{
    m_hsvwidget = new KisHSVWidget(view, "hsv");
    m_hsvwidget -> setCaption(i18n("HSV"));
    m_paletteManager->addWidget( m_hsvwidget, "hsvwidget", krita::COLORBOX);
    view->getCanvasSubject()->attach(m_hsvwidget);

}

void KritaDefaultDockers::createRGBWidget(KisView * view)
{
    m_rgbwidget = new KisRGBWidget(view, "rgb");
    m_rgbwidget -> setCaption(i18n("RGB"));
    m_paletteManager->addWidget( m_rgbwidget, "rgbwidget", krita::COLORBOX);
    view->getCanvasSubject()->attach(m_rgbwidget);
}

void KritaDefaultDockers::createGrayWidget(KisView * view)
{
    m_graywidget = new KisGrayWidget(view, "gray");
    m_graywidget -> setCaption(i18n("Gray"));
    m_paletteManager->addWidget( m_graywidget, "graywidget", krita::COLORBOX);
        view->getCanvasSubject()->attach(m_graywidget);
}

void KritaDefaultDockers::createPaletteWidget(KisView * view)
{
    m_palettewidget = new KisPaletteWidget(view);
    m_palettewidget -> setCaption(i18n("Palettes"));

    KisResourceServerBase* rServer;
    rServer = KisFactory::rServerRegistry() -> get("PaletteServer");
    QValueList<KisResource*> resources = rServer->resources();
    QValueList<KisResource*>::iterator it;
    for ( it = resources.begin(); it != resources.end(); ++it )
        m_palettewidget -> slotAddPalette( *it );


    connect(m_palettewidget, SIGNAL(colorSelected(const KisColor &)),
        view, SLOT(slotSetFGColor(const KisColor &)));

    m_paletteManager->addWidget( m_palettewidget, "palettewidget", krita::COLORBOX);
}

#include "defaultdockers.moc"
