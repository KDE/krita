/* This file is part of the KDE project
 * Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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

#ifndef TABLECREATEBUTTON_H
#define TABLECREATEBUTTON_H

#include <QToolButton>

class QMenu;

class QuickTableButton : public QToolButton
{
    Q_OBJECT
public:
    explicit QuickTableButton(QWidget *parent = 0);
    void emitCreate(int rows, int columns);
    void addAction(QAction *action);

Q_SIGNALS:
    void create(int rows, int columns);

private:
    QMenu *m_menu;
};

#endif
