/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __COMMENT_DELEGATE_H
#define __COMMENT_DELEGATE_H

#include <QStyledItemDelegate>

class StoryboardCommentModel;

/**
 * @class CommentDelegate
 * @brief Paints the comment menu of the storyboard docker
 * and creates widgets for editing data in @c CommentModel.
 */
class CommentDelegate : public QStyledItemDelegate
{
public:
    CommentDelegate(QObject *parent);
    ~CommentDelegate() override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif
