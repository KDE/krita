/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include "StylesWidget.h"
#include "ParagraphGeneral.h"

#include <KoStyleManager.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <KDebug>
#include <KInputDialog>

StylesWidget::StylesWidget(Type type, QWidget *parent)
    : QWidget(parent),
    m_type(type),
    m_styleManager(0),
    m_blockSignals(false)
{
    widget.setupUi(this);
    connect(widget.styleList, SIGNAL(itemPressed(QListWidgetItem *)), this, SLOT(itemSelected(QListWidgetItem*)));
    connect(widget.newStyle, SIGNAL(pressed()), this, SLOT(newStyleClicked()));
    connect(widget.deleteStyle, SIGNAL(pressed()), this, SLOT(deleteStyleClicked()));
    connect(widget.styleList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(editStyle(QListWidgetItem*)));

    widget.newStyle->setIcon( KIcon("edit-add") );
    widget.deleteStyle->setIcon( KIcon("edit-delete") );
    widget.deleteStyle->setEnabled(false);
}

void StylesWidget::setStyleManager(KoStyleManager *sm) {
    if(sm == m_styleManager)
        return;
    if(m_styleManager) {
        disconnect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    }
    m_styleManager = sm;
    widget.styleList->clear();
    if(m_styleManager == 0)
        return;

    if(m_type == CharacterStyle) {
        foreach(KoCharacterStyle *style, m_styleManager->characterStyles()) {
            QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styleList);
            item->setData(99, style->styleId());
            widget.styleList->addItem(item);
        }
        connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
        connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    }
    else {
        foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles()) {
            QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styleList);
            item->setData(99, style->styleId());
            widget.styleList->addItem(item);
        }
        connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    }
}

void StylesWidget::itemSelected(QListWidgetItem *item) {
    if(m_blockSignals)
        return;
    int styleId = item->data(99).toInt();

    if(m_type == CharacterStyle)
        emit characterStyleSelected(m_styleManager->characterStyle(styleId));
    else
        emit paragraphStyleSelected(m_styleManager->paragraphStyle(styleId));
    widget.deleteStyle->setEnabled( widget.styleList->currentRow() > 0);
}

void StylesWidget::setCurrentFormat(const QTextBlockFormat &format) {
    if(m_type == CharacterStyle || format == m_currentBlockFormat)
        return;
    m_currentBlockFormat = format;
    int id = m_currentBlockFormat.intProperty(KoParagraphStyle::StyleId);
    int index = 0;
    while(index < widget.styleList->count()) {
        if(widget.styleList->item(index)->data(99).toInt() == id)
            break;
        index++;
    }
    if(index >= widget.styleList->count()) // not here, so default to the first one.
        index = 0;
    m_blockSignals = true;
    widget.styleList->setCurrentItem(widget.styleList->item(index));
    m_blockSignals = false;
}

void StylesWidget::setCurrentFormat(const QTextCharFormat &format) {
    if(format == m_currentCharFormat)
        return;
    m_currentCharFormat = format;

    int id = m_currentCharFormat.intProperty(KoCharacterStyle::StyleId);
    if(m_type == CharacterStyle) { // update the list-selection
        int index = 0;
        while(index < widget.styleList->count()) {
            if(widget.styleList->item(index)->data(99).toInt() == id)
                break;
            index++;
        }
        if(index >= widget.styleList->count()) // not here, so default to the first one.
            index = 0;
        m_blockSignals = true;
        widget.styleList->setCurrentItem(widget.styleList->item(index));
        m_blockSignals = false;
    }
    else { // if the characterStyle is not the same as our parag style's one, mark it.
        // TODO
    }
}

void StylesWidget::newStyleClicked() {
    bool ok;
    QString name = KInputDialog::getText(i18n("New Style"), i18n("Enter style name:"), i18n("New Style"), &ok, this);
    if(! ok)
        return;
    if(m_type == CharacterStyle) {
        KoCharacterStyle *style = new KoCharacterStyle();
        style->setName(name);
        m_styleManager->add(style);
    }
    else {
        KoParagraphStyle *style = new KoParagraphStyle();
        style->setName(name);
        style->characterStyle()->setName(name);
        m_styleManager->add(style);
    }
}

void StylesWidget::deleteStyleClicked() {
    QListWidgetItem *item = widget.styleList->currentItem();
    Q_ASSERT(item);
    int row = widget.styleList->row(item);
    int styleId = item->data(99).toInt();
    if(m_type == CharacterStyle) {
        KoCharacterStyle *style = m_styleManager->characterStyle(styleId);
        Q_ASSERT(style);
        m_styleManager->remove(style);
    }
    else {
        KoParagraphStyle *style = m_styleManager->paragraphStyle(styleId);
        Q_ASSERT(style);
        m_styleManager->remove(style);
    }
}

void StylesWidget::editStyle(QListWidgetItem *item) {
    QWidget *widget = 0;
    if(m_type == CharacterStyle) {
        //KoCharacterStyle *style = m_styleManager->characterStyle(item->data(99).toInt());
        // TODO
    }
    else {
        KoParagraphStyle *style = m_styleManager->paragraphStyle(item->data(99).toInt());
        ParagraphGeneral *p = new ParagraphGeneral();
        // TODO get KoUnit from somewhere and set that on p
        p->setStyle(style);
        widget = p;
    }
    if(widget) {
        KDialog *dia = new KDialog(this);
        dia->setMainWidget(widget);
        dia->show();
    }
}

void StylesWidget::addParagraphStyle(KoParagraphStyle *style) {
    Q_ASSERT(m_type == ParagraphStyle);
    QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styleList);
    item->setData(99, style->styleId());
    widget.styleList->addItem(item);
}

void StylesWidget::addCharacterStyle(KoCharacterStyle *style) {
    Q_ASSERT(m_type == CharacterStyle);
    QListWidgetItem *item = new QListWidgetItem(style->name(), widget.styleList);
    item->setData(99, style->styleId());
    widget.styleList->addItem(item);
}

void StylesWidget::removeParagraphStyle(KoParagraphStyle *style) {
    Q_ASSERT(m_type == ParagraphStyle);
    removeStyle(style->styleId());
}

void StylesWidget::removeCharacterStyle(KoCharacterStyle *style) {
    Q_ASSERT(m_type == CharacterStyle);
    removeStyle(style->styleId());
}

void StylesWidget::removeStyle(int styleId) {
    for(int i=0; i < widget.styleList->count(); i++) {
        QListWidgetItem *item = widget.styleList->currentItem();
        int id = item->data(99).toInt();
        if(id == styleId) {
            delete item;

            widget.deleteStyle->setEnabled( false );
            item = widget.styleList->currentItem();
            if(item) {
                widget.deleteStyle->setEnabled( widget.styleList->count() > 1 );

                if(widget.styleList->row(item) == 0)
                    widget.styleList->setCurrentRow(1);
            }
            return;
        }
    }
}

#include <StylesWidget.moc>
