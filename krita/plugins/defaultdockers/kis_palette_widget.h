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

#include <qframe.h>
#include <qpixmap.h>
#include <qgridview.h>
#include <qdict.h>

#include "kselect.h"
#include "kcolordialog.h"

class QComboBox;
class QLineEdit;
class KListBox;
class KisPalette;
class KColorCells;
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

public slots:
    void setPalette(const QString &paletteName);

signals:
    void colorSelected( const KisColor &);
    void colorDoubleClicked( const KisColor &, const QString &);

protected slots:
    void slotColorCellSelected( int );
    void slotColorCellDoubleClicked( int );
    void slotSetPalette( const QString &_paletteName );

public slots:
    // Called by the resource server whenever a palette is loaded.
    void slotAddPalette(KisResource * palette);

protected:
    void readNamedColor( void );

protected:

    QDict<KisPalette> m_namedPaletteMap;
    KisPalette * m_currentPalette;
    QComboBox *combo;
    KColorCells *cells;
    QScrollView *sv;
    int mMinWidth;
    int mCols;
    bool init;
};

#endif        // __KCOLORDIALOG_H__

