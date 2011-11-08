/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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

#include <KIcon>

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QRect>
#include <QStyleOptionButton>

#include <KDebug>

StylesDelegate::StylesDelegate()
    : QStyledItemDelegate(),
      m_editButtonPressed(false),
      m_deleteButtonPressed(false)
{
    m_buttonSize = 16;
    m_buttonDistance = 2;
}

void StylesDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    if (!index.isValid() || !(option.state & QStyle::State_MouseOver)) {
    return;
    }
    // Open style manager dialog button.
    kDebug() << "draw. optionRect: " << option.rect;
    int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
    int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    int dx2 = -m_buttonSize - m_buttonDistance -2;
    int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    QStyleOptionButton optEdit;
    if (!m_editButtonPressed) {
        optEdit.state |= QStyle::State_Enabled;
    }
    optEdit.icon = KIcon("document-properties");
    optEdit.features |= QStyleOptionButton::Flat;
    optEdit.rect = option.rect.adjusted(dx1, dy1, dx2, dy2);
 //   m_editRect = optEdit.rect;
    const_cast<StylesDelegate*>(this)->m_editRect = optEdit.rect;
    kDebug() << "editRect: " << optEdit.rect;
    QApplication::style()->drawControl(QStyle::CE_PushButton, &optEdit, painter, 0);

    // Delete style button.
    dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
    dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    dx2 = -2;
    dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
    QStyleOptionButton optDel;
    if (!m_deleteButtonPressed) {
        optDel.state |= QStyle::State_Enabled;
    }
    optDel.icon = KIcon("edit-delete");
    optDel.features |= QStyleOptionButton::Flat;
    optDel.rect = option.rect.adjusted(dx1, dy1, dx2, dy2);
    const_cast<StylesDelegate*>(this)->m_delRect = optDel.rect;
 //   m_delRect = optDel.rect;
    kDebug() << "delRect: " << optDel.rect;
    QApplication::style()->drawControl(QStyle::CE_PushButton, &optDel, painter, 0);
}

QSize StylesDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(250, 48);
}

bool StylesDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    Q_UNUSED(model);
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        kDebug() << "buttonPressed event pos: " << mouseEvent->pos();
        kDebug() << "optionRect: " << option.rect;
        int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
        int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        int dx2 = - m_buttonSize - m_buttonDistance -2;
        int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect editRect = option.rect.adjusted(dx1, dy1, dx2, dy2);
        kDebug() << "buttonPressed. editRect: " << editRect;
  //      if (editRect.contains(mouseEvent->pos())) {
        if (m_editRect.contains(mouseEvent->pos())) {
            m_editButtonPressed = true;
            kDebug() << "edit button pressed";
        }
        else {
            m_editButtonPressed = false;
        }
        dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
        dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        dx2 = -2;
        dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect delRect = option.rect.adjusted(dx1, dy1, dx2, dy2);
        kDebug() << "buttonPressed. delRect: " << delRect;
//        if (delRect.contains(mouseEvent->pos())){
        if (m_delRect.contains(mouseEvent->pos())){
            m_deleteButtonPressed = true;
        }
        else {
            m_deleteButtonPressed = false;
        }
        emit needsUpdate(index);
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        m_editButtonPressed = false;
        m_deleteButtonPressed = false;
        emit needsUpdate(index);
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        kDebug() << "mouseREleased. event pos: " << mouseEvent->pos();
        int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
        int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        int dx2 = - m_buttonSize - m_buttonDistance -2;
        int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect editRect = option.rect.adjusted(dx1, dy1, dx2, dy2);
        kDebug() << "buttonReleased. editRect: " << editRect;
//        if (editRect.contains(mouseEvent->pos())) {
        if (m_editRect.contains(mouseEvent->pos())) {
            emit styleManagerButtonClicked(index);
            return true;
        }
        dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
        dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        dx2 = -2;
        dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect delRect = option.rect.adjusted(dx1, dy1, dx2, dy2);
        kDebug() << "buttonReleased. delRect: " << delRect;
//        if (delRect.contains(mouseEvent->pos())){
        if (m_delRect.contains(mouseEvent->pos())){
            emit deleteStyleButtonClicked(index);
            return true;
        }
        return false;
    }
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        int dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) - m_buttonSize - m_buttonDistance -2;
        int dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        int dx2 = - m_buttonSize - m_buttonDistance -2;
        int dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect editRect = option.rect.adjusted(dx1, dy1, dx2, dy2);
//        if (!editRect.contains(mouseEvent->pos())) {
        if (m_editRect.contains(mouseEvent->pos())) {
            m_editButtonPressed = false;
        }
        dx1 = option.rect.width() - qMin(option.rect.height()-2, m_buttonSize) -2;
        dy1 = 1 + (option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        dx2 = -2;
        dy2 = -1 -(option.rect.height()-qMin(option.rect.height(), m_buttonSize))/2;
        QRect delRect = option.rect.adjusted(dx1, dy1, dx2, dy2);
//        if (!delRect.contains(mouseEvent->pos())){
        if (m_delRect.contains(mouseEvent->pos())){
            m_deleteButtonPressed = false;
        }
        emit needsUpdate(index);
        return false;
    }
    return false;
}
