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

#include "ParagraphBulletsNumbers.h"

#include <KoParagraphStyle.h>
#include <KoListLevelProperties.h>
#include <KoCharSelectDia.h>

#include <KDebug>

ParagraphBulletsNumbers::ParagraphBulletsNumbers(QWidget *parent)
    : QWidget(parent),
    m_paragStyle(0)
{
    widget.setupUi(this);

    foreach(Lists::ListStyleItem item, Lists::genericListStyleItems())
        addStyle(item);
    addStyle( Lists::ListStyleItem( i18n( "Custom Bullet" ), KoListStyle::CustomCharItem) );
    foreach(Lists::ListStyleItem item, Lists::otherlistStyleItems())
        addStyle(item);

    widget.alignment->addItem(i18nc("Automatic horizontal alignment", "Auto"));
    widget.alignment->addItem(i18nc("Text alignment", "Left"));
    widget.alignment->addItem(i18nc("Text alignment", "Right"));
    widget.alignment->addItem(i18nc("Text alignment", "Centered"));

    connect(widget.listTypes, SIGNAL(currentRowChanged(int)), this, SLOT(styleChanged(int)));
    connect(widget.customCharacter, SIGNAL(clicked(bool)), this, SLOT(customCharButtonPressed()));
    connect(widget.letterSynchronization, SIGNAL(toggled(bool)), widget.startValue, SLOT(setLetterSynchronization(bool)));
}

void ParagraphBulletsNumbers::addStyle(const Lists::ListStyleItem &lsi) {
    m_mapping.insert(widget.listTypes->count(), lsi.style);
    widget.listTypes->addItem(lsi.name);
}

void ParagraphBulletsNumbers::open(KoParagraphStyle *style) {
    m_paragStyle = style;
    KoListStyle listStyle = style->listStyle();
    widget.listPropertiesPane->setEnabled(listStyle.isValid());
    widget.customCharacter->setText("-");
    if(!listStyle.isValid()) {
        widget.listTypes->setCurrentRow(0);
        return;
    }

    KoListLevelProperties llp = listStyle.levelProperties(m_paragStyle->listLevel());
    m_previousLevel = llp.level();
    widget.prefix->setText(llp.listItemPrefix());
    widget.suffix->setText(llp.listItemSuffix());
    widget.letterSynchronization->setChecked(llp.letterSynchronization());
    KoListStyle::Style s = llp.style();
    foreach(int row, m_mapping.keys()) {
        if(m_mapping[row] == s) {
            widget.listTypes->setCurrentRow(row);
            break;
        }
    }
    int align;
    if(llp.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute))
        align = 1;
    else if(llp.alignment() == (Qt::AlignRight | Qt::AlignAbsolute))
        align = 2;
    else if(llp.alignment() == Qt::AlignCenter)
        align = 3;
    else
        align = 0;

    widget.alignment->setCurrentIndex(align);
    widget.depth->setValue(llp.level());
    widget.levels->setValue(llp.displayLevel());
    widget.startValue->setValue(llp.startValue());
    if(s == KoListStyle::CustomCharItem)
        widget.customCharacter->setText(llp.bulletCharacter());

    // *** features not in GUI;
    // character style
    // relative bullet size (percent)
    // minimum label width
}

void ParagraphBulletsNumbers::save() {
kDebug() <<"ParagraphBulletsNumbers::save";
    Q_ASSERT(m_paragStyle);
    KoListStyle::Style style = m_mapping[widget.listTypes->currentRow()];
    if(style == KoListStyle::NoItem) {
        m_paragStyle->removeListStyle();
        return;
    }
    if(! m_paragStyle->listStyle().isValid()) {
        KoListStyle ls;
        m_paragStyle->setListStyle(ls);
    }
    KoListStyle listStyle = m_paragStyle->listStyle();
    KoListLevelProperties llp = listStyle.levelProperties(widget.depth->value());
    llp.setStyle(style);
    llp.setLevel(widget.depth->value());
    llp.setDisplayLevel(widget.levels->value());
    llp.setStartValue(widget.startValue->value());
    llp.setListItemPrefix(widget.prefix->text());
    llp.setListItemSuffix(widget.suffix->text());
    llp.setLetterSynchronization(widget.letterSynchronization->isVisible() && widget.letterSynchronization->isChecked());

    Qt::Alignment align;
    switch(widget.alignment->currentIndex()) {
        case 0: align = Qt::AlignLeft; break;
        case 1: align = Qt::AlignLeft | Qt::AlignAbsolute; break;
        case 2: align = Qt::AlignRight | Qt::AlignAbsolute; break;
        case 3: align = Qt::AlignCenter; break;
        default:
            Q_ASSERT(false);
    }
    llp.setAlignment(align);
    if(llp.level() != m_previousLevel)
        listStyle.removeLevelProperties(m_previousLevel);
    listStyle.setLevelProperties(llp);
    m_paragStyle->setListLevel(llp.level());
}

void ParagraphBulletsNumbers::styleChanged(int index) {
//kDebug() <<"ParagraphBulletsNumbers::styleChanged";
    KoListStyle::Style style = m_mapping[index];
    bool showLetterSynchronization = false;
    switch( style ) {
        case KoListStyle::SquareItem:
        case KoListStyle::DiscItem:
        case KoListStyle::CircleItem:
        case KoListStyle::BoxItem:
        case KoListStyle::CustomCharItem:
        case KoListStyle::NoItem:
            widget.startValue->setCounterType(KoListStyle::DecimalItem);
            widget.startValue->setValue(1);
            widget.startValue->setEnabled(false);
            break;
        case KoListStyle::AlphaLowerItem:
        case KoListStyle::UpperAlphaItem:
            showLetterSynchronization = true;
            // fall through
        default:
            widget.startValue->setEnabled(true);
            widget.startValue->setCounterType(style);
            int value = widget.startValue->value();
            widget.startValue->setValue(value +1);
            widget.startValue->setValue(value); // surely to trigger a change event.
    }
    widget.letterSynchronization->setVisible(showLetterSynchronization);
    widget.listPropertiesPane->setEnabled(style != KoListStyle::NoItem);
}

void ParagraphBulletsNumbers::customCharButtonPressed() {
    QString font;
    QChar character;
    if(KoCharSelectDia::selectChar(font, character, this)) {
        widget.customCharacter->setText(character);
        widget.customCharacter->setFont(QFont(font));
    }
    // also switch to the custom list style.
    foreach(int row, m_mapping.keys()) {
        if(m_mapping[row] == KoListStyle::CustomCharItem) {
            widget.listTypes->setCurrentRow(row);
            break;
        }
    }
}

#include "ParagraphBulletsNumbers.moc"
