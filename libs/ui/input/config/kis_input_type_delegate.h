/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINPUTTYPEDELEGATE_H
#define KISINPUTTYPEDELEGATE_H

#include <QStyledItemDelegate>

/**
 * \brief A delegate providing editors for the type property of KisShortcutConfiguration.
 */
class KisInputTypeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit KisInputTypeDelegate(QObject *parent = 0);
    ~KisInputTypeDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;

private:
    class Private;
    Private *const d;
};

#endif // KISINPUTTYPEDELEGATE_H
