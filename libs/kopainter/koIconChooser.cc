/* This file is part of the KDE project
  Copyright (c) 1999 Carsten Pfeiffer (pfeiffer@kde.org)
  Copyright (c) 2002 Igor Jansen (rm@kde.org)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include "koIconChooser.h"
#include <kglobal.h>

#include <QPainter>
#include <QCursor>
#include <QApplication>
#include <QLayout>
#include <QPixmap>
#include <Q3PtrList>
#include <Q3VBoxLayout>
#include <QFrame>
#include <QResizeEvent>
#include <QTableWidget>
#include <QHeaderView>
#include <kdebug.h>

KoIconChooser::KoIconChooser(QSize aIconSize, QWidget *parent):
QTableWidget(parent)
{
    m_itemCount = 0;
    mItemWidth = aIconSize.width();
    mItemHeight = aIconSize.height();
    setRowCount(1);
    setColumnCount(1);
    horizontalHeader()->hide();
    verticalHeader()->hide();
    setSelectionMode(QAbstractItemView::SingleSelection);
}

KoIconChooser::~KoIconChooser()
{
}

void KoIconChooser::keyPressEvent(QKeyEvent * e)
{
    if (e->key()== Qt::Key_Return)
    {
        e->accept();
        emit itemClicked(currentItem());
    }
    else
        QTableWidget::keyPressEvent(e);
}

// recalculate the number of items that fit into one row
// set the current item again after calculating the new grid
void KoIconChooser::resizeEvent(QResizeEvent *e)
{
    QTableWidget::resizeEvent(e);

    int oldNColumns = columnCount();
    int nColumns = width( ) / mItemWidth;
    if(nColumns == 0)
        nColumns = 1;

    int oldNRows = rowCount();
    int nRows = (m_itemCount - 1)/ nColumns +1;

    if(nColumns > oldNColumns)
    {
        // We are now wider than before so we must reorder from the top

        setColumnCount(nColumns); // make sure there is enough space for our reordering
        int newRow = 0, newColumn = 0, oldRow = 0, oldColumn = 0;
        for(int i = 0; i < m_itemCount; i++)
        {
            QTableWidgetItem *theItem;
            theItem = takeItem(oldRow, oldColumn);
            setItem(newRow, newColumn, theItem);
            newColumn++;
            if(newColumn == nColumns)
            {
                newColumn = 0;
                newRow++;
            }
            oldColumn++;
            if(oldColumn == oldNColumns)
            {
                oldColumn = 0;
                oldRow++;
            }
        }
    }
    else if(nColumns < oldNColumns)
    {
        // We are now not as wide as before so we must reorder from the top

        setRowCount(nRows);// make sure there is enough space for our reordering
        int newRow = nRows - 1, newColumn = m_itemCount - newRow * nColumns,
                      oldRow = oldNRows - 1, oldColumn = m_itemCount - oldRow * oldNColumns;
        for(int i = 0; i < m_itemCount; i++)
        {
            QTableWidgetItem *theItem;
            theItem = takeItem(oldRow, oldColumn);
            setItem(newRow, newColumn, theItem);
            newColumn--;
            if(newColumn < 0)
            {
                newColumn = nColumns - 1;
                newRow--;
            }
            oldColumn--;
            if(oldColumn < 0)
            {
                oldColumn = oldNColumns - 1;
                oldRow--;
            }
        }
    }

    // Set to the number of rows and columns
    setColumnCount(nColumns);
    setRowCount(nRows);

    // resize cells in case it's needed
    for(int i = 0; i < nColumns; i++)
        setColumnWidth(i, mItemWidth);
    for(int i = 0; i < nRows; i++)
        setRowHeight(i, mItemHeight);
}

QTableWidgetItem *KoIconChooser::itemAt(int index)
{
    int row = index / columnCount();
    int col = index - row * columnCount();
    return item(row,col);
}

// adds an item to the end
void KoIconChooser::addItem(QTableWidgetItem *item)
{
    int row = m_itemCount / columnCount();
    int col = m_itemCount - row* columnCount();
    if(row+1 > rowCount())
        setRowCount(row+1);
    setItem(row, col, item);
    m_itemCount++;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
}

KoPatternChooser::KoPatternChooser( const Q3PtrList<QTableWidgetItem> &list, QWidget *parent, const char *name )
 : QWidget( parent, name )
{
    // only serves as beautifier for the iconchooser
    //frame = new QHBox( this );
    //frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    chooser = new KoIconChooser( QSize(30,30), this);

	QObject::connect( chooser, SIGNAL(selected( QTableWidgetItem * ) ),
					            this, SIGNAL( selected( QTableWidgetItem * )));

	Q3PtrListIterator<QTableWidgetItem> itr( list );
	for( itr.toFirst(); itr.current(); ++itr )
		chooser->addItem( itr.current() );

	Q3VBoxLayout *mainLayout = new Q3VBoxLayout( this, 1, -1, "main layout" );
	mainLayout->addWidget( chooser, 10 );
}


KoPatternChooser::~KoPatternChooser()
{
  delete chooser;
  //delete frame;
}

// set the active pattern in the chooser - does NOT emit selected() (should it?)
void KoPatternChooser::setCurrentPattern( QTableWidgetItem *pattern )
{
    chooser->setCurrentItem( pattern );
}

void KoPatternChooser::addPattern( QTableWidgetItem *pattern )
{
    chooser->addItem( pattern );
}

// return the active pattern
QTableWidgetItem *KoPatternChooser::currentPattern()
{
    return chooser->currentItem();
}

#include "koIconChooser.moc"
