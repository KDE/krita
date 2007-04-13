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
#include "kis_resource.h"
#include "kis_palette.h"
#include "kis_palette_view.h"
#include "kis_resourceserver.h"
#include "kis_resource_provider.h"

KisPaletteDocker::KisPaletteDocker( KisView2 * view )
    : QDockWidget( i18n("Palettes") ), mMinWidth(210), mCols(16)
{
    m_view = view;
    init = false;

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_currentPalette = 0;

    QVBoxLayout *layout = new QVBoxLayout( mainWidget );

    combo = new QComboBox( this );
    combo->setFocusPolicy( Qt::ClickFocus );
    layout->addWidget(combo);

    m_paletteView = new KisPaletteView(this, 0, mMinWidth, mCols);
    layout->addWidget( m_paletteView );

    //setFixedSize(sizeHint());

    connect(combo, SIGNAL(activated(const QString &)),
            this, SLOT(slotSetPalette(const QString &)));
    connect(m_paletteView, SIGNAL(colorSelected(const KoColor &)),
            this, SLOT(colorSelected(const KoColor &)));

    KisResourceServerBase* rServer;
    rServer = KisResourceServerRegistry::instance()->get("PaletteServer");
    QList<KisResource*> resources = rServer->resources();

    foreach (KisResource *resource, resources) {
        slotAddPalette(resource);
    }
}

KisPaletteDocker::~KisPaletteDocker()
{
}

QString KisPaletteDocker::palette() const
{
    return combo->currentText();
}


// 2000-02-12 Espen Sand
// Set the color in two steps. The setPalette() slot will not emit a signal
// with the current color setting. The reason is that setPalette() is used
// by the color selector dialog on startup. In the color selector dialog
// we normally want to display a startup color which we specify
// when the dialog is started. The slotSetPalette() slot below will
// set the palette and then use the information to emit a signal with the
// new color setting. It is only used by the combobox widget.
//
void KisPaletteDocker::slotSetPalette( const QString &_paletteName )
{
    setPalette( _paletteName );
    m_paletteView->slotColorCellSelected(0); // FIXME: We need to save the current value!!
}


void KisPaletteDocker::setPalette( const QString &_paletteName )
{
    QString paletteName( _paletteName);

    m_currentPalette = m_namedPaletteMap[paletteName];

    if (combo->currentText() != paletteName)
    {
        int i = combo->findText(paletteName);

        if (i >= 0) {
            combo->setCurrentIndex(i);
        } else {
            combo->addItem(paletteName);
            combo->setCurrentIndex(combo->count() - 1);
        }
    }

    m_paletteView->setPalette(m_currentPalette);
}

void KisPaletteDocker::slotAddPalette(KisResource * palette)
{
    KisPalette * p = dynamic_cast<KisPalette*>(palette);

    if (p) {
        m_namedPaletteMap.insert(palette->name(), p);

        combo->addItem(palette->name());

        if (!init) {
            combo->setCurrentIndex(0);
            setPalette(combo ->currentText());
            init = true;
        }
    }
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

