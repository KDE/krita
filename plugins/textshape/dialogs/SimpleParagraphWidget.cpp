/* This file is part of the KDE project
 * Copyright (C) 2007, 2008, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009-2010 Casper Boemann <cbo@boemann.dk>
 * Copyright (C) 2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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
#include "SimpleParagraphWidget.h"
#include "TextTool.h"
#include <ListItemsHelper.h>
#include "FormattingButton.h"
#include "StylesWidget.h"
#include "SpecialButton.h"

#include <KAction>
#include <KoTextBlockData.h>
#include <KoParagraphStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocumentLayout.h>
#include <KoZoomHandler.h>
#include <KoStyleThumbnailer.h>
#include <KoStyleManager.h>
#include <KoListLevelProperties.h>
#include <KoShapePaintingContext.h>
#include <KDebug>

#include <QTextLayout>

SimpleParagraphWidget::SimpleParagraphWidget(TextTool *tool, QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false),
        m_tool(tool),
        m_directionButtonState(Auto)
{
    widget.setupUi(this);
    widget.alignCenter->setDefaultAction(tool->action("format_aligncenter"));
    widget.alignBlock->setDefaultAction(tool->action("format_alignblock"));
    // RTL layout will reverse the button order, but the align left/right then get mixed up.
    // this makes sure that whatever happens the 'align left' is to the left of the 'align right'
    if (QApplication::isRightToLeft()) {
        widget.alignLeft->setDefaultAction(tool->action("format_alignright"));
        widget.alignRight->setDefaultAction(tool->action("format_alignleft"));
    } else {
        widget.alignLeft->setDefaultAction(tool->action("format_alignleft"));
        widget.alignRight->setDefaultAction(tool->action("format_alignright"));
    }

    widget.decreaseIndent->setDefaultAction(tool->action("format_decreaseindent"));
    widget.increaseIndent->setDefaultAction(tool->action("format_increaseindent"));
    widget.changeTextDirection->setDefaultAction(tool->action("change_text_direction"));
    connect(widget.changeTextDirection, SIGNAL(clicked()), this, SIGNAL(doneWithFocus()));
    connect(widget.alignCenter, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.alignBlock, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.alignLeft, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.alignRight, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.decreaseIndent, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.increaseIndent, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));

    connect(widget.quickTable, SIGNAL(create(int, int)), this, SIGNAL(insertTableQuick(int, int)));
    connect(widget.quickTable, SIGNAL(create(int, int)), this, SIGNAL(doneWithFocus()));

    widget.bulletListButton->setDefaultAction(tool->action("format_bulletlist"));
    widget.numberedListButton->setDefaultAction(tool->action("format_numberlist"));
    widget.bulletListButton->setNumColumns(3);
    widget.numberedListButton->setNumColumns(3);
 
    fillListButtons();
    widget.bulletListButton->addSeparator();

    connect(widget.bulletListButton, SIGNAL(itemTriggered(int)), this, SLOT(listStyleChanged(int)));
    connect(widget.numberedListButton, SIGNAL(itemTriggered(int)), this, SLOT(listStyleChanged(int)));

    m_stylePopup = new StylesWidget(this, true, Qt::Popup);
    m_stylePopup->setFrameShape(QFrame::StyledPanel);
    m_stylePopup->setFrameShadow(QFrame::Raised);
    widget.blockFrame->setStylesWidget(m_stylePopup);

    connect(m_stylePopup, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)));
    connect(m_stylePopup, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SIGNAL(doneWithFocus()));
    connect(m_stylePopup, SIGNAL(paragraphStyleSelected(KoParagraphStyle *)), this, SLOT(hidePopup()));

    m_thumbnailer = new KoStyleThumbnailer();
}

SimpleParagraphWidget::~SimpleParagraphWidget()
{
    delete m_thumbnailer;
}

void SimpleParagraphWidget::fillListButtons()
{
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom(1.2);
    zoomHandler.setDpi(72, 72);

    KoInlineTextObjectManager itom;
    TextShape textShape(&itom);
    textShape.setSize(QSizeF(300, 100));
    QTextCursor cursor (textShape.textShapeData()->document());
    foreach(Lists::ListStyleItem item, Lists::genericListStyleItems()) {
        QPixmap pm(48,48);

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
            QTextCharFormat textCharFormat=cursor.blockCharFormat();
            textCharFormat.setFontPointSize(11);
            textCharFormat.setFontWeight(QFont::Normal);
            cursor.setCharFormat(textCharFormat);

            cursor.insertText("----");
            listStyle.applyStyle(cursor.block(),1);
            cursor.insertText("\n----");
            cursor.insertText("\n----");

            KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*>(textShape.textShapeData()->document()->documentLayout());
            if(lay)
                lay->layout();

            KoShapePaintingContext paintContext; //FIXME
            textShape.paintComponent(p, zoomHandler, paintContext);
            if(KoListStyle::isNumberingStyle(item.style)) {
                widget.numberedListButton->addItem(pm, static_cast<int> (item.style));
            } else {
                widget.bulletListButton->addItem(pm, static_cast<int> (item.style));
            }
        }
    }
}

void SimpleParagraphWidget::setCurrentBlock(const QTextBlock &block)
{
    m_currentBlock = block;
    m_blockSignals = true;
    struct Finally {
        Finally(SimpleParagraphWidget *p) {
            parent = p;
        }
        ~Finally() {
            parent->m_blockSignals = false;
        }
        SimpleParagraphWidget *parent;
    };
    Finally finally(this);

    QTextLayout *layout = block.layout();
    if (layout) {
        switch(layout->textOption().textDirection())
        {
        case Qt::LeftToRight:
             widget.changeTextDirection->setChecked(false);
             break;
        case Qt::RightToLeft:
            widget.changeTextDirection->setChecked(true);
            break;
        default:
              break;
            }
    }


    QTextBlockFormat format;

    int id = format.intProperty(KoParagraphStyle::StyleId);
    KoParagraphStyle *style(m_styleManager->paragraphStyle(id));
    if (style) {
        widget.blockFrame->setStylePreview(m_thumbnailer->thumbnail(style, widget.blockFrame->size()));
    }
    m_stylePopup->setCurrentFormat(format);
}

void SimpleParagraphWidget::setCurrentFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat)
        return;
    m_currentBlockFormat = format;

    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    KoParagraphStyle *style(m_styleManager->paragraphStyle(id));
    if (style) {
        widget.blockFrame->setStylePreview(m_thumbnailer->thumbnail(m_styleManager->paragraphStyle(id), widget.blockFrame->contentsRect().size()));
    }
    m_stylePopup->setCurrentFormat(format);
}

void SimpleParagraphWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
    m_stylePopup->setStyleManager(sm);
}

void SimpleParagraphWidget::hidePopup()
{
    widget.blockFrame->hidePopup();
}

void SimpleParagraphWidget::listStyleChanged(int id)
{
    emit doneWithFocus();
    if (m_blockSignals) return;
    m_tool->textEditor()->setListProperties(static_cast<KoListStyle::Style> (id));
}

#include <SimpleParagraphWidget.moc>
