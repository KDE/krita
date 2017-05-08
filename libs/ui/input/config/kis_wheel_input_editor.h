/*
 * This file is part of the KDE project
 * Copyright (C) 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISWHEELINPUTEDITOR_H
#define KISWHEELINPUTEDITOR_H

#include <QPushButton>

#include "input/kis_shortcut_configuration.h"

namespace Ui
{
class KisWheelInputEditor;
}

/**
 * \brief An editor widget for mouse wheel input with modifiers.
 */
class KisWheelInputEditor : public QPushButton
{
    Q_OBJECT
public:
    KisWheelInputEditor(QWidget *parent = 0);
    ~KisWheelInputEditor() override;

    QList<Qt::Key> keys() const;
    void setKeys(const QList<Qt::Key> &newKeys);

    KisShortcutConfiguration::MouseWheelMovement wheel() const;
    void setWheel(KisShortcutConfiguration::MouseWheelMovement newWheel);

private Q_SLOTS:
    void updateLabel();

private:
    class Private;
    Private *const d;
};

#endif // KISWHEELINPUTEDITOR_H
