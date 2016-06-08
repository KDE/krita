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
#ifndef STYLESDELEGATE_H
#define STYLESDELEGATE_H

#include <QStyledItemDelegate>

/** This is an internal class, used for the preview of styles in the dropdown of the @class StylesCombo.
  * This class is also responsible for drawing and handling the buttons to call the style manager or to delete a style.
  * NB. Deleting a style is currently not supported, therefore the button has been disabled. */

class StylesDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    StylesDelegate();

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                       const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model,
                             const QStyleOptionViewItem &option, const QModelIndex &index);
    void setEditButtonEnable(bool enable);

Q_SIGNALS:
    void styleManagerButtonClicked(const QModelIndex &index);
    void deleteStyleButtonClicked(const QModelIndex &index);
    void needsUpdate(const QModelIndex &index);
    void clickedInItem(const QModelIndex &index);

private:
    bool m_editButtonPressed;
    bool m_deleteButtonPressed;
    bool m_enableEditButton;

    int m_buttonSize;
    int m_buttonDistance;
};

#endif
