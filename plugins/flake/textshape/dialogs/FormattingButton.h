/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 C. Boemann <cbo@boemann.dk>
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

#ifndef FORMATTINGBUTTON_H
#define FORMATTINGBUTTON_H

#include <KoListStyle.h>

#include <QToolButton>
#include <QPixmap>
#include <QMap>

class QMenu;
class QAction;
class ItemChooserAction;

class FormattingButton : public QToolButton
{
    Q_OBJECT
public:
    explicit FormattingButton(QWidget *parent = 0);

    void setNumColumns(int columns);
    void setItemsBackground(const QColor &color);
    void addItem(const QPixmap &pm, int id, const QString &toolTip = QString());
    void addAction(QAction *action);
    void addBlanks(int n);
    void addSeparator();
    bool hasItemId(int id);
    bool isFirstTimeMenuShown();

Q_SIGNALS:
    void itemTriggered(int id);
    void doneWithFocus();
    void aboutToShowMenu();

private Q_SLOTS:
    void itemSelected();
    void menuShown();

private:
    int m_lastId;
    QMenu *m_menu;
    QMap<int, QObject *> m_styleMap;
    ItemChooserAction *m_styleAction;
    int m_columns;
    bool m_menuShownFirstTime;
};

#endif  //FORMATTINGBUTTON_H
