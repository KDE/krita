/*
   This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __ko_koolbar_h__
#define __ko_koolbar_h__

#include <q3frame.h>
#include <QPixmap>
#include <q3intdict.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QResizeEvent>
#include <koffice_export.h>
class QPushButton;
class QPixmap;
class KoKoolBar;
class KoKoolBarGroup;

class KoKoolBarItem : public QObject
{
private:
  Q_OBJECT
public:
  KoKoolBarItem( KoKoolBarGroup *_grp, const QPixmap& _pix, const QString& _text = QString::null );

  int id() const { return m_id; }
  void press();
  bool isEnabled() const { return m_bEnabled; }
  void setEnabled( bool _e ) { m_bEnabled = _e; }

  int height() const { return m_iHeight; }
  QPixmap pixmap() const { return m_pixmap; }
  void setText( const QString & text ) { m_strText = text; }
  QString text() const { return m_strText; }

signals:
  void pressed( int _group, int _id );
  void pressed();
protected:
  void calc( QWidget* );

  int m_iHeight;
  KoKoolBarGroup* m_pGroup;
  QString m_strText;
  QPixmap m_pixmap;
  int m_id;
  bool m_bEnabled;
  class KoKoolBarItemPrivate;
  KoKoolBarItemPrivate *d;
};

class KoKoolBarGroup : public QObject
{
  Q_OBJECT
public:
  KoKoolBarGroup( KoKoolBar *_bar, const QString& _text );
  ~KoKoolBarGroup();

  void append( KoKoolBarItem *_i ) { m_mapItems.insert( _i->id(), _i ); }
  void remove( int _id );

  KoKoolBar* bar() const { return m_pBar; }
  QPushButton* button() const { return m_pButton; }
  int id() const { return m_id; }
  bool isEnabled() const { return m_bEnabled; }
  void setEnabled( bool _e ) { m_bEnabled = _e; }
  KoKoolBarItem* item( int _id ) const { return m_mapItems[ _id ]; }
  int items() const { return m_mapItems.size(); }
  Q3IntDictIterator<KoKoolBarItem> iterator() const { return Q3IntDictIterator<KoKoolBarItem>( m_mapItems ); }

public slots:
  void pressed();

protected:
  Q3IntDict<KoKoolBarItem> m_mapItems;
  KoKoolBar* m_pBar;
  QString m_strText;
  int m_id;
  QPushButton* m_pButton;
  bool m_bEnabled;
  class KoKoolBarGroupPrivate;
  KoKoolBarGroupPrivate *d;
};

class KoKoolBarBox : public Q3Frame
{
  Q_OBJECT
public:
  KoKoolBarBox( KoKoolBar *_bar );

  void setActiveGroup( KoKoolBarGroup *_grp );
  int maxHeight() const;

  void sizeChanged() { resizeEvent(0L); }

protected slots:
  void scrollUp();
  void scrollDown();

protected:
  virtual void resizeEvent( QResizeEvent *_ev );
  virtual void drawContents( QPainter * );
  virtual void mousePressEvent( QMouseEvent *_ev )
  { KoKoolBarItem *item = findByPos( _ev->pos().y() + m_iYOffset ); if ( !item ) return; item->press(); }

  KoKoolBarItem* findByPos( int _abs_y ) const;

  bool needsScrolling() const;
  bool isAtBottom() const;
  bool isAtTop() const;
  void updateScrollButtons();

  KoKoolBar *m_pBar;
  int m_iYOffset;
  int m_iYIcon;
  KoKoolBarGroup *m_pGroup;
  QPushButton* m_pButtonUp;
  QPushButton* m_pButtonDown;
  class KoKoolBarBoxPrivate;
  KoKoolBarBoxPrivate *d;
};

class KOFFICEUI_EXPORT KoKoolBar : public QWidget
{
  Q_OBJECT
public:
  KoKoolBar( QWidget *_parent = 0L, const char *_name = 0L );
  virtual ~KoKoolBar() { };

  virtual int insertGroup( const QString& _text );
  virtual int insertItem( int _grp, const QPixmap& _pix, const QString& _text = QString::null,
			  QObject *_obj = 0L, const char *_slot = 0L );
  virtual void removeGroup( int _grp );
  virtual void removeItem( int _grp, int _id );
  virtual void renameItem( int _grp, int _id, const QString & _text );
  virtual void setActiveGroup( int _grp );
  virtual int activeGroup() const { return m_iActiveGroup; }
  virtual void enableItem( int _grp, int _id, bool _enable );
  virtual void enableGroup( int _grp, bool _enable );

protected:
  virtual void resizeEvent( QResizeEvent *_ev );

  Q3IntDict<KoKoolBarGroup> m_mapGroups;

  int m_iActiveGroup;
  KoKoolBarBox* m_pBox;
  class KoKoolBarPrivate;
  KoKoolBarPrivate *d;
};

#endif
