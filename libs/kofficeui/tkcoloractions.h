/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef TKCOLORACTION_H
#define TKCOLORACTION_H

#include "tkaction.h"
#include <koffice_export.h>
#include <kmenu.h>
#include <q3dict.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <Q3GridLayout>
#include <QShowEvent>
#include <Q3Frame>
#include <QEvent>
#include <QPaintEvent>

class Q3GridLayout;
class TKColorPanel;
class TKSelectColorActionPrivate;

class TKColorPopupMenu : public KMenu
{ Q_OBJECT
public:
  TKColorPopupMenu( QWidget* parent = 0, const char* name = 0 );
  ~TKColorPopupMenu();

public slots:
  void updateItemSize();
};
/****************************************************************************************/
class KOFFICEUI_EXPORT TKSelectColorAction : public TKAction
{ Q_OBJECT
public:
  enum Type {
    TextColor,
    LineColor,
    FillColor,
    Color
  };

  TKSelectColorAction( const QString& text, Type type, KActionCollection* parent, const char* name, bool menuDefaultColor=false);
  TKSelectColorAction( const QString& text, Type type,
                       QObject* receiver, const char* slot,
                       KActionCollection* parent, const char* name,bool menuDefaultColor=false );

  virtual ~TKSelectColorAction();

  QColor color() const { return m_pCurrentColor; }

  KMenu* popupMenu() const { return m_pMenu; }
  void setDefaultColor(const QColor &_col);


public slots:
  void setCurrentColor( const QColor& );
  void setActiveColor( const QColor& );
  virtual void activate();

signals:
  void colorSelected( const QColor& );

protected slots:
  void selectColorDialog();
  void panelColorSelected( const QColor& );
  void panelReject();
  virtual void slotActivated();
  void defaultColor();

protected:
  void init();
  virtual void initToolBarButton(TKToolBarButton*);
  void updatePixmap();
  void updatePixmap(TKToolBarButton*);

protected:
  TKColorPopupMenu* m_pMenu;
  TKColorPanel* m_pStandardColor;
  TKColorPanel* m_pRecentColor;
  int m_type;

  QColor m_pCurrentColor;

private:
  TKSelectColorActionPrivate *d;
};
/****************************************************************************************/
class TKColorPanelButton : public Q3Frame
{ Q_OBJECT
public:
  TKColorPanelButton( const QColor&, QWidget* parent, const char* name = 0 );
  ~TKColorPanelButton();

  void setActive( bool );

  QColor panelColor() const { return m_Color; }

signals:
  void selected( const QColor& );

protected:
  virtual void paintEvent( QPaintEvent* );
  virtual void enterEvent( QEvent* );
  virtual void leaveEvent( QEvent* );
  virtual void mouseReleaseEvent( QMouseEvent* );

  QColor m_Color;
  bool m_bActive;

private:
  class TKColorPanelButtonPrivate;
  TKColorPanelButtonPrivate *d;
};
/****************************************************************************************/
class TKColorPanel : public QWidget
{ Q_OBJECT

public:
  TKColorPanel( QWidget* parent = 0L, const char* name = 0 );
  ~TKColorPanel();

  void setActiveColor( const QColor& );
  void setNumCols( int col );
  void clear();

public slots:
  void insertColor( const QColor& );
  void insertColor( const QColor&, const QString& );
  void selected( const QColor& );

signals:
  void colorSelected( const QColor& );
  void reject();
  void sizeChanged();

protected:
  void addToGrid( TKColorPanelButton* );
  void resetGrid();

  virtual void mouseReleaseEvent( QMouseEvent* );
  virtual void showEvent( QShowEvent *e );

  Q3GridLayout* m_pLayout;
  int m_iWidth;
  int m_iX;
  int m_iY;

  QColor m_activeColor;
  Q3Dict<TKColorPanelButton> m_pColorDict;

private:
  void fillPanel();

  class TKColorPanelPrivate;
  TKColorPanelPrivate *d;
};

#endif
