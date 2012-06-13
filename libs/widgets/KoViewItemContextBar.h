/* This file is part of the KDE project
*
* Copyright (C) 2008 Peter Penz <peter.penz19@gmail.com>
* Copyright (C) 2011 Paul Mendez <paulestebanms@gmail.com>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with this library; see the file COPYING.LIB.  If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef KOVIEWITEMCONTEXTBAR_H
#define KOVIEWITEMCONTEXTBAR_H

#include <QObject>
#include "kowidgets_export.h"
#include <QModelIndex>

class QAbstractItemView;
class QItemSelection;
class QToolButton;
class QHBoxLayout;
class QRect;

/**
 * @brief Add context buttons to items of QAbstractView subclasses
 *
 * Whenever an item is hovered by the mouse, a toggle button is shown
 * which allows to select/deselect the current item, other buttons for
 * custom actions could be added using addContextButton method.
 */
class KOWIDGETS_EXPORT KoViewItemContextBar : public QObject
{
    Q_OBJECT

public:
    KoViewItemContextBar(QAbstractItemView *parent);
    virtual ~KoViewItemContextBar();
    virtual bool eventFilter(QObject *watched, QEvent *event);

    /**
     * Add a button to the context bar
     * @param text to be used for button tool tip
     * @param iconName or name of the icon displayed on the button
     * @return a QToolButton, so it could be connected to a slot.
     */
    QToolButton *addContextButton(QString text, QString iconName);
    //Returns the index of the item under the mouse cursor
    QModelIndex currentIndex();

    int preferredWidth();

signals:
    /** Is emitted if the selection has been changed by the toggle button. */
    void selectionChanged();

public slots:
    /** Hide context bar */
    void reset();
    void enableContextBar();
    void disableContextBar();

private slots:
    void slotEntered(const QModelIndex &index);
    void slotViewportEntered();
    void setItemSelected();
    /** Hide context bar if the selectem item has been removed */
    void slotRowsRemoved(const QModelIndex &parent, int start, int end);
    /** Updates contex bar buttons state*/
    void updateHoverUi(const QModelIndex& index);
    void showContextBar(const QRect &rect);
    /** Updates Selection Button state*/
    void updateToggleSelectionButton();
    /** Update Bar */
    void update();

private:
    void applyPointingHandCursor();
    void restoreCursor();
    QAbstractItemView *m_view;
    bool m_enabled;
    bool m_appliedPointingHandCursor;
    QModelIndex m_IndexUnderCursor;
    QWidget *m_ContextBar;
    QToolButton *m_ToggleSelectionButton;
    QHBoxLayout *m_Layout;
    QList <QToolButton*> m_contextBarButtons;
};

#endif // KOVIEWITEMCONTEXTBAR_H
