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

#include <qpainter.h>
#include <qcursor.h>
#include <qapplication.h>
#include <q3hbox.h>
#include <QLayout>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <Q3PtrList>
#include <QKeyEvent>
#include <Q3Frame>
#include <QResizeEvent>
#include <Q3VBoxLayout>
#include <QMouseEvent>
#include <kdebug.h>

KoPixmapWidget::KoPixmapWidget(const QPixmap &aPixmap, QWidget *parent, const char *name):
Q3Frame(parent, name, Qt::WType_Popup)
{
  kDebug() << "Popup created: " << name << "\n";
  setFrameStyle(Q3Frame::WinPanel | Q3Frame::Raised);
  mPixmap = aPixmap;
  int w = mPixmap.width() + 2 * lineWidth();
  int h = mPixmap.height() + 2 * lineWidth();
  resize(w, h);

}

KoPixmapWidget::~KoPixmapWidget()
{
}

// paint the centered pixmap; don't overpaint the frame
void KoPixmapWidget::paintEvent(QPaintEvent *e)
{
  Q3Frame::paintEvent(e);
  QPainter p(this);
  p.setClipRect(e->rect());
  p.drawPixmap(lineWidth(), lineWidth(), mPixmap);
}


void KoPixmapWidget::mouseReleaseEvent(QMouseEvent *)
{
	hide();
}


KoIconChooser::KoIconChooser(QSize aIconSize, QWidget *parent, const char *name, bool sort):
Q3GridView(parent, name)
{
  Q3GridView::setBackgroundColor(Qt::white);

  mMargin = 2;
  setCellWidth(aIconSize.width() + 2 * mMargin);
  setCellHeight(aIconSize.height() + 2 * mMargin);

  mIconList.clear();
  mPixmapWidget = 0L;
  mNCols = 0;
  mCurRow = 0;
  mCurCol = 0;
  mItemCount = 0;
  mItemWidth = aIconSize.width();
  mItemHeight = aIconSize.height();
  mMouseButtonDown = false;
  mDragEnabled = false;
  mSort = sort;
}

KoIconChooser::~KoIconChooser()
{
  mIconList.clear();
  if(mPixmapWidget)
    delete mPixmapWidget;
}

void KoIconChooser::addItem(KoIconItem *item)
{
  qint32 n = mItemCount;
  KoIconItem *current = currentItem();

  Q_ASSERT(item);

  int i;

  if (mSort)
  {
    i = sortInsertionIndex(item);
  }
  else
  {
    i = mItemCount;
  }

  mIconList.insert(i, item);
  mItemCount++;
  calculateCells();

  if (mSort)
  {
    setCurrentItem(current);
    updateContents();
  }
  else
  {
    updateCell(n / numCols(), n - (n / numCols()) * numCols());
  }
}

bool KoIconChooser::removeItem(KoIconItem *item)
{
  int index = mIconList.find(item);
  bool ok = mIconList.remove(item);
  if( ok )
  {
    mItemCount--;
    // select same cell as deleted cell, or new last cell if we deleted
    // last cell.
    setCurrentItem(itemAt(qMin(index, mItemCount - 1)));
    calculateCells();
  }
  return ok;
}

void KoIconChooser::clear()
{
  mItemCount = 0;
  mIconList.clear();
  calculateCells();
}

// return 0L if there is no current item
KoIconItem *KoIconChooser::currentItem()
{
  return itemAt(mCurRow, mCurCol);
}

// sets the current item to item
// does NOT emit selected()  (should it?)
void KoIconChooser::setCurrentItem(KoIconItem *item)
{
  int index = mIconList.find(item);

  // item is available
  if(index != -1 && mNCols > 0)
  {
    int oldRow = mCurRow;
    int oldCol = mCurCol;

    mCurRow = index / mNCols;
    mCurCol = index % mNCols;

    // repaint the old and the new item
    repaintCell(oldRow, oldCol);
    repaintCell(mCurRow, mCurCol);

    ensureCellVisible(mCurRow, mCurCol);
  }
}

// eventually select the item, clicked on
void KoIconChooser::mousePressEvent(QMouseEvent *e)
{
  Q3GridView::mousePressEvent(e);
}

void KoIconChooser::mouseMoveEvent(QMouseEvent *e)
{
  if(mMouseButtonDown && mDragEnabled )
  {
#if 0
    if(mPixmapWidget)
    {
      delete mPixmapWidget;
      mPixmapWidget = 0L;
    }
#endif
    if( ( mDragStartPos - e->pos() ).manhattanLength() > QApplication::startDragDistance() )
      startDrag();
  }
}

void
KoIconChooser::startDrag()
{
  mMouseButtonDown = false;
}

void KoIconChooser::mouseReleaseEvent(QMouseEvent * e)
{
  mMouseButtonDown = true;
  if(e->button() == Qt::LeftButton)
  {
    QPoint p = e->pos();
    mDragStartPos = p;
    int x = contentsX() + p.x();
    int y = contentsY() + p.y();
    QSize gridExtent = gridSize();

    if (x < gridExtent.width() && y < gridExtent.height())
    {
      int row = rowAt(y);
      int col = columnAt(x);

      KoIconItem *item = itemAt(row, col);
      if(item)
      {
        const QPixmap &pix = item->pixmap();
        if(pix.width() > mItemWidth || pix.height() > mItemHeight)
          showFullPixmap(pix, p);

        setCurrentItem(item);
        emit selected( item );
      }
    }
  }
}

// FIXME: implement keyboard navigation
void KoIconChooser::keyPressEvent(QKeyEvent *e)
{
  Q3GridView::keyPressEvent(e);
}

