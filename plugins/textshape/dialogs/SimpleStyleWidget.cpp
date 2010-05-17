/* This file is part of the KDE project
 * Copyright (C) 2007, 2008, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Casper Boemann <cbo@boemann.dk>
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
#include "SimpleStyleWidget.h"
#include "TextTool.h"
#include "../ListItemsHelper.h"
#include "../commands/ChangeListCommand.h"

#include <KAction>
#include <KoTextBlockData.h>
#include <KoParagraphStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocumentLayout.h>
#include <KoZoomHandler.h>

#include <KDebug>

#include <QTextLayout>

SimpleStyleWidget::SimpleStyleWidget(TextTool *tool, QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_comboboxHasBidiItems(false),
        m_tool(tool),
        m_directionButtonState(Auto),
        m_quickApplyListStyle(KoListStyle::DiscItem)
{
    widget.setupUi(this);
    widget.bold->setDefaultAction(tool->action("format_bold"));
    widget.italic->setDefaultAction(tool->action("format_italic"));
    widget.strikeOut->setDefaultAction(tool->action("format_strike"));
    widget.underline->setDefaultAction(tool->action("format_underline"));
    // RTL layout will reverse the button order, but the align left/right then get mixed up.
    // this makes sure that whatever happens the 'align left' is to the left of the 'align right'
    if (QApplication::isRightToLeft()) {
        widget.alignLeft->setDefaultAction(tool->action("format_alignright"));
        widget.alignRight->setDefaultAction(tool->action("format_alignleft"));
    } else {
        widget.alignLeft->setDefaultAction(tool->action("format_alignleft"));
        widget.alignRight->setDefaultAction(tool->action("format_alignright"));
    }

    widget.textColor->setDefaultAction(tool->action("format_textcolor"));
    widget.backgroundColor->setDefaultAction(tool->action("format_backgroundcolor"));
    widget.alignCenter->setDefaultAction(tool->action("format_aligncenter"));
    widget.alignBlock->setDefaultAction(tool->action("format_alignblock"));
    widget.superscript->setDefaultAction(tool->action("format_super"));
    widget.subscript->setDefaultAction(tool->action("format_sub"));
    widget.decreaseIndent->setDefaultAction(tool->action("format_decreaseindent"));
    widget.increaseIndent->setDefaultAction(tool->action("format_increaseindent"));

    QComboBox *family = qobject_cast<QComboBox*> (tool->action("format_fontfamily")->requestWidget(this));
    if (family) { // kdelibs 4.1 didn't return anything here.
        widget.fontsFrame->addWidget(family);
        connect(family, SIGNAL(activated(int)), this, SIGNAL(doneWithFocus()));
    }
    QComboBox *size = qobject_cast<QComboBox*> (tool->action("format_fontsize")->requestWidget(this));
    if (size) { // kdelibs 4.1 didn't return anything here.
        widget.fontsFrame->addWidget(size);
        connect(size, SIGNAL(activated(int)), this, SIGNAL(doneWithFocus()));
    }

    fillListsCombobox();

    connect(widget.listType, SIGNAL(activated(int)), this, SLOT(listStyleChanged(int)));
    connect(widget.reversedText, SIGNAL(clicked()), this, SLOT(directionChangeRequested()));
    connect(widget.listStyleAgain, SIGNAL(clicked()), this, SLOT(applyAgainPressed()));
}

void SimpleStyleWidget::fillListsCombobox()
{
    if (widget.listType->count() > 0 && (m_comboboxHasBidiItems || !m_tool->isBidiDocument()))
        return;

    widget.listType->clear();
    KoZoomHandler zoomHandler;
    zoomHandler.setZoomAndResolution(160, 72, 72);

    KoInlineTextObjectManager itom;
    TextShape textShape(&itom);
    textShape.setSize(QSizeF(300, 100));
    KoTextDocumentLayout *layouter = qobject_cast<KoTextDocumentLayout*> (textShape.textShapeData()->document()->documentLayout());
    Q_ASSERT(layouter);
    foreach(const Lists::ListStyleItem &item, Lists::genericListStyleItems()) {
        if (item.style == KoListStyle::None) {
            widget.listType->addItem(item.name, static_cast<int>(item.style));
            continue;
        }
        QPixmap pixmap(16, 16); // can we get the actual size from the style?
        pixmap.fill(Qt::transparent);
        QPainter p(&pixmap);
        KoListStyle listStyle;
        KoListLevelProperties llp = listStyle.levelProperties(1);
        llp.setStyle(item.style);
        if (KoListStyle::isNumberingStyle(item.style)) {
            llp.setStartValue(1);
            llp.setListItemSuffix(".");
        } else {
            p.setRenderHint(QPainter::Antialiasing);
        }
        listStyle.setLevelProperties(llp);
        listStyle.applyStyle(textShape.textShapeData()->document()->begin(),1);
        layouter->layout();
        textShape.paintComponent(p, zoomHandler);
        p.end();

        widget.listType->addItem(QIcon(pixmap), item.name, static_cast<int>(item.style));
        if (item.style == m_quickApplyListStyle) {
            widget.listStyleAgain->setIcon(QIcon(pixmap));
        }
    }
    if (m_tool->isBidiDocument()) {
        foreach(const Lists::ListStyleItem &item, Lists::otherListStyleItems())
            widget.listType->addItem(item.name, static_cast<int>(item.style));
        m_comboboxHasBidiItems = true;
    }
}

void SimpleStyleWidget::setCurrentBlock(const QTextBlock &block)
{
    m_currentBlock = block;
    m_blockSignals = true;
    struct Finally {
        Finally(SimpleStyleWidget *p) {
            parent = p;
        }
        ~Finally() {
            parent->m_blockSignals = false;
        }
        SimpleStyleWidget *parent;
    };
    Finally finally(this);

    widget.reversedText->setVisible(m_tool->isBidiDocument());
    QTextLayout *layout = block.layout();
    if (layout) {
        switch (layout->textOption().textDirection()) {
        case Qt::LeftToRight: updateDirection(LTR); break;
        case Qt::RightToLeft: updateDirection(RTL); break;
        }
    }

    //  rest of function is lists stuff. Don't add anything else down here.
    fillListsCombobox();

    QTextList *list = block.textList();
    if (list == 0) {
        widget.listType->setCurrentIndex(0); // the item 'NONE'
        return;
    }

    // TODO get style override from the bf and use that for the QTextListFormat
    //QTextBlockFormat bf = block.format();
    //bf.intProperty(KoListStyle::StyleOverride));

    QTextListFormat format = list->format();
    int style = format.intProperty(QTextListFormat::ListStyle);
    for (int i = 0; i < widget.listType->count(); i++) {
        if (widget.listType->itemData(i).toInt() == style) {
            widget.listType->setCurrentIndex(i);
            return;
        }
    }

    foreach(const Lists::ListStyleItem & item, Lists::otherListStyleItems()) {
        if (item.style == style) {
            widget.listType->addItem(item.name, static_cast<int>(item.style));
            widget.listType->setCurrentIndex(widget.listType->count() - 1);
            return;
        }
    }
}

void SimpleStyleWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void SimpleStyleWidget::setCurrentFormat(const QTextCharFormat& format)
{
    Q_UNUSED(format);
}

void SimpleStyleWidget::listStyleChanged(int row)
{
    if (m_blockSignals) return;

    KoListStyle::Style newStyle = static_cast<KoListStyle::Style>(widget.listType->itemData(row).toInt());
    m_tool->addCommand(new ChangeListCommand(m_tool->cursor(), newStyle, 0 /* level*/));
    if (m_quickApplyListStyle != newStyle && newStyle != KoListStyle::None) {
        m_quickApplyListStyle = newStyle;
        widget.listStyleAgain->setIcon(widget.listType->itemIcon(row));
    }
    emit doneWithFocus();
}

