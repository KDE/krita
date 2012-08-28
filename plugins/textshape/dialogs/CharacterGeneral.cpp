/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
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

#include "CharacterGeneral.h"
#include "CharacterHighlighting.h"
#include "LanguageTab.h"
#include "FontDecorations.h"
#include "FormattingPreview.h"

#include "StylesCombo.h"
#include "StylesModel.h"

#include <KoParagraphStyle.h>
#include <KoStyleThumbnailer.h>
#include <KoStyleManager.h>
#include <KoCharacterStyle.h>

#include "kdebug.h"

CharacterGeneral::CharacterGeneral(QWidget *parent)
        : QWidget(parent)
        , m_style(0)
        , m_styleManager(0)
        , m_thumbnail(new KoStyleThumbnailer())
        , m_paragraphStyleModel(new StylesModel(0,StylesModel::ParagraphStyle))
        , m_characterInheritedStyleModel(new StylesModel(0, StylesModel::CharacterStyle))
{
    widget.setupUi(this);
    // we don't have next style for character styles
    widget.nextStyle->setVisible(false);
    widget.label_2->setVisible(false);
    //

    // paragraph style model
    widget.nextStyle->showEditIcon(false);
    widget.nextStyle->setStyleIsOriginal(true);
    m_paragraphStyleModel->setStyleThumbnailer(m_thumbnail);
    widget.nextStyle->setStylesModel(m_paragraphStyleModel);
    // inherited style model
    widget.inheritStyle->showEditIcon(false);
    widget.inheritStyle->setStyleIsOriginal(true);
    //for character General
    m_characterInheritedStyleModel->setStyleThumbnailer(m_thumbnail);
    widget.inheritStyle->setStylesModel(m_characterInheritedStyleModel);
    widget.inheritStyle->setEnabled(false);

    m_characterHighlighting = new CharacterHighlighting(true, this);
    connect(m_characterHighlighting, SIGNAL(charStyleChanged()), this, SIGNAL(styleChanged()));
    connect(m_characterHighlighting, SIGNAL(charStyleChanged()), this, SLOT(setPreviewCharacterStyle()));

    m_languageTab = new LanguageTab(true, this);

    widget.tabs->addTab(m_characterHighlighting, i18n("Font"));

    m_languageTab->setVisible(false);

    connect(widget.name, SIGNAL(textChanged(const QString &)), this, SIGNAL(nameChanged(const QString&)));
}

void CharacterGeneral::hideStyleName(bool hide)
{
    if (hide) {
        disconnect(widget.name, SIGNAL(textChanged(const QString &)), this, SIGNAL(nameChanged(const QString&)));
        widget.tabs->removeTab(0);
        m_nameHidden = true;
    }
}

void CharacterGeneral::setStyle(KoCharacterStyle *style)
{
    m_style = style;
    if (m_style == 0)
        return;
    blockSignals(true);

    if (!m_nameHidden)
        widget.name->setText(style->name());

    m_characterHighlighting->setDisplay(style);
    //m_languageTab->setDisplay(style);

    widget.preview->setCharacterStyle(style);

    if (m_styleManager) {
        KoCharacterStyle *parentStyle = style->parentStyle();
        if (parentStyle) {
            widget.inheritStyle->setCurrentIndex(m_characterInheritedStyleModel->indexForCharacterStyle(*parentStyle).row());
        }
    }

    blockSignals(false);
}

void CharacterGeneral::save(KoCharacterStyle *style)
{
    KoCharacterStyle *savingStyle;
    if (style == 0) {
        if (m_style == 0)
            return;
        else
            savingStyle = m_style;
    }
    else
        savingStyle = style;

    m_characterHighlighting->save(savingStyle);
    //m_languageTab->save(savingStyle);
    savingStyle->setName(widget.name->text());

    if (m_style == savingStyle) {
        emit styleAltered(savingStyle);
    }
}

void CharacterGeneral::switchToGeneralTab()
{
    widget.tabs->setCurrentIndex(0);
}

void CharacterGeneral::selectName()
{
    widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.generalTab));
    widget.name->selectAll();
    widget.name->setFocus(Qt::OtherFocusReason);
}

void CharacterGeneral::setPreviewCharacterStyle()
{
    KoCharacterStyle *charStyle = new KoCharacterStyle();
    save(charStyle);
    if (charStyle) {
        widget.preview->setCharacterStyle(charStyle);
    }

    delete charStyle;
}

QString CharacterGeneral::styleName() const
{
    return widget.name->text();
}

void CharacterGeneral::setStyleManager(KoStyleManager *sm)
{
    if (!sm)
        return;
    m_styleManager = sm;
    m_paragraphStyleModel->setStyleManager(m_styleManager);
    m_characterInheritedStyleModel->setStyleManager(m_styleManager);
}

void CharacterGeneral::updateNextStyleCombo(KoParagraphStyle *style)
{
    widget.nextStyle->setCurrentIndex(m_paragraphStyleModel->indexForParagraphStyle(*style).row());
    m_paragraphStyleModel->setCurrentParagraphStyle(style->styleId());
}

int CharacterGeneral::nextStyleId()
{
    if (!m_styleManager) {
        return 0;
    }

    return m_styleManager->paragraphStyle(m_paragraphStyleModel->index(widget.nextStyle->currentIndex()).internalId())->styleId();
}

#include <CharacterGeneral.moc>
