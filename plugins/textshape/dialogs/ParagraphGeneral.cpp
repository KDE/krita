/* This file is part of the KDE project
 * Copyright (C) 2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
 * Copyright (C) 2012 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>
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
#include "ParagraphDecorations.h"
#include "ParagraphDropCaps.h"
#include "StylesModel.h"

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoStyleThumbnailer.h>

ParagraphGeneral::ParagraphGeneral(QWidget *parent)
    : CharacterGeneral(parent)
    , m_nameHidden(false)
    , m_style(0)
    , m_styleManager(0)
    , m_thumbnail(new KoStyleThumbnailer())
    , m_paragraphInheritedStyleModel(new StylesModel(0, StylesModel::ParagraphStyle))
{
//Disable for now
    //include in TOC
    widget.inToc->setVisible(false);
//
    widget.nextStyle->setVisible(true);
    widget.label_2->setVisible(true);

    m_paragraphInheritedStyleModel->setStyleThumbnailer(m_thumbnail);
    widget.inheritStyle->setStylesModel(m_paragraphInheritedStyleModel);

    m_paragraphIndentSpacing = new ParagraphIndentSpacing(this);
    widget.tabs->addTab(m_paragraphIndentSpacing, i18n("Indent/Spacing"));

    connect(m_paragraphIndentSpacing, SIGNAL(parStyleChanged()), this, SIGNAL(styleChanged()));

    m_paragraphLayout = new ParagraphLayout(this);
    widget.tabs->addTab(m_paragraphLayout, i18n("General Layout"));

    connect(m_paragraphLayout, SIGNAL(parStyleChanged()), this, SIGNAL(styleChanged()));

    m_paragraphBulletsNumbers = new ParagraphBulletsNumbers(this);
    widget.tabs->addTab(m_paragraphBulletsNumbers, i18n("Bullets/Numbers"));

    connect(m_paragraphBulletsNumbers, SIGNAL(parStyleChanged()), this, SIGNAL(styleChanged()));

    m_paragraphDecorations = new ParagraphDecorations(this);
    widget.tabs->addTab(m_paragraphDecorations, i18n("Decorations"));

    connect(m_paragraphDecorations, SIGNAL(parStyleChanged()), this, SIGNAL(styleChanged()));

    m_paragraphDropCaps = new ParagraphDropCaps(this);
    widget.tabs->addTab(m_paragraphDropCaps, i18n("Drop Caps"));

    connect(m_paragraphDropCaps, SIGNAL(parStyleChanged()), this, SIGNAL(styleChanged()));

    widget.preview->setText(QString("Lorem ipsum dolor sit amet, consectetuer adipiscing elit, sed diam nonummy nibh euismod tincidunt ut laoreet dolore magna aliquam erat volutpat."));

    connect(widget.name, SIGNAL(textChanged(QString)), this, SIGNAL(nameChanged(QString)));
    connect(widget.nextStyle, SIGNAL(currentIndexChanged(int)), this, SIGNAL(styleChanged()));

    connect(this, SIGNAL(styleChanged()), this, SLOT(setPreviewParagraphStyle()));
}

void ParagraphGeneral::hideStyleName(bool hide)
{
    if (hide) {
        disconnect(widget.name, SIGNAL(textChanged(QString)), this, SIGNAL(nameChanged(QString)));
        widget.tabs->removeTab(0);
        m_nameHidden = true;
    }
}

void ParagraphGeneral::selectName()
{
    widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.generalTab));
    widget.name->selectAll();
    widget.name->setFocus(Qt::OtherFocusReason);
}

void ParagraphGeneral::setStyle(KoParagraphStyle *style, int level)
{
    m_style = style;
    if (m_style == 0) {
        return;
    }

    CharacterGeneral::setStyle(style);

    blockSignals(true);

    /*    widget.inheritStyle->clear();
        widget.inheritStyle->addItem(i18nc("Inherit style", "None"));
        widget.inheritStyle->setCurrentIndex(0);
        Q_FOREACH (KoParagraphStyle *s, m_paragraphStyles) {
            KoParagraphStyle *parent = s;
            bool ok = true;
            while (ok && parent) {
                ok = parent->styleId() != style->styleId();
                parent = parent->parentStyle();
            }
            if (!ok) continue; // can't inherit from myself, even indirectly.

            widget.inheritStyle->addItem(s->name(), s->styleId());
            if (s == style->parent())
                widget.inheritStyle->setCurrentIndex(widget.inheritStyle->count() - 1);
        }
    */
    if (!m_nameHidden) {
        widget.name->setText(style->name());
    }

    if (m_styleManager) {
        CharacterGeneral::updateNextStyleCombo(m_styleManager->paragraphStyle(style->nextStyle()));
        KoParagraphStyle *parentStyle = style->parentStyle();
        if (parentStyle) {
            widget.inheritStyle->setCurrentIndex(m_paragraphInheritedStyleModel->indexOf(*parentStyle).row());
            //m_paragraphInheritedStyleModel->setCurrentParagraphStyle(parentStyle->styleId());
        }
    }

    m_paragraphIndentSpacing->setDisplay(style);
    m_paragraphLayout->setDisplay(style);
    m_paragraphBulletsNumbers->setDisplay(style, level);
    m_paragraphDecorations->setDisplay(style);
    m_paragraphDropCaps->setDisplay(style);

    widget.preview->setParagraphStyle(style);

    blockSignals(false);
}

void ParagraphGeneral::setUnit(const KoUnit &unit)
{
    m_paragraphIndentSpacing->setUnit(unit);
    m_paragraphDropCaps->setUnit(unit);
}

void ParagraphGeneral::save(KoParagraphStyle *style)
{
    KoParagraphStyle *savingStyle;

    if (style == 0) {
        if (m_style == 0) {
            return;
        } else {
            savingStyle = m_style;
        }
    } else {
        savingStyle = style;
    }

    CharacterGeneral::save(style);

    m_paragraphIndentSpacing->save(savingStyle);
    m_paragraphLayout->save(savingStyle);
    m_paragraphBulletsNumbers->save(savingStyle);
    m_paragraphDecorations->save(savingStyle);
    m_paragraphDropCaps->save(savingStyle);
    savingStyle->setName(widget.name->text());
    if (int nextStyleId = CharacterGeneral::nextStyleId()) {
        savingStyle->setNextStyle(nextStyleId);
    }

    if (m_style == savingStyle) {
        emit styleAltered(savingStyle);
    }
}

void ParagraphGeneral::switchToGeneralTab()
{
    widget.tabs->setCurrentIndex(0);
}

void ParagraphGeneral::setPreviewParagraphStyle()
{
    KoParagraphStyle *parStyle = new KoParagraphStyle();
    save(parStyle);
    if (parStyle) {
        widget.preview->setParagraphStyle(parStyle);
    }

    delete parStyle;
}

void ParagraphGeneral::setImageCollection(KoImageCollection *imageCollection)
{
    m_paragraphBulletsNumbers->setImageCollection(imageCollection);
}

QString ParagraphGeneral::styleName() const
{
    return widget.name->text();
}

void ParagraphGeneral::setStyleManager(KoStyleManager *sm)
{
    if (!sm) {
        return;
    }
    m_styleManager = sm;
    CharacterGeneral::setStyleManager(m_styleManager);
    m_paragraphInheritedStyleModel->setStyleManager(m_styleManager);
}

KoParagraphStyle *ParagraphGeneral::style() const
{
    return m_style;
}
