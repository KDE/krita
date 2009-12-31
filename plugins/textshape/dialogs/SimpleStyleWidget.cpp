/* This file is part of the KDE project
 * Copyright (C) 2007, 2008 Thomas Zander <zander@kde.org>
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
#include "OptionButton.h"
#include "../ListItemsHelper.h"
#include "../commands/ChangeListCommand.h"

#include <KAction>
#include <KoZoomHandler.h>
#include <KoTextBlockData.h>
#include <TextShape.h>
#include <KoParagraphStyle.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>

#include <KDebug>

#include <QTextLayout>
#include <QComboBox>

SimpleStyleWidget::SimpleStyleWidget(TextTool *tool, QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_tool(tool),
        m_directionButtonState(Auto)
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

    widget.bulletListButton->setIconSize(QSize(16,16));
    widget.numberedListButton->setIconSize(QSize(16,16));
    fillListButtons();

    connect(widget.bulletListButton, SIGNAL(itemTriggered(int)), this, SLOT(listStyleChanged(int)));
    connect(widget.numberedListButton, SIGNAL(itemTriggered(int)), this, SLOT(listStyleChanged(int)));
    connect(widget.reversedText, SIGNAL(clicked()), this, SLOT(directionChangeRequested()));
}

void SimpleStyleWidget::fillListButtons() {
    // we would maybe want to pass a language to this, to include localized list counting strategies.

    KoZoomHandler zoomHandler;
    zoomHandler.setZoomAndResolution(55,100,100);

    KoInlineTextObjectManager itom;
    TextShape textShape(&itom);
    textShape.setSize(QSizeF(300, 100));
    QTextCursor cursor (textShape.textShapeData()->document());
    foreach(Lists::ListStyleItem item, Lists::genericListStyleItems()) {
        QPixmap pm(16,16);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.translate(0, -1.5);
        p.setRenderHint(QPainter::Antialiasing);
        if(item.style != KoListStyle::None) {
            KoListStyle listStyle;
            KoListLevelProperties llp = listStyle.levelProperties(1);
            llp.setStyle(item.style);
            if (KoListStyle::isNumberingStyle(item.style)) {
                llp.setStartValue(1);
                llp.setListItemSuffix(".");
            }
            listStyle.setLevelProperties(llp);
            cursor.select(QTextCursor::Document);
            cursor.insertText("--");
            listStyle.applyStyle(cursor.block(),1);
            cursor.insertText("\n--");
            cursor.insertText("\n--");
            dynamic_cast<KoTextDocumentLayout*> (textShape.textShapeData()->document()->documentLayout())->layout();

            textShape.paintComponent(p, zoomHandler);
            if(listStyle.isNumberingStyle(item.style)) {
                widget.numberedListButton->addItem(pm, static_cast<int> (item.style));
            }
            else {
                widget.bulletListButton->addItem(pm, static_cast<int> (item.style));
            }
        }
        p.end();
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
}

void SimpleStyleWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void SimpleStyleWidget::setCurrentFormat(const QTextCharFormat& format)
{
    Q_UNUSED(format);
}

void SimpleStyleWidget::listStyleChanged(int id)
{
    if (m_blockSignals) return;

    m_tool->addCommand( new ChangeListCommand (m_tool->cursor(), static_cast<KoListStyle::Style> (id)));
    emit doneWithFocus();
}

void SimpleStyleWidget::directionChangeRequested()
{
    QTextCursor cursor(m_currentBlock);
    QTextBlockFormat format = cursor.blockFormat();
    KoText::Direction dir = static_cast<KoText::Direction>(format.intProperty(
                                KoParagraphStyle::TextProgressionDirection));
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
    case KoText::RightLeftTopBottom:
        updateDirection(Auto);
        format.clearProperty(KoParagraphStyle::TextProgressionDirection);
        break;
    case KoText::TopBottomRightLeft: ;// Unhandled.
        break;
    };
    cursor.setBlockFormat(format);
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

#include <SimpleStyleWidget.moc>
