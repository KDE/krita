/*  This file is part of the Calligra project, made within the KDE community.
 *
 * Copyright 2012  Friedrich W. H. Kossebau <kossebau@kde.org>
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

#ifndef KOPAGENAVIGATORBUTTON_H
#define KOPAGENAVIGATORBUTTON_H

// Qt
#include <QToolButton>

class QAction;

class KoPageNavigatorButton : public QToolButton
{
    Q_OBJECT

public:
    KoPageNavigatorButton(const char *iconName, QWidget *parent);

    void setAction(QAction *action);
    QAction *action() const { return m_action; }

private Q_SLOTS:
    void onActionChanged();
    void onClicked();

private:
    QAction *m_action;
};

#endif
