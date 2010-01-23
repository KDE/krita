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

#include <KoCanvasBase.h>
#include <KoResource.h>
#include <KoColorSet.h>
#include <KoColorSetWidget.h>
#include <KoResourceManager.h>
#include <KoColorSpaceRegistry.h>

KisPaletteDocker::KisPaletteDocker()
        : QDockWidget(i18n("Palettes"))
        , m_currentPalette(0)
        , m_canvas(0)
{

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    QVBoxLayout *layout = new QVBoxLayout(mainWidget);

    KoColorSetWidget* chooser = new KoColorSetWidget(this);
    layout->addWidget(chooser);
    mainWidget->setLayout(layout);

    connect(chooser, SIGNAL(colorChanged(const KoColor&, bool)), SLOT(colorSelected(const KoColor&, bool)));
}

KisPaletteDocker::~KisPaletteDocker()
{
}

void KisPaletteDocker::setCanvas(KoCanvasBase * canvas)
{
    m_canvas = canvas;
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


#include "kis_palette_docker.moc"

