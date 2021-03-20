/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
