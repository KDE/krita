/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
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
