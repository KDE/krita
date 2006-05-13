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

#ifndef __ko_iconchooser_h__
#define __ko_iconchooser_h__

#include <q3gridview.h>
#include <q3ptrlist.h>
#include <QPixmap>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3Frame>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <koffice_export.h>

class KoIconItem
{
public:
  KoIconItem() {}
  virtual ~KoIconItem() {}

  bool hasValidPixmap() {return validPixmap; }
  bool validPixmap;
  bool hasValidThumb() {return validThumb; }
  bool validThumb;

  virtual int spacing() const {return 0; }
  virtual void setSpacing(int) {}
  virtual QPixmap &pixmap() const = 0;
  virtual QPixmap &thumbPixmap() const = 0;
  // Return -1 if this is less than other, 0 if equal, 1 if greater than.
  virtual int compare(const KoIconItem */*other*/) const { return 0; }
};

class KoPixmapWidget : public Q3Frame
{
public:
  KoPixmapWidget(const QPixmap &aPixmap, QWidget *parent = 0L, const char *name = 0L);
  ~KoPixmapWidget();

protected:
  void paintEvent(QPaintEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);

private:
  QPixmap mPixmap;
};

class KOPAINTER_EXPORT KoIconChooser: public Q3GridView
{
  Q_OBJECT
public:
  // To make the items sorted, set 'sort' to true and override KoIconItem::compare().
  KoIconChooser(QSize iconSize, QWidget *parent = 0L, const char *name = 0L, bool sort = false);
  virtual ~KoIconChooser();

  bool autoDelete() const {return mIconList.autoDelete(); }
  void setAutoDelete(bool b) {mIconList.setAutoDelete(b); }

  void addItem(KoIconItem *item);
  bool removeItem(KoIconItem *item);
  void clear();

  KoIconItem *currentItem();
  void setCurrentItem(KoIconItem *item);

  void setDragEnabled(bool allow) { mDragEnabled = allow; }
  bool dragEnabled() const { return mDragEnabled; }
  
  KoIconItem *itemAt(int row, int col);
  KoIconItem *itemAt(int index);

signals:
  void  selected(KoIconItem *item);

protected:
  void keyPressEvent(QKeyEvent *e);
  void mousePressEvent( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent( QMouseEvent *e);
  void resizeEvent(QResizeEvent *e);
  void paintCell(QPainter *p, int row, int col);
  virtual void startDrag();

private:
  int cellIndex(int row, int col);
  void calculateCells();
  void showFullPixmap(const QPixmap &pix, const QPoint &p);
  int sortInsertionIndex(const KoIconItem *item);

private:
  Q3PtrList<KoIconItem>    mIconList;
  KoPixmapWidget         *mPixmapWidget;
  int                     mItemWidth;
  int                     mItemHeight;
  int                     mItemCount;
  int                     mNCols;
  int                     mCurRow;
  int                     mCurCol;
  int                     mMargin;
  QPoint                  mDragStartPos;
  bool                    mMouseButtonDown;
  bool                    mDragEnabled;
  bool                    mSort;
};

// This is a first attempt at a pattern chooser widget abstraction which is at least
// useful for two applications(karbon and krita). It is really a light version of
// kis_patternchooser. (Rob)
class KOPAINTER_EXPORT KoPatternChooser : public QWidget
{
  Q_OBJECT
public:
  KoPatternChooser( const Q3PtrList<KoIconItem> &list, QWidget *parent, const char *name = 0 );
  ~KoPatternChooser();

  KoIconItem *currentPattern();
  void setCurrentPattern( KoIconItem * );
  void addPattern( KoIconItem * );
 
private:
  KoIconChooser *chooser;

signals:
  void selected( KoIconItem * );
};


#endif
