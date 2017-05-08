/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_SHOW_PALETTE_ACTION_H
#define KIS_SHOW_PALETTE_ACTION_H

#include "kis_abstract_input_action.h"

#include <QObject>
#include <QPointer>
class QMenu;

/**
 * \brief Show Palette implementation of KisAbstractInputAction.
 *
 * The Show Palette action shows the popup palette.
 */
class KisShowPaletteAction : public QObject, public KisAbstractInputAction
{
    Q_OBJECT

public:
    explicit KisShowPaletteAction();
    ~KisShowPaletteAction() override;

    int priority() const override;

    void begin(int, QEvent *) override;

private Q_SLOTS:
    void slotShowMenu();

private:
    QPointer<QMenu> m_menu;
    bool m_requestedWithStylus;
};

#endif // KIS_SHOW_PALETTE_ACTION_H