void SimpleStyleWidget::directionChangeRequested()
{
    QTextCursor cursor = m_tool->cursor();
    QTextBlockFormat format;
    KoText::Direction dir = static_cast<KoText::Direction>(m_currentBlock.blockFormat()
            .intProperty(KoParagraphStyle::TextProgressionDirection));
    switch (dir) {
    case KoText::PerhapsLeftRightTopBottom:
    case KoText::LeftRightTopBottom:
        format.setProperty(KoParagraphStyle::TextProgressionDirection, KoText::RightLeftTopBottom);
        updateDirection(RTL);
        break;
    case KoText::InheritDirection:
    case KoText::AutoDirection:
        updateDirection(LTR);
        format.setProperty(KoParagraphStyle::TextProgressionDirection, KoText::LeftRightTopBottom);
        break;
    case KoText::PerhapsRightLeftTopBottom:
    case KoText::RightLeftTopBottom: {
        updateDirection(Auto);
        // clearProperty won't have any effect on merge below.
        int start = qMin(cursor.position(), cursor.anchor());
        int end = qMax(cursor.position(), cursor.anchor());
        cursor.setPosition(start);
        while (cursor.position() <= end) {
            QTextBlockFormat bf = cursor.blockFormat();
            bf.clearProperty(KoParagraphStyle::TextProgressionDirection);
            cursor.setBlockFormat(bf);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                break;
        }
        emit doneWithFocus();
        return;
    }
    case KoText::TopBottomRightLeft: ;// Unhandled.
        break;
    };
    cursor.mergeBlockFormat(format);
    emit doneWithFocus();
}

void SimpleStyleWidget::updateDirection(DirectionButtonState state)
{
    if (m_directionButtonState == state) return;
    m_directionButtonState = state;
    QString buttonText;
    switch (state) {
    case LTR:
        buttonText = i18nc("Short for LeftToRight", "LTR");
        break;
    case RTL:
        buttonText = i18nc("Short for RightToLeft", "RTL");
        break;
    default:
    case Auto:
        buttonText = i18nc("Automatic direction detection", "Auto");
        break;
    }
    widget.reversedText->setText(buttonText);
}

void SimpleStyleWidget::applyAgainPressed()
{
    // change combobox
    for (int row=0; row < widget.listType->count(); ++row) {
        if (widget.listType->itemData(row).toInt() == m_quickApplyListStyle) {
            widget.listType->setCurrentIndex(row);
            listStyleChanged(row);
            break;
        }
    }
}

#include <SimpleStyleWidget.moc>
