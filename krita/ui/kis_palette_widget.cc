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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#include <kcolordrag.h>
#include <config.h>
#include <kdebug.h>

#include "kis_palette_widget.h"
#include "kis_resource.h"
#include "kis_palette.h"

KisPaletteWidget::KisPaletteWidget( QWidget *parent, int minWidth, int cols)
	: QWidget( parent ), mMinWidth(minWidth), mCols(cols)
{
	init = false;

	cells = 0;
	m_currentPalette = 0;

	QVBoxLayout *layout = new QVBoxLayout( this );

	combo = new QComboBox( false, this );
        combo->setFocusPolicy( QWidget::ClickFocus );
	layout->addWidget(combo);

	sv = new QScrollView( this );
	QSize cellSize = QSize( mMinWidth, 120);
	sv->setHScrollBarMode( QScrollView::AlwaysOff);
	sv->setVScrollBarMode( QScrollView::AlwaysOn);
	QSize minSize = QSize(sv->verticalScrollBar()->width(), 0);
	minSize += QSize(sv->frameWidth(), 0);
	minSize += QSize(cellSize);
	sv->setFixedSize(minSize);
	layout->addWidget(sv);

	setFixedSize(sizeHint());

	connect( combo, SIGNAL(activated(const QString &)),
		 this, SLOT(slotSetPalette( const QString &)));
}

KisPaletteWidget::~KisPaletteWidget()
{
}

QString KisPaletteWidget::palette() const
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
void KisPaletteWidget::slotSetPalette( const QString &_paletteName )
{
	setPalette( _paletteName );
	slotColorCellSelected(0); // FIXME: We need to save the current value!!
}


void KisPaletteWidget::setPalette( const QString &_paletteName )
{
	QString paletteName( _paletteName);

	m_currentPalette = m_namedPaletteMap[paletteName];

	if (combo->currentText() != paletteName)
	{
		bool found = false;
		for(int i = 0; i < combo->count(); i++)
		{
			if (combo->text(i) == paletteName)
			{
				combo->setCurrentItem(i);
				found = true;
				break;
			}
		}
		if (!found)
		{
			combo->insertItem(paletteName);
			combo->setCurrentItem(combo->count()-1);
		}
	}

	delete cells;

	int rows = (m_currentPalette -> nColors() + mCols -1 ) / mCols;

	if (rows < 1) rows = 1;

	cells = new KColorCells( sv->viewport(), rows, mCols);
	Q_CHECK_PTR(cells);

	cells->setShading(false);
	cells->setAcceptDrags(false);

	QSize cellSize = QSize( mMinWidth, mMinWidth * rows / mCols);
	cells->setFixedSize( cellSize );

	for( int i = 0; i < m_currentPalette -> nColors(); i++)
	{
		QColor c = m_currentPalette -> getColor(i).color;
		cells->setColor( i, c );
	}

	connect( cells, SIGNAL( colorSelected( int ) ),
		 SLOT( slotColorCellSelected( int ) ) );

	connect( cells, SIGNAL( colorDoubleClicked( int ) ),
		 SLOT( slotColorCellDoubleClicked( int ) ) );

	sv->addChild( cells );
	cells->show();
	sv->updateScrollBars();

}

void KisPaletteWidget::slotColorCellSelected( int col )
{
	if (!m_currentPalette || (col >= m_currentPalette->nColors()))
		return;
	emit colorSelected( m_currentPalette->getColor(col).color );

}

void KisPaletteWidget::slotColorCellDoubleClicked( int col )
{
	if (!m_currentPalette || (col >= m_currentPalette -> nColors()))
		return;
	emit colorDoubleClicked( m_currentPalette->getColor(col).color, m_currentPalette->getColor(col).name);
}


void KisPaletteWidget::slotAddPalette(KisResource * palette)
{
	KisPalette * p = dynamic_cast<KisPalette*>(palette);

	m_namedPaletteMap.insert(palette -> name(), p);

	combo -> insertItem(palette -> name());

	if (!init) {
		combo -> setCurrentItem(0);
		setPalette(combo ->currentText());
		init = true;
	}
}


#include "kis_palette_widget.moc"

