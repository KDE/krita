/*
 *  iconchooser.h - part of KImageShop
 *
 *  A general chooserwidget, showing items represented by pixmaps
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef ICONCHOOSER_H
#define ICONCHOOSER_H

#include <qevent.h>
#include <qgridview.h>
#include <qpixmap.h>
#include <qptrlist.h>

class QColor;
class QFrame;
class QPainter;
class IconItem;
class PixmapWidget;

class IconChooser: public QGridView {
  Q_OBJECT

  typedef QGridView super;

public:
  IconChooser( QWidget *parent, QSize iconSize, const char *name );
  virtual ~IconChooser();


  void		addItem( IconItem * );
  bool 		removeItem( IconItem * );
  IconItem *	currentItem();
  void 		setCurrentItem( IconItem * );
  void 		clear();
  void 		setAutoDelete( bool b )       { iconList.setAutoDelete( b );  }
  bool 		autoDelete() 		const { return iconList.autoDelete(); }
  void 		setBackgroundColor( const QColor& color ) const;
  const QColor& backgroundColor()	const { return bgColor;		      }


protected:
  int 		count() 		const { return itemCount; }
  IconItem *	itemAt( int row, int col );
  IconItem *	itemAt( int index );
  int 		cellIndex( int row, int col );
  void 		calculateCells();
  void 		showFullPixmap( const QPixmap&, const QPoint& );
  virtual void 	keyPressEvent( QKeyEvent * );
  virtual void	mousePressEvent( QMouseEvent * );
  virtual void 	mouseReleaseEvent( QMouseEvent * );
  virtual void	paintCell( QPainter *, int, int );
  virtual void 	resizeEvent( QResizeEvent * );

  int 		nCols;
  int 		curRow;
  int 		curCol;
  int 		itemWidth;
  int 		itemHeight;
  int 		margin;
  QColor 	bgColor;
  QPtrList<IconItem> iconList;
  PixmapWidget 	*pw;

private:
  int 		itemCount;


signals:
  void 		selected( IconItem * );

};


class PixmapWidget : public QFrame
{
public:
  PixmapWidget( const QPixmap&, QWidget *parent=0, const char *name=0 );
  ~PixmapWidget() {}

private:
  void 		paintEvent( QPaintEvent * );

  QPixmap 	pixmap;

};


#endif // ICONCHOOSER_H
