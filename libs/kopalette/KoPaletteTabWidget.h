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

#ifndef KOPALETTETABWIDGET_H
#define KOPALETTETABWIDGET_H

#include <QWidget>
#include <QMap>

class QToolButton;
class QVBoxLayout;
class KoPaletteTabBar;

class KoPaletteTabWidget : public QWidget
{
  Q_OBJECT

  public:
    KoPaletteTabWidget(QWidget* parent);
    ~KoPaletteTabWidget();

    /** Adds a new tab to the widget
      * @param child the widget shown for this tab
      * @param icon the icon used as label for the tab
      * @param toolTip tool tip text shown when hovering the tab label
      */
    void addTab(QWidget* child, const QIcon& icon, const QString& toolTip);

    /** Inserts a new tab in the widget at @p index
      * @param index the index to insert the tab at
      * @param child the widget shown for this tab
      * @param icon the icon used as label for the tab
      * @param toolTip tool tip text shown when hovering the tab label
      */
    void insertTab(int index, QWidget* child, const QIcon& icon, const QString& toolTip);

    /** Removes a tab from the widget
      * @param child the widget to remove
      */
    void removeTab(QWidget* child);
    /** Removes a tab from the widget
      * @param index index of the tab to remove
      */
    void removeTab(int index);

    /** Takes the tab from the widget but doesn't delete the child widget
      * @param child the widget to take away
      */
    void takeTab(QWidget* child);

    /// Returns the index of @p child
    int indexOf(QWidget* child);

    /// Returns true if the tab at @p index is hidden
    bool isTabHidden(int index);

    /// Returns the number of tabs including hidden ones
    int count() const;
    /// Returns the number of visible tabs
    int visibleCount() const;

  public slots:
    /// Hide the tab in the tab bar
    void setTabHidden(int index, bool hide);

  protected slots:
    /// Sets if tab @p index should be shown or not.
    void setTabActive(int index, bool active);

  private:
    KoPaletteTabBar* m_tabBar;
    QVBoxLayout* m_tabLayout;

    QList<QWidget*> m_widgets;
};

#endif
