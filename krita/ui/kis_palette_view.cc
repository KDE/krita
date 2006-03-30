/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *            (c) 2005 Bart Coppens <kde@bartcoppens.be>
 *
 *  Based on already much changed code by Waldo Bastian <bastian@kde.org> from KisPaletteWidget
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

#include <stdio.h>
#include <stdlib.h>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdrawutil.h>
#include <qevent.h>
#include <qfile.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtimer.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klistbox.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kpalette.h>
#include <kimageeffect.h>

#include <kcolordialog.h>
#include <k3colordrag.h>
#include <config.h>
#include <kdebug.h>

#include <kis_meta_registry.h>
#include <kis_color.h>
#include <kis_factory.h>
#include <kis_colorspace_factory_registry.h>
#include "kis_palette_view.h"
#include "kis_resource.h"
#include "kis_palette.h"

KisPaletteView::KisPaletteView(QWidget *parent, const char* name, int minWidth, int cols)
    : Q3ScrollView( parent, name ), mMinWidth(minWidth), mCols(cols)
{
    m_cells = 0;
    m_currentPalette = 0;

    QSize cellSize = QSize( mMinWidth, 50);

    setHScrollBarMode(Q3ScrollView::AlwaysOff);
    setVScrollBarMode(Q3ScrollView::AlwaysOn);

    QSize minSize = QSize(verticalScrollBar()->width(), 0);
    minSize += QSize(frameWidth(), 0);
    minSize += QSize(cellSize);

    setMinimumSize(minSize);
    setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
}

KisPaletteView::~KisPaletteView()
{
}

KisPalette* KisPaletteView::palette() const
{
    return m_currentPalette;
}

void KisPaletteView::setPalette(KisPalette* palette)
{
    m_currentPalette = palette;
    delete m_cells;

    int rows = (m_currentPalette->nColors() + mCols -1 ) / mCols;

    if (rows < 1) rows = 1;

    m_cells = new KColorCells(viewport(), rows, mCols);
    Q_CHECK_PTR(m_cells);

    m_cells->setShading(false);
    m_cells->setAcceptDrags(false);

    QSize cellSize = QSize( mMinWidth, mMinWidth * rows / mCols);
    m_cells->setFixedSize( cellSize );

    for( int i = 0; i < m_currentPalette->nColors(); i++)
    {
        QColor c = m_currentPalette->getColor(i).color;
        m_cells->setColor( i, c );
    }

    connect(m_cells, SIGNAL(colorSelected(int)),
            SLOT(slotColorCellSelected(int)));

    connect(m_cells, SIGNAL(colorDoubleClicked(int)),
            SLOT(slotColorCellDoubleClicked(int)) );

    addChild( m_cells );
    m_cells->show();
    updateScrollBars();
}

void KisPaletteView::slotColorCellSelected( int col )
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    if (!m_currentPalette || (col >= m_currentPalette->nColors()))
        return;

    m_currentEntry = m_currentPalette->getColor(col);
    emit colorSelected(KisColor(m_currentPalette->getColor(col).color, cs));
    emit colorSelected(m_currentPalette->getColor(col).color);
}

void KisPaletteView::slotColorCellDoubleClicked( int col )
{
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getRGB8();
    if (!m_currentPalette || (col >= m_currentPalette->nColors()))
        return;

    emit colorDoubleClicked(KisColor(m_currentPalette->getColor(col).color, cs),
                            m_currentPalette->getColor(col).name);
}

#include "kis_palette_view.moc"

