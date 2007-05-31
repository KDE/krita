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

#include <QComboBox>
#include <QVBoxLayout>

#include <kdebug.h>

#include "kis_view2.h"
#include "kis_palette_docker.h"
#include "KoResource.h"
#include "KoColorSet.h"
#include "kis_palette_view.h"
#include "kis_resourceserver.h"
#include "kis_resource_provider.h"

KisPaletteDocker::KisPaletteDocker( KisView2 * view )
    : QDockWidget(i18n("Palettes"))
{
    m_view = view;

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_currentPalette = 0;

    QVBoxLayout *layout = new QVBoxLayout( mainWidget );

    //m_paletteView = new KisPaletteView(this, 0, mMinWidth, mCols);
    //layout->addWidget( m_paletteView );

    //setFixedSize(sizeHint());
/*
    connect(m_paletteView, SIGNAL(colorSelected(const KoColor &)),
            this, SLOT(colorSelected(const KoColor &)));
*/
    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("PaletteServer");
    QList<KoResource*> resources = rServer->resources();
}

KisPaletteDocker::~KisPaletteDocker()
{
}

void KisPaletteDocker::colorSelected( const KoColor& color )
{
    m_view->resourceProvider()->setFGColor(color);
}

QString KisPaletteDockerFactory::id() const
{
    return QString("KisPaletteDocker");
}

Qt::DockWidgetArea KisPaletteDockerFactory::defaultDockWidgetArea() const
{
    return Qt::RightDockWidgetArea;
}

QDockWidget* KisPaletteDockerFactory::createDockWidget()
{
    KisPaletteDocker* dockWidget = new KisPaletteDocker(m_view);
    dockWidget->setObjectName(id());

    return dockWidget;
}


#include "kis_palette_docker.moc"

