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

#include "storyboardDelegate.h"

#include <QLineEdit>
#include <QDebug>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include <QSize>
#include <QMouseEvent>
#include <QListView>

#include <kis_icon.h>
#include "storyboardModel.h"

StoryboardDelegate::StoryboardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

StoryboardDelegate::~StoryboardDelegate()
{
}

void StoryboardDelegate::paint(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    p->save();
    {
        QStyle *style = option.widget ? option.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, p, option.widget);

        p->setFont(option.font);
        if (!index.isValid()){
            p->restore();
            return;
        }
        if (!index.parent().isValid()){
            QRect parentRect = option.rect;

            QSize cellSize = sizeHint(option, index);
            parentRect.setSize(cellSize);
            p->drawRect(parentRect);

            //draw frame number rect
            parentRect.setTopLeft(parentRect.topLeft() + QPoint(5, 5));
        }
        else{
            //paint Child index (the indices that hold data)
            QModelIndex parent = index.parent();

            //draw the child items
            int rowNum = index.model()->rowCount(parent);
            int childNum = index.row();
            QString data = index.model()->data(index, Qt::DisplayRole).toString();

            switch (childNum)
            {
                case 0:
                {
                    QRect frameNumRect = option.rect;
                    frameNumRect.setHeight(m_view->fontMetrics().height()+3);
                    frameNumRect.setWidth(3 * m_view->fontMetrics().width("0")+2);
                    frameNumRect.moveBottom(option.rect.top()-1);
                    p->drawRect(frameNumRect);
                    p->drawText(frameNumRect, Qt::AlignHCenter | Qt::AlignVCenter, data);

                    QIcon icon = KisIconUtils::loadIcon("krita-base");
                    icon.paint(p, option.rect);
                    p->drawRect(option.rect);
                    break;
                }
                case 1:
                {
                    QRect itemNameRect = option.rect;
                    itemNameRect.setLeft(option.rect.left() + 5);
                    p->drawText(itemNameRect, Qt::AlignLeft | Qt::AlignVCenter, data);
                    p->drawRect(option.rect);
                    break;
                }
                case 2:
                {
                    p->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, data);
                    //TODO: draw spin boxes
                    p->drawRect(option.rect);
                    break;
                }
                case 3:    //frame duration
                {
                    p->drawText(option.rect, Qt::AlignHCenter | Qt::AlignVCenter, data);
                    //TODO: draw spin boxes.
                    p->drawRect(option.rect);
                    break;
                }
                default:
                {
                    p->drawRect(option.rect);
                    break;
                }
            }
        }
    }
    p->restore();
}


QSize StoryboardDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.parent().isValid()){
        int width = option.widget->width() - 17;
        const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
        int numComments = model->commentCount();
        int numItem = width/200;
        if(numItem <=0){
            return QSize(0, 0);
        }
        return QSize(width / numItem, 140 + numComments*100);
        //return QSize(200,250);
    }
    else {
        return option.rect.size();
    }
    return QSize(0,0);
}

/*
QWidget *StoryboardDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option ,
    const QModelIndex &index) const
{
    //only create editor for children
    if (index.parent().isValid()){
        int row = index.row();
        switch (row)
        {
            case 0:
            case 2:            //we handle spinbox edit event separately
            {
                return nullptr;
            }
            default:             // for itemName and comments
            {
                QLineEdit *editor = new QLineEdit(parent);
                return editor;
            }
        }
    }
    return nullptr;
}

bool StoryboardDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem newOption = option;

    if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        && (index.flags() & Qt::ItemIsEnabled))
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        //handle the duration edit event
        if (index.row() == 2){
        }
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
void StoryboardDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);
}

void StoryboardDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
    QString value = lineEdit->text();

    //don't add empty string
    model->setData(index, value, Qt::EditRole);

    //do we need to emit closeEditor() ???
}

void StoryboardDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editor->setGeometry(option.rect);
    qDebug()<<"setting geometry";
}
*/
void StoryboardDelegate::setView(QListView *view){
    m_view = view;
}