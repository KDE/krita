/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "CommentDelegate.h"

#include <QLineEdit>
#include <QDebug>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include <QSize>
#include <QMouseEvent>

#include <kis_icon.h>

CommentDelegate::CommentDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

CommentDelegate::~CommentDelegate()
{
}

void CommentDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    p->save();
    {
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, option.widget);

        p->setFont(option.font);

        {
            QIcon icon =index.model()->data(index, Qt::DecorationRole).value<QIcon>();
            QRect r = option.rect;
            r.setSize(QSize(22, 22));
            icon.paint(p, r);
        }
        {
            QRect r = option.rect;
            r.translate(25, 0);
            QString value = index.model()->data(index, Qt::DisplayRole).toString();

            p->drawText(r, Qt::AlignLeft | Qt::AlignVCenter, value);
        }
    }
    p->restore();
}

QSize CommentDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QSize(option.rect.width(), 22);
}

QWidget *CommentDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option ,
    const QModelIndex &index) const
{
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

bool CommentDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem newOption = option;

    if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        && (index.flags() & Qt::ItemIsEnabled)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        QRect visibilityRect = option.rect;
        visibilityRect.setSize(QSize(22, 22));
        const bool visibilityClicked = visibilityRect.isValid() &&
            visibilityRect.contains(mouseEvent->pos());

        const bool leftButton = mouseEvent->buttons() & Qt::LeftButton;

        if (leftButton && visibilityClicked) {
            model->setData(index, true, Qt::DecorationRole);
            return true;
        }
    }
    return false;
}

//set the existing data in the editor
void CommentDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);
}

void CommentDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString value = lineEdit->text();

    //TO DO: don't add empty string
    model->setData(index, value, Qt::EditRole);
}

void CommentDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
}