/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
