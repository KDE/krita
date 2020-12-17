/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISKEYINPUTEDITOR_H
#define KISKEYINPUTEDITOR_H

#include <QPushButton>

namespace Ui
{
class KisKeyInputEditor;
}

/**
 * \brief An editor widget for a list of keys.
 */
class KisKeyInputEditor : public QPushButton
{
    Q_OBJECT
public:
    KisKeyInputEditor(QWidget *parent = 0);
    ~KisKeyInputEditor() override;

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

#endif // KISKEYINPUTEDITOR_H
