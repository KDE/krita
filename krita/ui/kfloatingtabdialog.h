/*
 *  kfloatingtabdialog.h - part of KImageShop
 *
 *  based on KTabCtl Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#ifndef __kfloatingtabdialog_h__
#define __kfloatingtabdialog_h__

#include "kfloatingdialog.h"
#include <qtabbar.h>
#include <qmemarray.h>

class KFloatingTabDialog : public KFloatingDialog
{
  Q_OBJECT
	
 public:
  KFloatingTabDialog(QWidget *parent = 0, const char *name = 0);
  ~KFloatingTabDialog();
  
  void show();
  void setFont(const QFont & font);
  void setTabFont( const QFont &font );
  
  void addTab(QWidget *, const QString&);
  bool isTabEnabled(const QString& );
  void setTabEnabled(const QString&, bool);
  void setShape( QTabBar::Shape shape );
  virtual QSize sizeHint(void) const;
  
 protected:
  void resizeEvent(QResizeEvent *);

 signals:
  void tabSelected(int);
    
 protected slots:
 void showTab(int i);

 protected:
 void setSizes();
 QRect getChildRect();

 QTabBar * tabs;
 QMemArray<QWidget *> pages;
};
#endif
