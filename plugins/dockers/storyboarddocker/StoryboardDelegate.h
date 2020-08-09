/*
 *  Copyright (c) 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __STORYBOARD_DELEGATE_H
#define __STORYBOARD_DELEGATE_H

#include <QStyledItemDelegate>
#include "StoryboardView.h"
#include <kis_types.h>
#include <kis_image.h>

class QListView;
class StoryboardModel;

class StoryboardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    StoryboardDelegate(QObject *parent);
    ~StoryboardDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
    void setView(StoryboardView *view);

    /**
     * @brief Draw the spin box.
     */
    void drawSpinBox(QPainter *p, const QStyleOptionViewItem &option, QString data, QString suffix) const;

    /**
     * @brief Draw the spin box.
     */
    QStyleOptionSlider drawComment(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
    /**
     * @return rectangle for Spinbox Up button for spin box at @c option.rect
     */
    QRect spinBoxUpButton(const QStyleOptionViewItem &option);

    /**
     * @return rectangle for Spinbox down button for spin box at @c option.rect
     */
    QRect spinBoxDownButton(const QStyleOptionViewItem &option);

    /**
     * @return rectangle for Spinbox edit field for spin box at @c option.rect
     */
    QRect spinBoxEditField(const QStyleOptionViewItem &option);

    /**
     * @return rectangle for Scrollbar(the rectangular controller) for scroll bar at @c option.rect
     */
    QRect scrollBar(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption) const;

    /**
     * @return rectangle for Scrollbar's down button for scroll bar at @c option.rect
     */
    QRect scrollDownButton(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption);

    /**
     * @return rectangle for Scrollbar's up button for scroll bar at @c option.rect
     */
    QRect scrollUpButton(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption);
    void setImageSize(QSize imageSize);

private Q_SLOTS:

    /**
     * @brief updates the scroll value of the @c CommentBox in @c StoryboardModel
     * This enables the model to keep track of the part of the comment that has to be drawn in delegate.
     */
    void slotCommentScrolledTo(int value) const;

private:
    StoryboardView *m_view;
    QPoint m_lastDragPos = QPoint(0, 0);
    QSize m_imageSize;
};

#endif
