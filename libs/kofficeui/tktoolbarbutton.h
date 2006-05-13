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
#ifndef TKTOOLBARBUTTON_H
#define TKTOOLBARBUTTON_H

#include <tkaction.h>

#include <QPixmap>
#include <QToolButton>
//Added by qt3to4:
#include <QEvent>
#include <Q3PopupMenu>
#include <kglobal.h>

class KToolBar;
class KInstance;
class KMenu;
class QPainter;
class QIcon;

class TKToolBarButton : public QToolButton
{ Q_OBJECT
public:
  TKToolBarButton(const QString& icon, const QString& txt,
                     QWidget* parent = 0, const char *name=0L,
                     KInstance *_instance = KGlobal::instance());

  TKToolBarButton(const QPixmap&, const QString&, QWidget* parent=0, const char* name=0);
  ~TKToolBarButton();

  void setIconMode(TK::IconMode);
  void setRaised(bool);
  void setAutoRaised(bool);

  /**
   * Enable/Disable this button
   *
   * @param enable Defaults to true
   */
  void setEnabled(bool enable = true);

  /**
   * Set the pixmap directly for this button.  This pixmap should be
   * the active one... the dimmed and disabled pixmaps are constructed
   * based on this one.  However, don't use this function unless you
   * are positive that you don't want to use @ref setIcon.
   *
   * @param pixmap The active pixmap
   */
  virtual void setPixmap(const QPixmap &pixmap);

  /**
   * Set the pixmap directly for this button.  This pixmap should be
   * the active one.. however, the disabled and default pixmaps will
   * only be constructed if @ref #generate is true.  In any event,
   * don't use this function unless you are positive that you don't
   * want to use @ref setIcon.
   *
   * @param pixmap   The active pixmap
   * @param generate If true, then the other pixmaps will be
   *                 automatically generated using configurable effects
   */
  virtual void setPixmap(const QPixmap &pixmap, bool generate);

  /**
   * Force the button to use this pixmap as the default one rather
   * then generating it using effects.
   *
   * @param pixmap The pixmap to use as the default (normal) one
   */
  virtual void setDefaultPixmap(const QPixmap& pixmap);

  /**
   * Force the button to use this pixmap when disabled one rather then
   * generating it using effects.
   *
   * @param pixmap The pixmap to use when disabled
   */
  virtual void setDisabledPixmap(const QPixmap& pixmap);

  /**
   * Set the text for this button.  The text will be either used as a
   * tooltip (IconOnly) or will be along side the icon
   *
   * @param text The button (or tooltip) text
   */
  virtual void setText(const QString &text);
  QString text();

  /**
   * Set the icon for this button.  This icon should be the active
   * one... the dimmed and disabled icons are constructed based on
   * this one.  The actual pixmap will be loaded internally.  This
   * function is preferred over @ref setPixmap
   *
   * @param icon The name of the active pixmap
   */
  virtual void setIcon(const QString &icon);

  /**
   * Force the button to use this icon as the default one rather
   * then generating it using effects.
   *
   * @param icon The icon to use as the default (normal) one
   */
  virtual void setDefaultIcon(const QString& icon);

  /**
   * Force the button to use this icon when disabled one rather then
   * generating it using effects.
   *
   * @param icon The icon to use when disabled
   */
  virtual void setDisabledIcon(const QString& icon);

  /**
   * Turn this button on or off
   *
   * @param flag true or false
   */
  void on(bool flag = true);

  /**
   * Toggle this button
   */
  void toggle();

  /**
   * Turn this button into a toggle button or disable the toggle
   * aspects of it.  This does not toggle the button itself.  Use @ref
   * toggle for that.
   *
   * @param toggle true or false
   */
  void setToggle(bool toggle = true);

  /**
   * Return a pointer to this button's popup menu (if it exists)
   */
  KMenu *popup();

  /**
   * Give this button a popup menu.  There will not be a delay when
   * you press the button.  Use @ref setDelayedPopup if you want that
   * behavior
   *
   * @param p The new popup menu
   */
  void setPopup (KMenu *p);

  /**
   * Gives this button a delayed popup menu.
   *
   * This function allows you to add a delayed popup menu to the button.
   * The popup menu is then only displayed when the button is pressed and
   * held down for about half a second.  You can also make the poup-menu
   * "sticky", i.e. visible until a selection is made or the mouse is
   * clikced elsewhere, by simply setting the second argument to true.
   * This "sticky" button feature allows you to make a selection without
   * having to press and hold down the mouse while making a selection.
   *
   * @param p the new popup menu
   * @param toggle if true, makes the button "sticky" (toggled)
   */
  void setDelayedPopup(KMenu *p, bool toggle = false);

  QPixmap getActivePixmap() const;

  virtual QSize sizeHint() const;
  virtual QSize minimumSizeHint() const;

signals:
  void buttonClicked();
  void buttonPressed();
  void buttonReleased();
  void buttonToggled();

public slots:
  void modeChange();

protected:
  void paletteChange(const QPalette &);
  void leaveEvent(QEvent *e);
  void enterEvent(QEvent *e);
  void drawButton(QPainter *p);
  bool eventFilter (QObject *o, QEvent *e);
  void showMenu();

  void makeDefaultPixmap();
  void makeDisabledPixmap();
  bool arrowPressed( const QPoint& pos ) {
	int x = pos.x();
	int y = pos.y();
	return (x > width() - 12 && x <= width() && y > 0 && y < height());
  }

private:
  QPixmap defaultPixmap;
  QPixmap activePixmap;
  QPixmap disabledPixmap;
  virtual void setIcon(const QPixmap &p) { QWidget::setIcon(p); }
  class TKToolBarButtonPrivate;
  TKToolBarButtonPrivate *d;


protected slots:
  void slotClicked();
  void slotPressed();
  void slotReleased();
  void slotToggled();
  void slotDelayTimeout();
};

#endif
