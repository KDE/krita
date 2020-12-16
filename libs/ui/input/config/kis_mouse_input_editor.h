/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMOUSEINPUTEDITOR_H
#define KISMOUSEINPUTEDITOR_H

#include <QPushButton>

namespace Ui
{
class KisMouseInputEditor;
}

/**
 * \brief An editor widget for mouse buttons with modifiers.
 */
class KisMouseInputEditor : public QPushButton
{
    Q_OBJECT
public:
    KisMouseInputEditor(QWidget *parent = 0);
    ~KisMouseInputEditor() override;

    QList<Qt::Key> keys() const;
    void setKeys(const QList<Qt::Key> &newKeys);

    Qt::MouseButtons buttons() const;
    void setButtons(Qt::MouseButtons newButtons);

private Q_SLOTS:
    void updateLabel();

private:
    class Private;
    Private *const d;
};

#endif // KISMOUSEINPUTEDITOR_H
