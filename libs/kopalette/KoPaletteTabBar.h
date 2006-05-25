/* This file is part of the KDE project
   Copyright (C)  2006 Peter Simonsson <peter.simonsson@gmail.com>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KOPALETTETABBAR_H
#define KOPALETTETABBAR_H

#include <QWidget>

class KoPaletteTabBarPrivate;
class QEvent;

class KoPaletteTabBar : public QWidget
{
  Q_OBJECT

  public:
    KoPaletteTabBar(QWidget* parent);
    ~KoPaletteTabBar();

    /** Adds a tab to the widget.
      * @param icon icon shown in the tab.
      * @param toolTip tool tip text for the tab.
      * @return index of the new tab.
      */
    int addTab(const QIcon& icon, const QString& toolTip);

    /** Inserts a tab at @p index.
      * @param index index to insert the tab at.
      * @param icon icon shown in the tab.
      * @param toolTip tool tip text for the tab.
      */
    void insertTab(int index, const QIcon& icon, const QString& toolTip);

    /// Removes the tab at @p index
    void removeTab(int index);

    /// sets the the size of the icons in the tab labels
    void setIconSize(int size);

    /// Returns true if the tab at @p index is hidden
    bool isTabHidden(int index);

    /// Returns the number of tabs including hidden ones
    int count() const;
    /// Returns the number of visible tabs
    int visibleCount() const;

    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

  public slots:
    /// hides/shows the tab at @p index
    void setTabHidden(int index, bool hide);

  signals:
    /// Emited when the selection of a tab has changed
    void tabSelectionChanged(int index, bool selected);

    /// Emited when all tabs are hidden
    void allTabsHidden();

  protected:
    /// Reimplemented to paint the tabs
    virtual void paintEvent(QPaintEvent* event);

    ///Reimplemented to handle tooltip events
    virtual bool event(QEvent* event);

    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

    /// Calculate the rects of all tabs
    void layoutTabs();

  private:
    KoPaletteTabBarPrivate* d;
};

#endif
