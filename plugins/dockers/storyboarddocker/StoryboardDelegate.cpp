/*
 *  SPDX-FileCopyrightText: 2020 Saurabh Kumar <saurabhk660@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "StoryboardDelegate.h"
#include "StoryboardModel.h"

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
#include <kis_image_animation_interface.h>

StoryboardDelegate::StoryboardDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , m_imageSize(QSize())
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
        if (!index.isValid()) {
            p->restore();
            return;
        }
        if (!index.parent().isValid()) {
            QRect parentRect = option.rect;
            p->setPen(QPen(option.palette.background(), 2));
            p->drawRect(parentRect);

            parentRect.setTopLeft(parentRect.topLeft() + QPoint(4, 4));
            parentRect.setBottomRight(parentRect.bottomRight() - QPoint(4, 4));

            if (option.state & QStyle::State_Selected) {
                p->fillRect(option.rect, option.palette.highlight());
            }
            else {
                p->fillRect(option.rect, option.palette.window());
            }
            p->eraseRect(parentRect);
        }
        else {
            //draw the child items
            int childNum = index.row();
            QString data = index.data().toString();

            switch (childNum)
            {
            case StoryboardItem::FrameNumber:
            {
                if (m_view->thumbnailIsVisible()) {
                    QRect frameNumRect = option.rect;
                    frameNumRect.setHeight(m_view->fontMetrics().height()+3);
#if QT_VERSION >= QT_VERSION_CHECK(5,11,0)
                    frameNumRect.setWidth(3 * m_view->fontMetrics().horizontalAdvance("0") + 2);
#else
                    frameNumRect.setWidth(3 * m_view->fontMetrics().width('0') + 2);
#endif
                    frameNumRect.moveBottom(option.rect.top()-1);
                    p->setPen(QPen(option.palette.dark(), 2));
                    p->drawRect(frameNumRect);
                    p->setPen(QPen(option.palette.text(), 1));
                    p->drawText(frameNumRect, Qt::AlignHCenter | Qt::AlignVCenter, data);

                    if (!m_imageSize.isEmpty()) {
                        float scale = qMin(option.rect.height() / (float)m_imageSize.height(), (float)option.rect.width() / m_imageSize.width());
                        QRect thumbnailRect = option.rect;
                        thumbnailRect.setSize(m_imageSize * scale);
                        thumbnailRect.moveCenter(option.rect.center());

                        QPixmap  thumbnailPixmap= index.data(Qt::UserRole).value<QPixmap>();
                        p->drawPixmap(thumbnailRect, thumbnailPixmap);
                    }
                    p->setPen(QPen(option.palette.dark(), 2));
                    p->drawRect(option.rect);

                    QRect buttonsRect = option.rect;
                    buttonsRect.setTop(option.rect.bottom() - 22);

                    buttonsRect.setWidth(22);
                    buttonsRect.moveBottomLeft(option.rect.bottomLeft());
                    QIcon addIcon = KisIconUtils::loadIcon("list-add");
                    p->fillRect(buttonsRect, option.palette.window());
                    addIcon.paint(p, buttonsRect);

                    buttonsRect.moveBottomRight(option.rect.bottomRight());
                    QIcon deleteIcon = KisIconUtils::loadIcon("edit-delete");
                    p->fillRect(buttonsRect, option.palette.window());
                    deleteIcon.paint(p, buttonsRect);
                }
                else {
                    QRect frameNumRect = option.rect;
                    p->setPen(QPen(option.palette.dark(), 2));
                    p->drawRect(frameNumRect);
                    p->setPen(QPen(option.palette.text(), 1));
                    p->drawText(frameNumRect, Qt::AlignHCenter | Qt::AlignVCenter, data);
                }
                break;
            }
            case StoryboardItem::ItemName:
            {
                QRect itemNameRect = option.rect;
                itemNameRect.setLeft(option.rect.left() + 5);
                p->setPen(QPen(option.palette.text(), 1));
                p->drawText(itemNameRect, Qt::AlignLeft | Qt::AlignVCenter, data);
                p->setPen(QPen(option.palette.dark(), 2));
                p->drawRect(option.rect);
                break;
            }
            case StoryboardItem::DurationSecond:
            {
                drawSpinBox(p, option, data, i18nc("suffix in spin box in storyboard that means 'seconds'", "s"));
                break;
            }
            case StoryboardItem::DurationFrame:
            {
                drawSpinBox(p, option, data, i18nc("suffix in spin box in storyboard that means 'frames'", "f"));
                break;
            }
            default:
            {
                const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
                if (m_view->commentIsVisible() && model->getComment(index.row() - 4).visibility) {
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

void StoryboardDelegate::drawSpinBox(QPainter *p, const QStyleOptionViewItem &option, QString data, QString suffix) const
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
    p->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, data + suffix);
}

QStyleOptionSlider StoryboardDelegate::drawComment(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
    QString data = index.data().toString();

    QRect titleRect = option.rect;
    titleRect.setHeight(option.fontMetrics.height() + 3);
    if (p) {
        p->setPen(QPen(option.palette.text(), 1));
        p->drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, model->getComment(index.row() - 4).name);
        p->setPen(QPen(option.palette.dark(), 2));
        p->drawRect(titleRect);
    }

    QRect contentRect = option.rect;
    contentRect.setTop(option.rect.top() + option.fontMetrics.height() + 3);
    if (p) {
        p->setPen(QPen(option.palette.dark(), 2));
        p->drawRect(contentRect);
        p->save();
    }
    contentRect.setTopLeft(contentRect.topLeft() + QPoint(5, 5));
    contentRect.setBottomRight(contentRect.bottomRight() - QPoint(5, 5));

    int scrollValue = index.data(Qt::UserRole).toInt();

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
    if (p) {
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
    scrollRect.setSize(QSize(15, option.rect.height() - option.fontMetrics.height() - 3));
    scrollRect.moveTopLeft(QPoint(0, 0));
    scrollbarOption.rect = scrollRect;

    if (p && scrollbarOption.pageStep <= doc.size().height()) {
        p->save();
        p->setPen(QPen(option.palette.dark(), 2));
        p->translate(QPoint( option.rect.right()-15, option.rect.top() + option.fontMetrics.height() + 3));
        style->drawComplexControl(QStyle::CC_ScrollBar, &scrollbarOption, p, option.widget);
        p->restore();
    }
    return scrollbarOption;
}

QSize StoryboardDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    if (!index.parent().isValid()) {
        if (m_view->itemOrientation() == Qt::Vertical) {
            int width = option.widget->width() - 17;
            const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
            int numComments = model->visibleCommentCount();
            int numItem = width/250;
            if (numItem <= 0) {
                numItem = 1;
            }

            int thumbnailheight = m_view->thumbnailIsVisible() ? 120 : 0;
            int commentHeight = m_view->commentIsVisible() ? numComments*100 : 0;
            return QSize(width / numItem, thumbnailheight  + option.fontMetrics.height() + 3 + commentHeight + 10);
        }
        else {
            const StoryboardModel* model = dynamic_cast<const StoryboardModel*>(index.model());
            int numComments = model->visibleCommentCount();
            int commentWidth = 0;
            if (numComments && m_view->commentIsVisible()) {
                commentWidth = qMax(200, (m_view->viewport()->width() - 250) / numComments);
            }
            int width = 250 + numComments * commentWidth;
            return QSize(width + 10, 120 + option.fontMetrics.height() + 3 + 10);
        }
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
    Q_UNUSED(option);
    //only create editor for children
    if (index.parent().isValid()) {
        int row = index.row();
        switch (row)
        {
        case StoryboardItem::FrameNumber:
            return nullptr;
        case StoryboardItem::ItemName:
        {
            QLineEdit *editor = new QLineEdit(parent);
            return editor;
        }
        case StoryboardItem::DurationSecond:
        {
            QSpinBox *spinbox = new QSpinBox(parent);
            spinbox->setRange(0, 999);
            spinbox->setSuffix(i18nc("suffix in spin box in storyboard that means 'seconds'", "s"));
            return spinbox;
        }
        case StoryboardItem::DurationFrame:
        {
            QSpinBox *spinbox = new QSpinBox(parent);
            spinbox->setRange(0, 99);
            spinbox->setSuffix(i18nc("suffix in spin box in storyboard that means 'frames'", "f"));
            return spinbox;
        }
        default:              //for comments
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
        if (index.parent().isValid() && (index.row() == StoryboardItem::DurationSecond || index.row() == StoryboardItem::DurationFrame)) {
            QRect upButton = spinBoxUpButton(option);
            QRect downButton = spinBoxDownButton(option);

            bool upButtonClicked = upButton.isValid() && upButton.contains(mouseEvent->pos());
            bool downButtonClicked = downButton.isValid() && downButton.contains(mouseEvent->pos());

            StoryboardModel* sbModel = dynamic_cast<StoryboardModel*>(model);
            if (leftButton && upButtonClicked) {
                sbModel->setData(index, index.data().toInt() + 1);
                //                return sbModel->insertHoldFramesAfter(index.data().toInt() + 1, index.data().toInt(), index);
                return true;
            }
            else if (leftButton && downButtonClicked) {
                sbModel->setData(index, index.data().toInt() - 1);
                //                return sbModel->insertHoldFramesAfter(std::max(-1, index.data().toInt() - 1), index.data().toInt(), index);
                return true;
            }
        }
        else if (index.parent().isValid() && index.row() >= StoryboardItem::Comments) {
            QStyleOptionSlider scrollBarOption = drawComment(nullptr, option, index);
            QRect upButton = scrollUpButton(option, scrollBarOption);
            QRect downButton = scrollDownButton(option, scrollBarOption);

            bool upButtonClicked = upButton.isValid() && upButton.contains(mouseEvent->pos());
            bool downButtonClicked = downButton.isValid() && downButton.contains(mouseEvent->pos());

            if (leftButton && upButtonClicked) {
                int lastValue = model->data(index, Qt::UserRole).toInt();
                int value = lastValue - option.fontMetrics.height();
                StoryboardModel* modelSB = dynamic_cast<StoryboardModel*>(model);
                modelSB->setCommentScrollData(index, qMax(0, value));
                return true;
            }
            else if (leftButton && downButtonClicked) {
                int lastValue = model->data(index, Qt::UserRole).toInt();
                int value = lastValue + option.fontMetrics.height();
                StoryboardModel* modelSB = dynamic_cast<StoryboardModel*>(model);
                modelSB->setCommentScrollData(index, qMin(scrollBarOption.maximum, value));
                return true;
            }
        }

        else if (index.parent().isValid() && index.row() == StoryboardItem::FrameNumber && m_view->thumbnailIsVisible()) {     //thumbnail add/delete events
            QRect addItemButton(QPoint(0, 0), QSize(22, 22));
            addItemButton.moveBottomLeft(option.rect.bottomLeft());

            QRect deleteItemButton(QPoint(0, 0), QSize(22, 22));
            deleteItemButton.moveBottomRight(option.rect.bottomRight());

            bool addItemButtonClicked = addItemButton.isValid() && addItemButton.contains(mouseEvent->pos());
            bool deleteItemButtonClicked = deleteItemButton.isValid() && deleteItemButton.contains(mouseEvent->pos());


            StoryboardModel* sbModel = dynamic_cast<StoryboardModel*>(model);
            if (leftButton && addItemButtonClicked) {
                sbModel->insertItem(index.parent(), true);
                return true;
            }
            else if (leftButton && deleteItemButtonClicked) {
                model->removeRows(index.parent().row(), 1);
                return true;
            }
        }
    }

    if ((event->type() == QEvent::MouseMove) && (index.flags() & Qt::ItemIsEnabled)) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        const bool leftButton = mouseEvent->buttons() & Qt::LeftButton;

        QStyleOptionSlider scrollBarOption = drawComment(nullptr, option, index);
        QRect scrollBarRect = scrollBar(option, scrollBarOption);

        bool lastClickPosInScroll = scrollBarRect.isValid() && scrollBarRect.contains(m_lastDragPos);
        bool currClickPosInScroll = scrollBarRect.isValid() && scrollBarRect.contains(mouseEvent->pos());

        if (leftButton && index.parent().isValid() && index.row() >= StoryboardItem::Comments) {
            if (lastClickPosInScroll && currClickPosInScroll) {
                int lastValue = model->data(index, Qt::UserRole).toInt();
                int value = lastValue + mouseEvent->pos().y() - m_lastDragPos.y();
                StoryboardModel* modelSB = dynamic_cast<StoryboardModel*>(model);
                if (value >= 0 && value <= scrollBarOption.maximum) {
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
    if (index.parent().isValid()) {
        int row = index.row();
        switch (row)
        {
        case StoryboardItem::FrameNumber:             //frame thumbnail is uneditable
            return;
        case StoryboardItem::ItemName:
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            lineEdit->setText(value.toString());
            return;
        }
        case StoryboardItem::DurationSecond:
        case StoryboardItem::DurationFrame:
        {
            QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
            spinbox->setValue(value.toInt());
            return;
        }
        default:             // for comments
        {
            QTextEdit *textEdit = static_cast<QTextEdit*>(editor);
            textEdit->setText(value.toString());
            textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
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
    if (index.parent().isValid()) {
        int row = index.row();
        switch (row)
        {
        case StoryboardItem::FrameNumber:             //frame thumbnail is uneditable
            return;
        case StoryboardItem::ItemName:
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            QString value = lineEdit->text();
            model->setData(index, value, Qt::EditRole);
            return;
        }
        case StoryboardItem::DurationSecond:
        case StoryboardItem::DurationFrame:
        {
            QSpinBox *spinbox = static_cast<QSpinBox*>(editor);
            int value = spinbox->value();

            StoryboardModel* sbModel = dynamic_cast<StoryboardModel*>(model);
            sbModel->setData(index, value);
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
    if (index.row() < StoryboardItem::Comments) {
        editor->setGeometry(option.rect);
    }
    else {                                                //for comment textedits
        QRect commentRect = option.rect;
        commentRect.setTop(option.rect.top() + option.fontMetrics.height() + 3);
        editor->setGeometry(commentRect);
    }
}

void StoryboardDelegate::setView(StoryboardView *view)
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
    rect.moveTopLeft(rect.topLeft() + option.rect.bottomRight() - scrollBarOption.rect.bottomRight());
    return rect;
}

QRect StoryboardDelegate::scrollDownButton(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption)
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QRect rect = style->subControlRect(QStyle::CC_ScrollBar, &scrollBarOption,
                                       QStyle::QStyle::SC_ScrollBarAddLine);
    rect.moveTopLeft(rect.topLeft() + scrollBarOption.rect.topLeft());
    rect.moveBottomRight(option.rect.bottomRight());
    return rect;
}

QRect StoryboardDelegate::scrollUpButton(const QStyleOptionViewItem &option, QStyleOptionSlider &scrollBarOption)
{
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    QRect rect = style->subControlRect(QStyle::CC_ScrollBar, &scrollBarOption,
                                       QStyle::QStyle::SC_ScrollBarSubLine);
    rect.moveTopLeft(rect.topLeft() + scrollBarOption.rect.topLeft());
    rect.moveTop(option.rect.bottom() - scrollBarOption.rect.height());
    rect.moveRight(option.rect.right());
    return rect;
}

void StoryboardDelegate::setImageSize(QSize imageSize)
{
    m_imageSize = imageSize;
}
