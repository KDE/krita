/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *            (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef __KIS_PALETTE_VIEW_H__
#define __KIS_PALETTE_VIEW_H__

#include <q3scrollview.h>
#include "kis_palette.h"

class KListBox;
class KisPalette;
class KColorCells;
class KisResource;
class KoColor;

/**
 * A scrolling view that lists a single KisPalette
 */
class KisPaletteView : public Q3ScrollView
{
    Q_OBJECT
public:
    KisPaletteView(QWidget *parent, const char* name = 0, int minWidth=210, int cols = 16);
    virtual ~KisPaletteView();

    KisPalette* palette() const;
    /// Might return the default constructed entry...
    KisPaletteEntry currentEntry() const { return m_currentEntry; }

public slots:
    void setPalette(KisPalette* p);

signals:
    void colorSelected(const KoColor &);
    void colorSelected(const QColor &);
    void colorDoubleClicked(const KoColor &, const QString &);

protected slots:
    void slotColorCellSelected( int );
    void slotColorCellDoubleClicked( int );

protected:
    KisPalette* m_currentPalette;
    KColorCells* m_cells;
    KisPaletteEntry m_currentEntry;
    int mMinWidth;
    int mCols;

    friend class KisPaletteWidget; // Because it calls slotColorCellSelected from a FIXME
};

#endif

