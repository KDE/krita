/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINPUTEDITORDELEGATE_H
#define KISINPUTEDITORDELEGATE_H

#include <QStyledItemDelegate>

/**
 * \brief A delegate providing editors for the keys/buttons/etc. of KisShortcutConfiguration.
 */
class KisInputEditorDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    KisInputEditorDelegate(QObject *parent = 0);
    ~KisInputEditorDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    class Private;
    Private *const d;
};

#endif // KISINPUTEDITORDELEGATE_H
