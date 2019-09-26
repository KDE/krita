/* This file is part of the KDE project
 * Copyright (C) 2007, 2008, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009-2010 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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
#include "SimpleParagraphWidget.h"
#include "TextTool.h"
#include <ListItemsHelper.h>
#include "FormattingButton.h"
#include <KoStyleThumbnailer.h>

#include "StylesCombo.h"
#include "StylesModel.h"
#include "DockerStylesComboModel.h"
#include "StylesDelegate.h"
#include "ListLevelChooser.h"
#include "commands/ChangeListLevelCommand.h"

#include <KoTextEditor.h>
#include <KoTextBlockData.h>
#include <KoParagraphStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoTextDocumentLayout.h>
#include <KoZoomHandler.h>
#include <KoStyleManager.h>
#include <KoListLevelProperties.h>
#include <KoShapePaintingContext.h>

#include <QAction>

#include <QTextLayout>
#include <QFlags>
#include <QMenu>
#include <QWidgetAction>
#include <KisSignalMapper.h>

#include <QDebug>

SimpleParagraphWidget::SimpleParagraphWidget(TextTool *tool, QWidget *parent)
    : QWidget(parent)
    , m_styleManager(0)
    , m_blockSignals(false)
    , m_tool(tool)
    , m_directionButtonState(Auto)
    , m_thumbnailer(new KoStyleThumbnailer())
    , m_mapper(new KisSignalMapper(this))
    , m_stylesModel(new StylesModel(0, StylesModel::ParagraphStyle))
    , m_sortedStylesModel(new DockerStylesComboModel())
    , m_stylesDelegate(0)
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

    widget.moreOptions->setText("...");
    widget.moreOptions->setToolTip(i18n("Change paragraph format"));
    connect(widget.moreOptions, SIGNAL(clicked(bool)), tool->action("format_paragraph"), SLOT(trigger()));

    connect(widget.changeTextDirection, SIGNAL(clicked()), this, SIGNAL(doneWithFocus()));
    connect(widget.alignCenter, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.alignBlock, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.alignLeft, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.alignRight, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.decreaseIndent, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.increaseIndent, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));

    widget.bulletListButton->setDefaultAction(tool->action("format_bulletlist"));
    widget.bulletListButton->setNumColumns(3);

    fillListButtons();
    widget.bulletListButton->addSeparator();

    connect(widget.bulletListButton, SIGNAL(itemTriggered(int)), this, SLOT(listStyleChanged(int)));

    m_stylesModel->setStyleThumbnailer(m_thumbnailer);
    widget.paragraphStyleCombo->setStylesModel(m_sortedStylesModel);
    connect(widget.paragraphStyleCombo, SIGNAL(selected(QModelIndex)), this, SLOT(styleSelected(QModelIndex)));
    connect(widget.paragraphStyleCombo, SIGNAL(newStyleRequested(QString)), this, SIGNAL(newStyleRequested(QString)));
    connect(widget.paragraphStyleCombo, SIGNAL(newStyleRequested(QString)), this, SIGNAL(doneWithFocus()));
    connect(widget.paragraphStyleCombo, SIGNAL(showStyleManager(int)), this, SLOT(slotShowStyleManager(int)));

    connect(m_mapper, SIGNAL(mapped(int)), this, SLOT(changeListLevel(int)));

    m_sortedStylesModel->setStylesModel(m_stylesModel);
}

SimpleParagraphWidget::~SimpleParagraphWidget()
{
    //the style model is set on the comboBox who takes over ownership
    delete m_thumbnailer;
}

void SimpleParagraphWidget::fillListButtons()
{
    KoZoomHandler zoomHandler;
    zoomHandler.setZoom(1.2);
    zoomHandler.setDpi(72, 72);

    KoInlineTextObjectManager itom;
    KoTextRangeManager tlm;
    TextShape textShape(&itom, &tlm);
    textShape.setSize(QSizeF(300, 100));
    QTextCursor cursor(textShape.textShapeData()->document());
    Q_FOREACH (const Lists::ListStyleItem &item, Lists::genericListStyleItems()) {
        QPixmap pm(48, 48);

        pm.fill(Qt::transparent);
        QPainter p(&pm);

        p.translate(0, -1.5);
        p.setRenderHint(QPainter::Antialiasing);
        if (item.style != KoListStyle::None) {
            KoListStyle listStyle;
            KoListLevelProperties llp = listStyle.levelProperties(1);
            llp.setStyle(item.style);
            if (KoListStyle::isNumberingStyle(item.style)) {
                llp.setStartValue(1);
                llp.setListItemSuffix(".");
            }
            listStyle.setLevelProperties(llp);
            cursor.select(QTextCursor::Document);
            QTextCharFormat textCharFormat = cursor.blockCharFormat();
            textCharFormat.setFontPointSize(11);
            textCharFormat.setFontWeight(QFont::Normal);
            cursor.setCharFormat(textCharFormat);

            QTextBlock cursorBlock = cursor.block();
            KoTextBlockData data(cursorBlock);
            cursor.insertText("----");
            listStyle.applyStyle(cursor.block(), 1);
            cursorBlock = cursor.block();
            KoTextBlockData data1(cursorBlock);
            cursor.insertText("\n----");
            cursorBlock = cursor.block();
            KoTextBlockData data2(cursorBlock);
            cursor.insertText("\n----");
            cursorBlock = cursor.block();
            KoTextBlockData data3(cursorBlock);

            KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout *>(textShape.textShapeData()->document()->documentLayout());
            if (lay) {
                lay->layout();
            }

            KoShapePaintingContext paintContext; //FIXME
            textShape.paintComponent(p, zoomHandler, paintContext);
            widget.bulletListButton->addItem(pm, static_cast<int>(item.style));
        }
    }

    widget.bulletListButton->addSeparator();

    QAction *action = new QAction(i18n("Change List Level"), this);
    action->setToolTip(i18n("Change the level the list is at"));

    QMenu *listLevelMenu = new QMenu();
    const int levelIndent = 13;
    for (int level = 0; level < 10; ++level) {
        QWidgetAction *wa = new QWidgetAction(listLevelMenu);
        ListLevelChooser *chooserWidget = new ListLevelChooser((levelIndent * level) + 5);
        wa->setDefaultWidget(chooserWidget);
        listLevelMenu->addAction(wa);
        m_mapper->setMapping(wa, level + 1);
        connect(chooserWidget, SIGNAL(clicked()), wa, SLOT(trigger()));
        connect(wa, SIGNAL(triggered()), m_mapper, SLOT(map()));
    }

    action->setMenu(listLevelMenu);
    widget.bulletListButton->addAction(action);
}

void SimpleParagraphWidget::setCurrentBlock(const QTextBlock &block)
{
    if (block == m_currentBlock) {
        return;
    }

    m_currentBlock = block;
    m_blockSignals = true;
    struct Finally {
        Finally(SimpleParagraphWidget *p)
        {
            parent = p;
        }
        ~Finally()
        {
            parent->m_blockSignals = false;
        }
        SimpleParagraphWidget *parent;
    };
    Finally finally(this);

    setCurrentFormat(m_currentBlock.blockFormat());
}

void SimpleParagraphWidget::setCurrentFormat(const QTextBlockFormat &format)
{
    if (!m_styleManager || format == m_currentBlockFormat) {
        return;
    }
    m_currentBlockFormat = format;

    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    KoParagraphStyle *style(m_styleManager->paragraphStyle(id));
    if (style) {
        bool unchanged = true;

        Q_FOREACH (int property, m_currentBlockFormat.properties().keys()) {
            switch (property) {
            case QTextFormat::ObjectIndex:
            case KoParagraphStyle::ListStyleId:
            case KoParagraphStyle::OutlineLevel:
            case KoParagraphStyle::ListStartValue:
            case KoParagraphStyle::IsListHeader:
            case KoParagraphStyle::UnnumberedListItem:
                continue;
            // These can be both content and style properties so let's ignore
            case KoParagraphStyle::BreakBefore:
            case KoParagraphStyle::MasterPageName:
                continue;

            default:
                break;
            }
            if (property == QTextBlockFormat::BlockAlignment) { //the default alignment can be retrieved in the defaultTextOption. However, calligra sets the Qt::AlignAbsolute flag, so we need to or this flag with the default alignment before comparing.
                if ((m_currentBlockFormat.property(property) != style->value(property))
                        && !(style->value(property).isNull()
                             && ((m_currentBlockFormat.intProperty(property)) == int(m_currentBlock.document()->defaultTextOption().alignment() | Qt::AlignAbsolute)))) {
                    unchanged = false;
                    break;
                } else {
                    continue;
                }
            }
            if (property == KoParagraphStyle::TextProgressionDirection) {
                if (style->value(property).isNull() && m_currentBlockFormat.intProperty(property) == KoText::LeftRightTopBottom) {
                    //LTR seems to be Qt default when unset
                    continue;
                }
            }
            if ((m_currentBlockFormat.property(property) != style->value(property)) && !(style->value(property).isNull() && !m_currentBlockFormat.property(property).toBool())) {
                //the last check seems to work. might be cause of a bug. The problem is when comparing an unset property in the style with a set to {0, false, ...) property in the format (eg. set then unset bold)
                unchanged = false;
                break;
            }
        }
        //we are updating the combo's selected item to what is the current format. we do not want this to apply the style as it would mess up the undo stack, the change tracking,...
        disconnect(widget.paragraphStyleCombo, SIGNAL(selected(QModelIndex)), this, SLOT(styleSelected(QModelIndex)));
        m_sortedStylesModel->styleApplied(style);
        widget.paragraphStyleCombo->setCurrentIndex(m_sortedStylesModel->indexOf(style).row());
        widget.paragraphStyleCombo->setStyleIsOriginal(unchanged);
        m_stylesModel->setCurrentParagraphStyle(id);
        widget.paragraphStyleCombo->slotUpdatePreview();
        connect(widget.paragraphStyleCombo, SIGNAL(selected(QModelIndex)), this, SLOT(styleSelected(QModelIndex)));
    }
}

void SimpleParagraphWidget::setStyleManager(KoStyleManager *sm)
{
    Q_ASSERT(sm);
    if (!sm || m_styleManager == sm) {
        return;
    }
    if (m_styleManager) {
        disconnect(m_styleManager, SIGNAL(styleApplied(const KoParagraphStyle*)), this, SLOT(slotParagraphStyleApplied(const KoParagraphStyle*)));
    }
    m_styleManager = sm;
    //we want to disconnect this before setting the stylemanager. Populating the model apparently selects the first inserted item. We don't want this to actually set a new style.
    disconnect(widget.paragraphStyleCombo, SIGNAL(selected(QModelIndex)), this, SLOT(styleSelected(QModelIndex)));
    m_stylesModel->setStyleManager(sm);
    m_sortedStylesModel->setStyleManager(sm);
    connect(widget.paragraphStyleCombo, SIGNAL(selected(QModelIndex)), this, SLOT(styleSelected(QModelIndex)));
    connect(m_styleManager, SIGNAL(styleApplied(const KoParagraphStyle*)), this, SLOT(slotParagraphStyleApplied(const KoParagraphStyle*)));
}

void SimpleParagraphWidget::setInitialUsedStyles(QVector<int> list)
{
    m_sortedStylesModel->setInitialUsedStyles(list);
}

void SimpleParagraphWidget::listStyleChanged(int id)
{
    emit doneWithFocus();
    if (m_blockSignals) {
        return;
    }
    KoListLevelProperties llp;
    llp.setStyle(static_cast<KoListStyle::Style>(id));
    llp.setLevel(1);
    KoTextEditor::ChangeListFlags flags(KoTextEditor::AutoListStyle);
    m_tool->textEditor()->setListProperties(llp, flags);
}

void SimpleParagraphWidget::styleSelected(int index)
{
    KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(m_sortedStylesModel->index(index, 0, QModelIndex()).internalId());
    if (paragStyle) {
        emit paragraphStyleSelected(paragStyle);
    }
    emit doneWithFocus();
}

void SimpleParagraphWidget::styleSelected(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(index.internalId());
    if (paragStyle) {
        emit paragraphStyleSelected(paragStyle);
    }
    emit doneWithFocus();
}

void SimpleParagraphWidget::slotShowStyleManager(int index)
{
    int styleId = m_sortedStylesModel->index(index, 0, QModelIndex()).internalId();
    emit showStyleManager(styleId);
    emit doneWithFocus();
}

void SimpleParagraphWidget::slotParagraphStyleApplied(const KoParagraphStyle *style)
{
    m_sortedStylesModel->styleApplied(style);
}

void SimpleParagraphWidget::changeListLevel(int level)
{
    emit doneWithFocus();
    if (m_blockSignals) {
        return;
    }

    m_tool->setListLevel(level);
}
