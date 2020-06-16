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
#include <QScrollBar>

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
                        drawComment(p, option, index);
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

QStyleOptionSlider StoryboardDelegate::drawComment(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
    QString data = index.model()->data(index, Qt::DisplayRole).toString();

    QRect titleRect = option.rect;
    titleRect.setHeight(option.fontMetrics.height() + 3);
    if (p){
        p->setPen(QPen(option.palette.text(), 1));
        p->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, model->getComment(index.row() - 4).name);
    }

    QRect contentRect = option.rect;
    contentRect.setTop(option.rect.top() + option.fontMetrics.height() + 3);
    if (p){
        p->setPen(QPen(option.palette.dark(), 2));
        p->drawRect(contentRect);
        p->save();
    }
    contentRect.setTopLeft(contentRect.topLeft() + QPoint(5, 5));
    contentRect.setBottomRight(contentRect.bottomRight() - QPoint(5, 5));

    int scrollValue = index.model()->data(index, Qt::UserRole).toInt();

    //draw comment
    QRect commentRect = contentRect;
    commentRect.setRight(contentRect.right() - 15);
    QTextDocument doc;

    doc.setTextWidth(commentRect.width());
    doc.setDocumentMargin(0);
    doc.setDefaultFont(option.font);
    doc.setPlainText(data.simplified());
    QRectF clipRect = commentRect;
    clipRect.moveTopLeft(QPoint(0, 0 + scrollValue));
    if (p){
        p->translate(QPoint(commentRect.topLeft().x(), commentRect.topLeft().y() - scrollValue));
        p->setPen(QPen(option.palette.text(), 1));
        doc.drawContents(p, clipRect);
        p->restore();
    }
    //draw scroll bar
    QStyleOptionSlider scrollbarOption;
    scrollbarOption.sliderPosition = scrollValue;
    scrollbarOption.minimum = 0;
    scrollbarOption.maximum = qMax(0.0, doc.size().height() - contentRect.height());
    scrollbarOption.sliderPosition = qMin(scrollValue, scrollbarOption.maximum);
    scrollbarOption.pageStep = contentRect.height() - 2;
    scrollbarOption.orientation = Qt::Vertical;

    QRect scrollRect = option.rect;
    scrollRect.setTop(option.rect.top() + option.fontMetrics.height() + 3);
    scrollRect.setLeft(option.rect.right()-15);
    scrollbarOption.rect = scrollRect;

    if (p){
        p->save();
        p->setPen(QPen(option.palette.dark(), 2));
        p->translate(scrollRect.topLeft());
        p->drawRect(scrollRect);
        style->drawComplexControl(QStyle::CC_ScrollBar, &scrollbarOption, p, option.widget);
        p->restore();
    }
    return scrollbarOption;
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
        const bool leftButton = mouseEvent->buttons() & Qt::LeftButton;

        //handle the duration edit event
        if (index.parent().isValid() && (index.row() == 2 || index.row() == 3)){
            QRect upButton = spinBoxUpButton(option);
            QRect downButton = spinBoxDownButton(option);

            bool upButtonClicked = upButton.isValid() && upButton.contains(mouseEvent->pos());
            bool downButtonClicked = downButton.isValid() && downButton.contains(mouseEvent->pos());

            if (leftButton && upButtonClicked){
                model->setData(index, index.data().toInt() + 1);
                return true;
            }
            else if (leftButton && downButtonClicked){
                model->setData(index, std::max(0,index.data().toInt() - 1));
                return true;
            }
        }
        else if (index.parent().isValid() && index.row() > 3){
            QStyleOptionSlider scrollBarOption = drawComment(nullptr, option, index);
            QRect upButton = scrollUpButton(option, scrollBarOption);
            QRect downButton = scrollDownButton(option, scrollBarOption);

            bool upButtonClicked = upButton.isValid() && upButton.contains(mouseEvent->pos());
            bool downButtonClicked = downButton.isValid() && downButton.contains(mouseEvent->pos());

            if (leftButton && upButtonClicked){
                int lastValue = model->data(index, Qt::UserRole).toInt();
                int value = lastValue - option.fontMetrics.height();
                StoryboardModel* modelSB = dynamic_cast<StoryboardModel*>(model);
                modelSB->setCommentScrollData(index, qMax(0, value));
                return true;
            }
            else if (leftButton && downButtonClicked){
                int lastValue = model->data(index, Qt::UserRole).toInt();
                int value = lastValue + option.fontMetrics.height();
                StoryboardModel* modelSB = dynamic_cast<StoryboardModel*>(model);
                modelSB->setCommentScrollData(index, qMin(scrollBarOption.maximum, value));
                return true;
            }
        }
    }

    if ((event->type() == QEvent::MouseMove) && (index.flags() & Qt::ItemIsEnabled))
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        const bool leftButton = mouseEvent->buttons() & Qt::LeftButton;

        QStyleOptionSlider scrollBarOption = drawComment(nullptr, option, index);
        QRect scrollBarRect = scrollBar(option, scrollBarOption);

        bool lastClickPosInScroll = scrollBarRect.isValid() && scrollBarRect.contains(m_lastDragPos);
        bool currClickPosInScroll = scrollBarRect.isValid() && scrollBarRect.contains(mouseEvent->pos());

        if (leftButton && index.parent().isValid() && index.row() > 3){
            if (lastClickPosInScroll && currClickPosInScroll){
                int lastValue = model->data(index, Qt::UserRole).toInt();
                int value = lastValue + mouseEvent->pos().y() - m_lastDragPos.y();
                StoryboardModel* modelSB = dynamic_cast<StoryboardModel*>(model);
                if (value >= 0 && value <= scrollBarOption.maximum){
                    modelSB->setCommentScrollData(index, value);
                    return true;
                }
                return false;
            }
            m_lastDragPos = mouseEvent->pos();
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
            case 1:             //for itemName
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
            default:             // for comments
            {
                QTextEdit *textEdit = static_cast<QTextEdit*>(editor);
                textEdit->setText(value.toString());
                textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
                textEdit->verticalScrollBar()->setProperty("index", index);
                connect(textEdit->verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(slotCommentScrolledTo(int)));
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

void StoryboardDelegate::slotCommentScrolledTo(int value) const
{
    const QModelIndex index = sender()->property("index").toModelIndex();
    StoryboardModel* model = dynamic_cast<StoryboardModel*>(m_view->model());
    model->setCommentScrollData(index, value);
}

QRect StoryboardDelegate::scrollBar(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption) const
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QRect rect = style->subControlRect(QStyle::CC_ScrollBar, &scrollBarOption,
                    QStyle::QStyle::SC_ScrollBarSlider);
    rect.moveTopLeft(rect.topLeft() + scrollBarOption.rect.topLeft());
    return rect;
}

QRect StoryboardDelegate::scrollDownButton(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption)
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QRect rect = style->subControlRect(QStyle::CC_ScrollBar, &scrollBarOption,
                    QStyle::QStyle::SC_ScrollBarAddLine);
    rect.moveTopLeft(rect.topLeft() + scrollBarOption.rect.topLeft());
    return rect;
}

QRect StoryboardDelegate::scrollUpButton(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption)
{

    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QRect rect = style->subControlRect(QStyle::CC_ScrollBar, &scrollBarOption,
                    QStyle::QStyle::SC_ScrollBarSubLine);
    rect.moveTopLeft(rect.topLeft() + scrollBarOption.rect.topLeft());
    return rect;
}
