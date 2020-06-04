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

            QSize itemSize = QSize(190, 240);
            parentRect.translate(5 + (cellSize.width() - itemSize.width())/2, 5);
            parentRect.setSize(itemSize);

            //draw the child items
            const QAbstractItemModel *model = index.model();
            for (int row = 0; row < model->rowCount(); row++){
                QModelIndex childIndex = model->index(row, 0, index);
                switch (row)
                {
                    case 0:
                    {
                        int frameNum = model->data(childIndex, Qt::DisplayRole).toInt();
                        QString value = QString::number(frameNum);

                        QRect frameRect = parentRect;

                        frameRect.setHeight(20);
                        p->drawRect(frameRect);
                        frameRect.setWidth(20);
                        p->drawRect(frameRect);

                        frameRect.translate(5,5);
                        frameRect.setSize(QSize(10,15));
                        p->drawText(frameRect, Qt::AlignLeft | Qt::AlignVCenter, value);

                        //drawImage rect
                        QRect imageRect = parentRect;
                        imageRect.translate(0,20);
                        imageRect.setHeight(115);
                        p->drawRect(imageRect);

                        //draw image(placeholder for now)
                        QIcon icon = KisIconUtils::loadIcon("krita-base");
                        icon.paint(p, imageRect);
                        break;
                        //drawFrame(p, option, index, parentRect);

                    }
                    case 1:
                    {
                        QString itemName = model->data(childIndex, Qt::DisplayRole).toString();
                        QRect nameRect = parentRect;

                        nameRect.setSize(QSize(120, 20));
                        nameRect.translate(20, 0);
                        p->drawRect(nameRect);
                        nameRect.setWidth(110);
                        p->drawText(nameRect, Qt::AlignRight | Qt::AlignVCenter, itemName);

                        break;
                        //drawItemName(p, option, index);
                    }
                    case 2:
                    {
                        int duration = model->data(childIndex, Qt::DisplayRole).toInt();
                        QRect durationRect = parentRect;

                        durationRect.setSize(QSize(50, 20));
                        durationRect.translate(140, 0);

                        QStyleOptionSpinBox opt;
                        opt.rect = durationRect;
                        opt.state = option.state;
                        style->drawComplexControl(QStyle::CC_SpinBox, &opt, p, option.widget);
                        break;
                    }    //drawDuration()
                    default:
                    {
                        QString comment = model->data(childIndex, Qt::DisplayRole).toString();
                        //should we use Qt::TextWordWrap ??
                    }
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
        int numItem = width/200;
        int spacingItem = (width - numItem*200) / (numItem + 1);
        if (option.Middle){
            return QSize(200 + spacingItem, 250);
        }
        else{
            return QSize(200 + (3 * spacingItem)/2, 250);
        }
    }
    /*
    else if (!index.parent().parent().isValid()){

    }*/
    return QSize(0,0);
}

/*
QWidget *StoryboardDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option ,
    const QModelIndex &index) const
{
    //QLineEdit *editor = new QLineEdit(parent);
    //return editor;
}

bool StoryboardDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QStyleOptionViewItem newOption = option;

    if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        && (index.flags() & Qt::ItemIsEnabled))
    {
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