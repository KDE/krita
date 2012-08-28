/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "StylesDelegate.h"

#include <KoIcon.h>

#include <QAbstractItemView>
#include <QColor>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QScrollBar>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionViewItemV4>

#include <KDebug>


StylesDelegate::StylesDelegate()
    : QStyledItemDelegate(),
      m_editButtonPressed(false),
      m_deleteButtonPressed(false),
      m_enableEditButton(true)
{
    m_buttonSize = 16;
    m_buttonDistance = 2;
}

void StylesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &optionV1,
                            const QModelIndex &index) const
{
    QStyleOptionViewItemV4 option = optionV1;
    initStyleOption(&option, index);
    QStyledItemDelegate::paint(painter, option, index);

    //the following is needed to find out if the view has vertical scrollbars. If there is no view just paint and do not attempt to draw the control buttons.
    //this is needed because it seems that the option.rect given does not exclude the vertical scrollBar. This means that we can draw the button in an area that is going to be covered by the vertical scrollBar.
    const QAbstractItemView *view = static_cast<const QAbstractItemView*>(option.widget);
    if (!view){
        return;
    }
    QScrollBar *scrollBar = view->verticalScrollBar();
    int scrollBarWidth = 0;
    if (scrollBar->isVisible()) {
        scrollBarWidth = scrollBar->width();
    }

    if (!index.isValid() || !(option.state & QStyle::State_MouseOver)) {
    return;
    }
    // Delete style button.
    int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
    int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    int dx2 = -m_buttonSize - m_buttonDistance -2;
    int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
/* TODO: when we can safely delete styles, re-enable this
    QStyleOptionButton optDel;
    if (!m_deleteButtonPressed) {
        optDel.state |= QStyle::State_Enabled;
    }
    optDel.icon = koIcon("edit-delete");
    optDel.features |= QStyleOptionButton::Flat;
    optDel.rect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
    view->style()->drawControl(QStyle::CE_PushButton, &optDel, painter, 0);
*/
    // Open style manager dialog button.
    if (!m_enableEditButton) {  // when we don't want edit icon
        return;
    }
    dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
    dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    dx2 = -2;
    dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    QStyleOptionButton optEdit;
    if (!m_editButtonPressed) {
        optEdit.state |= QStyle::State_Enabled;
    }
    optEdit.icon = koIcon("document-properties");
    optEdit.features |= QStyleOptionButton::Flat;
    optEdit.rect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
    view->style()->drawControl(QStyle::CE_PushButton, &optEdit, painter, 0);
}

QSize StylesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    return index.data(Qt::SizeHintRole).toSize();
}

bool StylesDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &optionV1, const QModelIndex &index)
{
    Q_UNUSED(model);
    QStyleOptionViewItemV4 option = optionV1;
    initStyleOption(&option, index);

    //the following is needed to find out if the view has vertical scrollbars. If not just paint and do not attempt to draw the control buttons.
    //this is needed because it seems that the option.rect given does not exclude the vertical scrollBar. This means that we can draw the button in an area that is going to be covered by the vertical scrollBar.

    const QAbstractItemView *view = static_cast<const QAbstractItemView*>(option.widget);
    if (!view){
        return false;
    }
    QScrollBar *scrollBar = view->verticalScrollBar();
    int scrollBarWidth = 0;
    if (scrollBar->isVisible()) {
        scrollBarWidth = scrollBar->width();
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int dx1 = option.rect.width()- qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
        int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        int dx2 = - m_buttonSize - m_buttonDistance -2;
        int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
/*TODO: when we can safely delete styles, re-enable this
        QRect delRect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
        if (delRect.contains(mouseEvent->pos())) {
            m_deleteButtonPressed = true;
        }
        else {
            m_deleteButtonPressed = false;
        }
*/
        dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
        dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        dx2 = -2;
        dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect editRect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
        if (editRect.contains(mouseEvent->pos())){
            m_editButtonPressed = true;
        }
        else {
            m_editButtonPressed = false;
        }
        emit needsUpdate(index);
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        m_deleteButtonPressed = false;
        m_editButtonPressed = false;
        emit needsUpdate(index);
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
        int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        int dx2 = - m_buttonSize - m_buttonDistance -2;
        int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
/*TODO: when we can safely delete styles, re-enable this
        QRect delRect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
        if (delRect.contains(mouseEvent->pos())) {
            emit deleteStyleButtonClicked(index);
            return true;
        }
*/
        dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
        dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        dx2 = -2;
        dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect editRect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
        if (editRect.contains(mouseEvent->pos())){
            emit styleManagerButtonClicked(index);
            return true;
        }
        emit clickedInItem(index);
        return false;
    }
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
        int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        int dx2 = - m_buttonSize - m_buttonDistance -2;
        int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
/*TODO: when we can safely delete styles, re-enable this
        QRect delRect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
        if (!delRect.contains(mouseEvent->pos())) {
            m_deleteButtonPressed = false;
        }
*/
        dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
        dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        dx2 = -2;
        dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect editRect = option.rect.adjusted(dx1 - scrollBarWidth, dy1, dx2 - scrollBarWidth, dy2);
        if (!editRect.contains(mouseEvent->pos())){
            m_editButtonPressed = false;
        }
        emit needsUpdate(index);
        return false;
    }
    return false;
}

void StylesDelegate::setEditButtonEnable(bool enable)
{
    m_enableEditButton = enable;
}
