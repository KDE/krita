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

#ifndef KISINPUTMODEDELEGATE_H
#define KISINPUTMODEDELEGATE_H

#include <QStyledItemDelegate>

class KisAbstractInputAction;
/**
 * \brief A delegate providing editors for the mode property of KisShortcutConfiguration.
 */
class KisInputModeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit KisInputModeDelegate(QObject *parent = 0);
    ~KisInputModeDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;

    void setAction(KisAbstractInputAction *action);

private:
    class Private;
    Private *const d;
};

#endif // KISINPUTMODEDELEGATE_H
