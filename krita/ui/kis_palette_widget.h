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

#include <qdict.h>
#include "kis_palette_view.h"

class QComboBox;
class QLineEdit;
class KListBox;
class KisPalette;
class KisResource;
class KisColor;

/**
 * A color palette in table form.
 *
 * This is copied, mostly, from KPaletteTable in KColorDialog, original
 *  @author was Waldo Bastian <bastian@kde.org> -- much has changed, though,
 * to work with KisPalettes and the resource server.
 */
class KisPaletteWidget : public QWidget
{
    Q_OBJECT
public:
    KisPaletteWidget( QWidget *parent, int minWidth=210, int cols = 16);
    virtual ~KisPaletteWidget();

    QString palette() const;
    KisPaletteEntry currentEntry() const { return m_view->currentEntry(); }

public slots:
    void setPalette(const QString &paletteName);

signals:
    void colorSelected(const KisColor &);
    void colorSelected(const QColor&);
    void colorDoubleClicked( const KisColor &, const QString &);

protected slots:
    void slotSetPalette( const QString &_paletteName );

public slots:
    // Called by the resource server whenever a palette is loaded.
    void slotAddPalette(KisResource * palette);

protected:
    void readNamedColor( void );

protected:
    KisPaletteView* m_view;
    QDict<KisPalette> m_namedPaletteMap;
    KisPalette * m_currentPalette;
    QComboBox *combo;
    QScrollView *sv;
    int mMinWidth;
    int mCols;
    bool init;
};

#endif

