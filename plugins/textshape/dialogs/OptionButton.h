/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Casper Boemann <cbo@boemann.dk>
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

#ifndef OPTIONBUTTON_H
#define OPTIONBUTTON_H

#include <KoListStyle.h>

#include <QToolButton>
#include <QPixmap>
#include <QMap>

class QMenu;
class QAction;

class OptionButton : public QToolButton
{
    Q_OBJECT
public:
    OptionButton( QWidget *parent = 0 );

    void addItem(QPixmap pm, int id);
    
    void setDeactiveId(QPixmap pm, int id);

signals:
    void itemTriggered(int id);

private slots:
    void itemSelected(QAction *action);

private:
    bool m_firstItemAdded;
    QMenu *m_menu;
    QMap<QAction *, int > m_actionMap;
};

#endif