// recalculate the number of items that fit into one row
// set the current item again after calculating the new grid
void KoIconChooser::resizeEvent(QResizeEvent *e)
{
  Q3GridView::resizeEvent(e);

  KoIconItem *item = currentItem();
  int oldNCols = mNCols;

  if (cellWidth() != 0)
  {
    mNCols = e -> size().width() / cellWidth();
  }

  if(mNCols != oldNCols)
  {
    setNumCols(mNCols);
    calculateCells();
    setCurrentItem(item);
    updateContents();
  }
}

// paint one cell
// mark the current item and center items smaller than the cellSize
// TODO: scale down big pixmaps and paint the size as text into the pixmap
void KoIconChooser::paintCell(QPainter *p, int row, int col)
{
  KoIconItem *item = itemAt(row, col);

  if(item)
  {
    const QPixmap &pix = item->pixmap();

    int x = mMargin;
    int y = mMargin;
    int pw = pix.width();
    int ph = pix.height();
    int cw = cellWidth();
    int ch = cellHeight();

    // center small pixmaps
    if(pw < mItemWidth)
      x = (cw - pw) / 2;
    if(ph < mItemHeight)
      y = (cw - ph) / 2;

    if((!item->hasValidThumb()) || (pw <= mItemWidth && ph <= mItemHeight))
      p->drawPixmap(x, y, pix, 0, 0, mItemWidth, mItemHeight);
    else
    {
      const QPixmap &thumbpix = item->thumbPixmap();
      x = mMargin;
      y = mMargin;
      pw = thumbpix.width();
      ph = thumbpix.height();
      cw = cellWidth();
      ch = cellHeight();

      // center small pixmaps
      if(pw < mItemWidth)
        x = (cw - pw) / 2;
      if(ph < mItemHeight)
        y = (cw - ph) / 2;
      p->drawPixmap(x, y, thumbpix, 0, 0, mItemWidth, mItemHeight);
    }

    // highlight current item
    if(row == mCurRow && col == mCurCol)
    {
      p->setPen(Qt::blue);
      p->drawRect(0, 0, cw, ch);
    }
    else
    {
      p->setPen(Qt::gray);
      //p->drawRect(0, 0, cw, ch);
      p->drawLine(cw-1, 0, cw-1, ch-1);
      p->drawLine(0, ch-1, cw-1, ch-1);
    }
  }
  else
  {
    // empty cell
    p->fillRect(0, 0, cellWidth(), cellHeight(), QBrush(Qt::white));
  }
}

// return the pointer of the item at (row,col) - beware, resizing disturbs
// rows and cols!
// return 0L if item is not found
KoIconItem *KoIconChooser::itemAt(int row, int col) 
{
  return itemAt(cellIndex(row, col));
}

// return the pointer of the item at position index
// return 0L if item is not found
KoIconItem *KoIconChooser::itemAt(int index)
{
  if(index < 0)
    return 0L;
  return mIconList.count() > uint(index) ? mIconList.at(index) : 0;
}

// return the index of a cell, given row and column position
// maps directly to the position in the itemlist
// return -1 on failure
int KoIconChooser::cellIndex(int row, int col)
{
  if(row < 0 || col < 0)
    return -1;
  else
    return((row * mNCols) + col);
}

// calculate the grid and set the number of rows and columns
// reorder all items approrpriately
void KoIconChooser::calculateCells()
{
  if(mNCols == 0)
    return;
  bool update = isUpdatesEnabled();
  int rows = mItemCount / mNCols;
  setUpdatesEnabled(false);
  if((rows * mNCols) < mItemCount)
    rows++;
  setNumRows(rows);
  setUpdatesEnabled(update);
  updateContents();
}

// show the full pixmap of a large item in an extra widget
void KoIconChooser::showFullPixmap(const QPixmap &pix, const QPoint &/*p*/)
{
  //delete mPixmapWidget;
  mPixmapWidget = new KoPixmapWidget(pix, this);

  // center widget under mouse cursor
  QPoint p = QCursor::pos();
  int w = mPixmapWidget->width();
  int h = mPixmapWidget->height();
  mPixmapWidget->move(p.x() - w / 2, p.y() - h / 2);
  mPixmapWidget->show();
}

int KoIconChooser::sortInsertionIndex(const KoIconItem *item)
{
  int index = 0;

  if (!mIconList.isEmpty())
  {
    // Binary insertion
    int first = 0;
    int last = mIconList.count() - 1;
    
    while (first != last)
    {
      int middle = (first + last) / 2;
    
      if (item -> compare(mIconList.at(middle)) < 0)
      {
        last = middle - 1;

        if (last < first)
        {
          last = first;
        }
      }
      else
      {
        first = middle + 1;

        if (first > last)
        {
          first = last;
        }
      }
    }

    if (item -> compare(mIconList.at(first)) < 0)
    {
      index = first;
    }
    else
    {
      index = first + 1;
    }
  }

  return index;
}

KoPatternChooser::KoPatternChooser( const Q3PtrList<KoIconItem> &list, QWidget *parent, const char *name )
 : QWidget( parent, name )
{
    // only serves as beautifier for the iconchooser
    //frame = new QHBox( this );
    //frame->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    chooser = new KoIconChooser( QSize(30,30), this, "pattern chooser" );

	QObject::connect( chooser, SIGNAL(selected( KoIconItem * ) ),
					            this, SIGNAL( selected( KoIconItem * )));

	Q3PtrListIterator<KoIconItem> itr( list );
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
void KoPatternChooser::setCurrentPattern( KoIconItem *pattern )
{
    chooser->setCurrentItem( pattern );
}

void KoPatternChooser::addPattern( KoIconItem *pattern )
{
    chooser->addItem( pattern );
}

// return the active pattern
KoIconItem *KoPatternChooser::currentPattern()
{
    return chooser->currentItem();
}

#include "koIconChooser.moc"
