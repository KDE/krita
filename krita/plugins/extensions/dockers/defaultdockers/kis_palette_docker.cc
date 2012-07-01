/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_palette_docker.h"

#include <QComboBox>
#include <QVBoxLayout>
#include <QTimer>

#include <KoCanvasBase.h>
#include <KoResource.h>
#include <KoColorSet.h>
#include <KoColorSetWidget.h>
#include <KoCanvasResourceManager.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceServerProvider.h>
#include <kis_config.h>
#include <kis_workspace_resource.h>
#include <kis_canvas_resource_provider.h>
#include <kis_view2.h>
#include <kis_canvas2.h>

KisPaletteDocker::KisPaletteDocker()
        : QDockWidget(i18n("Palettes"))
        , m_currentPalette(0)
        , m_canvas(0)
{

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);

    m_chooser = new KoColorSetWidget(this);
    layout->addWidget(m_chooser);
    mainWidget->setLayout(layout);

    connect(m_chooser, SIGNAL(colorChanged(const KoColor&, bool)), SLOT(colorSelected(const KoColor&, bool)));

    KisConfig cfg;
    m_defaultPalette = cfg.defaultPalette();

    KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
    m_serverAdapter = new KoResourceServerAdapter<KoColorSet>(rServer, this);
    connect(m_serverAdapter, SIGNAL(resourceAdded(KoResource*)), this, SLOT(resourceAddedToServer(KoResource*)));
    m_serverAdapter->connectToResourceServer();
    checkForDefaultResource();
}

KisPaletteDocker::~KisPaletteDocker()
{
    KoColorSet* colorSet = m_chooser->colorSet();
    if (colorSet) {
        KisConfig cfg;
        cfg.setDefaultPalette(colorSet->name());
    }
}

void KisPaletteDocker::setCanvas(KoCanvasBase * canvas)
{
    m_canvas = canvas;

    KisCanvas2* kisCanvas = dynamic_cast<KisCanvas2*>(canvas);
    Q_ASSERT(canvas);
    KisView2* view = kisCanvas->view();
    connect(view->resourceProvider(), SIGNAL(sigSavingWorkspace(KisWorkspaceResource*)), SLOT(saveToWorkspace(KisWorkspaceResource*)));
    connect(view->resourceProvider(), SIGNAL(sigLoadingWorkspace(KisWorkspaceResource*)), SLOT(loadFromWorkspace(KisWorkspaceResource*)));
}

void KisPaletteDocker::colorSelected(const KoColor& c, bool final)
{
    if (final && m_canvas) {
        m_canvas->resourceManager()->setForegroundColor(c);
    }
}

QString KisPaletteDockerFactory::id() const
{
    return QString("KisPaletteDocker");
}

QDockWidget* KisPaletteDockerFactory::createDockWidget()
{
    KisPaletteDocker* dockWidget = new KisPaletteDocker();
    dockWidget->setObjectName(id());

    return dockWidget;
}

void KisPaletteDocker::resourceAddedToServer(KoResource* resource)
{
    // Avoiding resource mutex deadlock
    QTimer::singleShot( 0, this, SLOT( checkForDefaultResource() ) );
}

void KisPaletteDocker::checkForDefaultResource()
{
    foreach(KoResource* resource, m_serverAdapter->resources()) {
        if (resource->name() == m_defaultPalette) {
            KoColorSet* colorSet = static_cast<KoColorSet*>(resource);
            m_chooser->setColorSet(colorSet);
        }
    }
}

void KisPaletteDocker::saveToWorkspace(KisWorkspaceResource* workspace)
{
    KoColorSet* colorSet = m_chooser->colorSet();
    if (colorSet) {
        workspace->setProperty("palette", colorSet->name());
    }
}

void KisPaletteDocker::loadFromWorkspace(KisWorkspaceResource* workspace)
{
    if (workspace->hasProperty("palette")) {
        KoResourceServer<KoColorSet>* rServer = KoResourceServerProvider::instance()->paletteServer();
        KoColorSet* colorSet = rServer->getResourceByName(workspace->getString("palette"));
        if (colorSet) {
            m_chooser->setColorSet(colorSet);
        }
    }
}


#include "kis_palette_docker.moc"

