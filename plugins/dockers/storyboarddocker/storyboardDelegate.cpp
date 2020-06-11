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
#include <QTextEdit>
#include <QDebug>
#include <QStyle>
#include <QPainter>
#include <QApplication>
#include <QSize>
#include <QMouseEvent>
#include <QListView>
#include <QSpinBox>

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
            p->drawRect(parentRect);

            parentRect.setTopLeft(parentRect.topLeft() + QPoint(4, 4));
            parentRect.setBottomRight(parentRect.bottomRight() - QPoint(4, 4));
            //TODO: change highlight color and the area that is highlighted
            if (option.state & QStyle::State_Selected){
                p->fillRect(option.rect, option.palette.foreground());
            }
            else {
                p->fillRect(option.rect, option.palette.background());
            }
            p->eraseRect(parentRect);
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
                    p->setPen(QPen(option.palette.dark(), 2));
                    p->drawRect(frameNumRect);
                    p->setPen(QPen(option.palette.text(), 1));
                    p->drawText(frameNumRect, Qt::AlignHCenter | Qt::AlignVCenter, data);

                    QIcon icon = KisIconUtils::loadIcon("krita-base");
                    icon.paint(p, option.rect);
                    p->setPen(QPen(option.palette.dark(), 2));
                    p->drawRect(option.rect);

                    if (option.state & QStyle::State_MouseOver){
                        QRect buttonsRect = option.rect;
                        buttonsRect.setTop(option.rect.bottom() - 22);
                        p->fillRect(buttonsRect, option.palette.background());

                        buttonsRect.setWidth(22);
                        buttonsRect.moveBottomLeft(option.rect.bottomLeft());
                        QIcon addIcon = KisIconUtils::loadIcon("list-add");
                        addIcon.paint(p, buttonsRect);

                        buttonsRect.moveBottomRight(option.rect.bottomRight());
                        QIcon deleteIcon = KisIconUtils::loadIcon("trash-empty");
                        deleteIcon.paint(p, buttonsRect);
                    }
                    break;
                }
                case 1:
                {
                    QRect itemNameRect = option.rect;
                    itemNameRect.setLeft(option.rect.left() + 5);
                    p->setPen(QPen(option.palette.text(), 1));
                    p->drawText(itemNameRect, Qt::AlignLeft | Qt::AlignVCenter, data);
                    p->setPen(QPen(option.palette.dark(), 2));
                    p->drawRect(option.rect);
                    break;
                }
                case 2:    //time duration
                case 3:    //frame duration
                {
                    drawSpinBox(p, option, data);
                    break;
                }
                default:
                {
                    const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
                    if (model->getComment(index.row() - 4).visibility){
                        p->setPen(QPen(option.palette.dark(), 2));
                        drawComment(p, option, data, index);
                    }
                    break;
                }
            }
        }
    }
    p->restore();
}

void StoryboardDelegate::drawSpinBox(QPainter *p, const QStyleOptionViewItem &option, QString data) const
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QStyleOptionSpinBox spinBoxOption;
    spinBoxOption.stepEnabled = QAbstractSpinBox::StepDownEnabled | QAbstractSpinBox::StepUpEnabled;
    spinBoxOption.subControls = QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown;
    spinBoxOption.rect = option.rect;
    p->setPen(QPen(option.palette.dark(), 2));
    p->drawRect(option.rect);
    style->drawComplexControl(QStyle::CC_SpinBox, &spinBoxOption, p, option.widget);

    QRect rect = style->subControlRect(QStyle::CC_SpinBox, &spinBoxOption,
                    QStyle::QStyle::SC_SpinBoxEditField);
    rect.moveTopLeft(option.rect.topLeft());
    p->setPen(QPen(option.palette.text(), 1));
    p->drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, data);
}

void StoryboardDelegate::drawComment(QPainter *p, const QStyleOptionViewItem &option, QString data, const QModelIndex &index) const
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());

    QRect titleRect = option.rect;
    titleRect.setHeight(option.fontMetrics.height() + 3);
    p->setPen(QPen(option.palette.text(), 1));
    p->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, model->getComment(index.row() - 4).name);

    QRect contentRect = option.rect;
    contentRect.setTop(option.rect.top() + option.fontMetrics.height() + 3);
    p->setPen(QPen(option.palette.dark(), 2));
    p->drawRect(contentRect);
    contentRect.setTopLeft(contentRect.topLeft() + QPoint(5, 5));
    contentRect.setBottomRight(contentRect.bottomRight() - QPoint(5, 5));
    p->setPen(QPen(option.palette.text(), 1));
    p->drawText(contentRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, data);
}

