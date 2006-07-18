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
#include <QFrame>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <koffice_export.h>

class KOPAINTER_EXPORT KoIconChooser: public QTableWidget
{
    Q_OBJECT
public:
    // To make the items sorted, set 'sort' to true and override QTableWidgetItem::compare().
    KoIconChooser(QSize iconSize, QWidget *parent = 0L);
    virtual ~KoIconChooser();

    void addItem(QTableWidgetItem *item);
    QTableWidgetItem *itemAt(int index);

protected:
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent(QKeyEvent * e);

private:
    int mItemWidth;
    int mItemHeight;
    int m_itemCount;
};

// This is a first attempt at a pattern chooser widget abstraction which is at least
// useful for two applications(karbon and krita). It is really a light version of
// kis_patternchooser. (Rob)
class KOPAINTER_EXPORT KoPatternChooser : public QWidget
{
  Q_OBJECT
public:
  KoPatternChooser( const Q3PtrList<QTableWidgetItem> &list, QWidget *parent, const char *name = 0 );
  ~KoPatternChooser();

  QTableWidgetItem *currentPattern();
  void setCurrentPattern( QTableWidgetItem * );
  void addPattern( QTableWidgetItem * );
 
private:
  KoIconChooser *chooser;

signals:
  void selected( QTableWidgetItem * );
};


#endif
