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

#include "ParagraphGeneral.h"
#include "ParagraphIndentSpacing.h"
#include "ParagraphLayout.h"
#include "ParagraphBulletsNumbers.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>

ParagraphGeneral::ParagraphGeneral(QWidget *parent)
    :QWidget(parent),
    m_blockSignals(false),
    m_style(0)
{
    widget.setupUi(this);

    m_paragraphIndentSpacing = new ParagraphIndentSpacing (this);
    m_paragraphLayout = new ParagraphLayout (this);
    m_paragraphBulletsNumbers = new ParagraphBulletsNumbers (this);

    widget.tabs->addTab(m_paragraphIndentSpacing, i18n("Indent/Spacing"));
    widget.tabs->addTab(m_paragraphLayout, i18n("General Layout"));
    widget.tabs->addTab(m_paragraphBulletsNumbers, i18n("Bullets/Numbers"));

    connect(widget.name, SIGNAL(textChanged (const QString &)), this, SIGNAL(nameChanged(const QString&)));
}

void ParagraphGeneral::setStyle(KoParagraphStyle *style) {
    m_style = style;
    if(m_style == 0)
        return;
    m_blockSignals = true;

    widget.inheritStyle->clear();
    widget.inheritStyle->addItem(i18nc("Inherit style", "None"));
    widget.inheritStyle->setCurrentIndex(0);
    foreach(KoParagraphStyle *s, m_paragraphStyles) {
        KoParagraphStyle *parent = s;
        bool ok = true;
        while(ok && parent) {
            ok = parent->styleId() != style->styleId();
            parent = parent->parent();
        }
        if(! ok) continue; // can't inherit from myself, even indirectly.

        widget.inheritStyle->addItem(s->name(), s->styleId());
        if(s == style->parent())
            widget.inheritStyle->setCurrentIndex( widget.inheritStyle->count()-1 );
    }
    widget.name->setText(style->name());
    for(int i=0; i < widget.nextStyle->count(); i++) {
        if(widget.nextStyle->itemData(i).toInt() == style->nextStyle()) {
            widget.nextStyle->setCurrentIndex(i);
            break;
        }
    }

    m_paragraphIndentSpacing->open(style);
    m_paragraphLayout->open(style);
    m_paragraphBulletsNumbers->open(style);

    m_blockSignals = false;
}

void ParagraphGeneral::setParagraphStyles(const QList<KoParagraphStyle*> styles) {
    m_paragraphStyles = styles;
    foreach(KoParagraphStyle *style, m_paragraphStyles)
        widget.nextStyle->addItem(style->name(), style->styleId());
}

void ParagraphGeneral::setUnit(const KoUnit &unit) {
    m_paragraphIndentSpacing->setUnit(unit);
}

void ParagraphGeneral::save() {
    if(m_style == 0) return;
    m_paragraphIndentSpacing->save();
    m_paragraphLayout->save();
    m_paragraphBulletsNumbers->save();

    m_style->setName(widget.name->text());
    m_style->setNextStyle(widget.nextStyle->itemData(widget.nextStyle->currentIndex()).toInt());
    int parentStyleId = widget.inheritStyle->itemData(widget.inheritStyle->currentIndex()).toInt();
    if(parentStyleId == 0)
        m_style->setParent(0);
    else {
        foreach(KoParagraphStyle *style, m_paragraphStyles) {
            if(style->styleId() == parentStyleId) {
                m_style->setParent(style);
                break;
            }
        }
    }
}

void ParagraphGeneral::switchToGeneralTab() {
    widget.tabs->setCurrentIndex(0);
}

#include <ParagraphGeneral.moc>