QSize StoryboardDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.parent().isValid()){
        int width = option.widget->width() - 17;
        const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
        int numComments = model->visibleCommentCount();
        int numItem = width/250;
        if(numItem <=0){
            return QSize(0, 0);
        }
        return QSize(width / numItem, 120 + option.fontMetrics.height() + 3 + numComments*100 + 10);
    }
    else {
        return option.rect.size();
    }
    return QSize(0,0);
}


QWidget *StoryboardDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &option ,
    const QModelIndex &index) const
{
    //only create editor for children
    if (index.parent().isValid()){
        int row = index.row();
        switch (row)
        {
            case 0:             //frame thumbnail is uneditable
            return nullptr;
            case 1:
            {
                QLineEdit *editor = new QLineEdit(parent);
                return editor;
            }
            case 2:            //second and frame spin box
            case 3:
            {
                QSpinBox *spinbox = new QSpinBox(parent);
                spinbox->setRange(0, 999);
                return spinbox;
            }
            default:             // for itemName and comments
            {
                QTextEdit *editor = new QTextEdit(parent);
                return editor;
            }
        }
    }
    return nullptr;
}

bool StoryboardDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if ((event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonDblClick)
        && (index.flags() & Qt::ItemIsEnabled))
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        //handle the duration edit event
        if (index.parent().isValid() && (index.row() == 2 || index.row() == 3)){
            QRect upButton = spinBoxUpButton(option);
            QRect downButton = spinBoxDownButton(option);

            bool upButtonClicked = upButton.isValid() && upButton.contains(mouseEvent->pos());
            bool downButtonClicked = downButton.isValid() && downButton.contains(mouseEvent->pos());
            const bool leftButton = mouseEvent->buttons() & Qt::LeftButton;

            if (leftButton && upButtonClicked){
                model->setData(index, index.data().toInt() + 1);
                return true;
            }
            else if (leftButton && downButtonClicked){
                model->setData(index, std::max(0,index.data().toInt() - 1));
                return true;
            }
        }
    }
    return false;
}

//set the existing data in the editor
void StoryboardDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QVariant value = index.data();
    if (index.parent().isValid()){
        int row = index.row();
        switch (row)
        {
            case 0:             //frame thumbnail is uneditable
                return;
            case 1:
            {
                QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
                lineEdit->setText(value.toString());
                return;
            }
            case 2:            //second and frame spin box
            case 3:
            {
                QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
                spinbox->setValue(value.toInt());
                return;
            }
            default:             // for itemName and comments
            {
                QTextEdit *textEdit = static_cast<QTextEdit*>(editor);
                textEdit->setText(value.toString());
                return;
            }
        }
    }
}

void StoryboardDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QVariant value = index.data();
    if (index.parent().isValid()){
        int row = index.row();
        switch (row)
        {
            case 0:             //frame thumbnail is uneditable
                return;
            case 1:              // for itemName
            {
                QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
                QString value = lineEdit->text();
                model->setData(index, value, Qt::EditRole);
                return;
            }
            case 2:            //second and frame spin box
            case 3:
            {
                QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
                int value = spinbox->value();
                model->setData(index, value, Qt::EditRole);
                return;
            }
            default:             // for comments
            {
                QTextEdit *textEdit = static_cast<QTextEdit*>(editor);
                QString value = textEdit->toPlainText();
                model->setData(index, value, Qt::EditRole);
                return;
            }
        }
    }
}

void StoryboardDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.row() < 4){
        editor->setGeometry(option.rect);
    }
    else {                                                //for comment textedits
        QRect commentRect = option.rect;
        commentRect.setTop(option.rect.top() + option.fontMetrics.height() + 3);
        editor->setGeometry(commentRect);
    }
}

void StoryboardDelegate::setView(QListView *view)
{
    m_view = view;
}

QRect StoryboardDelegate::spinBoxUpButton(const QStyleOptionViewItem &option)
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QStyleOptionSpinBox spinOption;
    spinOption.rect = option.rect;
    QRect rect = style->subControlRect(QStyle::CC_SpinBox, &spinOption,
                    QStyle::QStyle::SC_SpinBoxUp);
    rect.moveTopRight(option.rect.topRight());
    return rect;
}

QRect StoryboardDelegate::spinBoxDownButton(const QStyleOptionViewItem &option)
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QStyleOptionSpinBox spinOption;
    spinOption.rect = option.rect;
    QRect rect = style->subControlRect(QStyle::CC_SpinBox, &spinOption,
                    QStyle::QStyle::SC_SpinBoxDown);
    rect.moveBottomRight(option.rect.bottomRight());
    return rect;
}

QRect StoryboardDelegate::spinBoxEditField(const QStyleOptionViewItem &option)
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QStyleOptionSpinBox spinOption;
    spinOption.rect = option.rect;
    QRect rect = style->subControlRect(QStyle::CC_SpinBox, &spinOption,
                    QStyle::QStyle::SC_SpinBoxEditField);
    rect.moveTopLeft(option.rect.topLeft());
    return rect;
}