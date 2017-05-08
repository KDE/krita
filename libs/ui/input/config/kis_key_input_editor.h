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
