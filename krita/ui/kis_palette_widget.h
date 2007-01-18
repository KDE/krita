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

#ifndef __KIS_PALETTE_WIDGET_H__
#define __KIS_PALETTE_WIDGET_H__

#include <q3dict.h>
#include <QDockWidget>
#include <klocale.h>

#include <KoDockFactory.h>
#include "kis_palette_view.h"

class QComboBox;
class QLineEdit;
class KListBox;
class KisPalette;
class KisResource;
class KoColor;
class KisView2;

/**
 * A color palette in table form.
 *
 * This is copied, mostly, from KPaletteTable in KColorDialog, original
 *  @author was Waldo Bastian <bastian@kde.org> -- much has changed, though,
 * to work with KisPalettes and the resource server.
 */
class KisPaletteWidget : public QDockWidget
{
    Q_OBJECT
public:
    KisPaletteWidget( KisView2 * view );
    virtual ~KisPaletteWidget();

    QString palette() const;
    KisPaletteEntry currentEntry() const { return m_paletteView->currentEntry(); }

public slots:
    void setPalette(const QString &paletteName);

protected slots:
    void slotSetPalette( const QString &_paletteName );
    void colorSelected( const KoColor& color );

public slots:
    // Called by the resource server whenever a palette is loaded.
    void slotAddPalette(KisResource * palette);

protected:
    void readNamedColor( void );

protected:
    KisPaletteView* m_paletteView;
    Q3Dict<KisPalette> m_namedPaletteMap;
    KisPalette * m_currentPalette;
    QComboBox *combo;
    Q3ScrollView *sv;
    int mMinWidth;
    int mCols;
    bool init;
    KisView2 * m_view;
};

class KisPaletteDockerFactory : public KoDockFactory
{
public:
    KisPaletteDockerFactory(KisView2 * view) { m_view = view; }
    ~KisPaletteDockerFactory() {}

    QString dockId() const;
    Qt::DockWidgetArea defaultDockWidgetArea() const;
    QDockWidget * createDockWidget();

private:
    KisView2 * m_view;
};

#endif

