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

#ifndef __KIS_PALETTE_DOCKER_H__
#define __KIS_PALETTE_DOCKER_H__

#include <QDockWidget>
#include <klocale.h>

#include <KoDockFactoryBase.h>
#include <KoColorSet.h>
#include <KoCanvasObserverBase.h>

class KoColor;
class KisView2;

/**
 * A color palette in table form.
 *
 * This is copied, mostly, from KPaletteTable in KColorDialog, original
 *  @author was Waldo Bastian <bastian@kde.org> -- much has changed, though,
 * to work with KisPalettes and the resource server.
 */
class KisPaletteDocker : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    KisPaletteDocker();
    virtual ~KisPaletteDocker();

    QString palette() const;

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);

private slots:

    void colorSelected(const KoColor& color, bool final);

private:
    void readNamedColor(void);

protected:
    KoColorSet* m_currentPalette;
    KoCanvasBase* m_canvas;
};

class KisPaletteDockerFactory : public KoDockFactoryBase
{
public:
    KisPaletteDockerFactory() {}
    ~KisPaletteDockerFactory() {}

    virtual QString id() const;
    QDockWidget * createDockWidget();

    DockPosition defaultDockPosition() const {
        return DockMinimized;
    }

};

#endif

