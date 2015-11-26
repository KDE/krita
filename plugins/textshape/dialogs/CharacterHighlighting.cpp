/* This file is part of the KDE project
   Copyright (C)  2001, 2002 Montel Laurent <lmontel@mandrakesoft.com>
   Copyright (C)  2006-2007 Thomas Zander <zander@kde.org>
   Copyright (C)  2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
   Copyright (C)  2011 Mojtaba Shahi Senobari <mojtaba.shahi3000@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "CharacterHighlighting.h"

#include <KoText.h>
#include <KoCharacterStyle.h>
#include <KoIcon.h>

#include <QFontDatabase>
#include <QStringList>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTextCharFormat>

enum Position {
    Normal,
    Superscript,
    Subscript,
    Custom
};

CharacterHighlighting::CharacterHighlighting(bool uniqueFormat, QWidget *parent)
    : QWidget(parent)
    , m_uniqueFormat(uniqueFormat)
{
    widget.setupUi(this);

    QStringList list;
    KFontChooser::getFontList(list, KFontChooser::SmoothScalableFonts);
    m_fontChooser = new KFontChooser(this, (m_uniqueFormat) ? KFontChooser::NoDisplayFlags : KFontChooser::ShowDifferences, list, false);
    m_fontChooser->setSampleBoxVisible(false);
    widget.fontLayout->addWidget(m_fontChooser);

    widget.capitalizationList->addItems(capitalizationList());
    widget.underlineStyle->addItems(KoText::underlineTypeList());
    widget.underlineLineStyle->addItems(KoText::underlineStyleList());

    widget.positionList->addItems(fontLayoutPositionList());

    widget.strikethroughType->addItems(KoText::underlineTypeList()); //TODO make KoText consistent: either add strikethroughTypeList, or change from underlineTypeList to lineTypeList
    widget.strikethroughLineStyle->addItems(KoText::underlineStyleList()); //TODO idem

    connect(widget.underlineStyle, SIGNAL(activated(int)), this, SLOT(underlineTypeChanged(int)));
    connect(widget.underlineLineStyle, SIGNAL(activated(int)), this, SLOT(underlineStyleChanged(int)));
    connect(widget.underlineColor, SIGNAL(changed(QColor)), this, SLOT(underlineColorChanged(QColor)));

    connect(widget.strikethroughType, SIGNAL(activated(int)), this, SLOT(strikethroughTypeChanged(int)));
    connect(widget.strikethroughLineStyle, SIGNAL(activated(int)), this, SLOT(strikethroughStyleChanged(int)));
    connect(widget.strikethroughColor, SIGNAL(changed(QColor)), this, SLOT(strikethroughColorChanged(QColor)));

    connect(widget.capitalizationList, SIGNAL(activated(int)), this, SLOT(capitalisationChanged(int)));

    connect(widget.positionList, SIGNAL(activated(int)), this, SLOT(positionChanged(int)));

    connect(m_fontChooser, SIGNAL(fontSelected(QFont)), this, SIGNAL(fontChanged(QFont)));
    connect(m_fontChooser, SIGNAL(fontSelected(QFont)), this, SIGNAL(charStyleChanged()));

    const QIcon clearIcon = koIcon("edit-clear");
    widget.resetTextColor->setIcon(clearIcon);
    widget.resetBackground->setIcon(clearIcon);
    connect(widget.textColor, SIGNAL(changed(QColor)), this, SLOT(textColorChanged()));
    connect(widget.backgroundColor, SIGNAL(changed(QColor)), this, SLOT(backgroundColorChanged()));
    connect(widget.resetTextColor, SIGNAL(clicked()), this, SLOT(clearTextColor()));
    connect(widget.resetBackground, SIGNAL(clicked()), this, SLOT(clearBackgroundColor()));
    connect(widget.enableText, SIGNAL(toggled(bool)), this, SLOT(textToggled(bool)));
    connect(widget.enableBackground, SIGNAL(toggled(bool)), this, SLOT(backgroundToggled(bool)));
}

KoCharacterStyle::LineType CharacterHighlighting::indexToLineType(int index)
{
    KoCharacterStyle::LineType lineType;
    switch (index) {
    case 1: lineType = KoCharacterStyle::SingleLine; break;
    case 2: lineType = KoCharacterStyle::DoubleLine; break;
    case 0:
    default:
        lineType = KoCharacterStyle::NoLineType; break;
    }
    return lineType;
}

KoCharacterStyle::LineStyle CharacterHighlighting::indexToLineStyle(int index)
{
    KoCharacterStyle::LineStyle lineStyle;
    switch (index) {
    case 1: lineStyle = KoCharacterStyle::DashLine; break;
    case 2: lineStyle = KoCharacterStyle::DottedLine; break;
    case 3: lineStyle = KoCharacterStyle::DotDashLine; break;
    case 4: lineStyle = KoCharacterStyle::DotDotDashLine; break;
    case 5: lineStyle = KoCharacterStyle::WaveLine; break;
    case 0:
    default:
        lineStyle = KoCharacterStyle::SolidLine; break;
    }
    return lineStyle;
}

int CharacterHighlighting::lineTypeToIndex(KoCharacterStyle::LineType type)
{
    int index;
    switch (type) {
    case KoCharacterStyle::NoLineType: index = 0; break;
    case KoCharacterStyle::SingleLine: index = 1; break;
    case KoCharacterStyle::DoubleLine: index = 2; break;
    default: index = 0; break;
    }
    return index;
}

int CharacterHighlighting::lineStyleToIndex(KoCharacterStyle::LineStyle type)
{
    int index;
    switch (type) {
    case KoCharacterStyle::SolidLine: index = 0; break;
    case KoCharacterStyle::DashLine: index = 1; break;
    case KoCharacterStyle::DottedLine: index = 2; break;
    case KoCharacterStyle::DotDashLine: index = 3; break;
    case KoCharacterStyle::DotDotDashLine: index = 4; break;
    case KoCharacterStyle::WaveLine: index = 5; break;
    default: index = 0; break;
    }
    return index;
}

void CharacterHighlighting::capitalisationChanged(int item)
{
    if (m_uniqueFormat || widget.capitalizationList->currentIndex() >= 0) {
        switch (item) {
        case 0:
            emit capitalizationChanged(QFont::MixedCase);
            m_mixedCaseInherited = false;
            break;
        case 1:
            emit capitalizationChanged(QFont::SmallCaps);
            m_smallCapsInherited = false;
            break;
        case 2:
            emit capitalizationChanged(QFont::AllUppercase);
            m_allUpperCaseInherited = false;
            break;
        case 3:
            emit capitalizationChanged(QFont::AllLowercase);
            m_allLowerCaseInherited = false;
            break;
        case 4:
            emit capitalizationChanged(QFont::Capitalize);
            m_capitalizInherited = false;
            break;
        }
    }
    emit charStyleChanged();
}

void CharacterHighlighting::positionChanged(int item)
{
    Q_UNUSED(item);
    m_positionInherited = false;
    emit charStyleChanged();
}

void CharacterHighlighting::underlineTypeChanged(int item)
{
    widget.underlineLineStyle->setEnabled(item > 0);
    widget.underlineColor->setEnabled(item > 0);
    m_underlineInherited = false;
    emit underlineChanged(indexToLineType(item), indexToLineStyle(widget.underlineLineStyle->currentIndex()), widget.underlineColor->color());
    emit charStyleChanged();
}

void CharacterHighlighting::underlineStyleChanged(int item)
{
    if (widget.underlineStyle->currentIndex()) {
        emit underlineChanged(indexToLineType(widget.underlineStyle->currentIndex()), indexToLineStyle(item), widget.underlineColor->color());
    }
    m_underlineInherited = false;
    emit charStyleChanged();
}

void CharacterHighlighting::underlineColorChanged(QColor color)
{
    if (widget.underlineStyle->currentIndex()) {
        emit underlineChanged(indexToLineType(widget.underlineStyle->currentIndex()), indexToLineStyle(widget.underlineLineStyle->currentIndex()), color);
    }
    m_underlineInherited = false;
    emit charStyleChanged();
}

void CharacterHighlighting::strikethroughTypeChanged(int item)
{
    widget.strikethroughLineStyle->setEnabled(item > 0);
    widget.strikethroughColor->setEnabled(item > 0);
    m_strikeoutInherited = false;
    emit strikethroughChanged(indexToLineType(item), indexToLineStyle(widget.strikethroughLineStyle->currentIndex()), widget.strikethroughColor->color());
    emit charStyleChanged();
}

void CharacterHighlighting::strikethroughStyleChanged(int item)
{
    if (widget.strikethroughType->currentIndex()) {
        emit strikethroughChanged(indexToLineType(widget.strikethroughType->currentIndex()), indexToLineStyle(item), widget.strikethroughColor->color());
    }
    m_strikeoutInherited = false;
    emit charStyleChanged();
}

void CharacterHighlighting::strikethroughColorChanged(QColor color)
{
    if (widget.strikethroughType->currentIndex()) {
        emit strikethroughChanged(indexToLineType(widget.strikethroughType->currentIndex()), indexToLineStyle(widget.strikethroughLineStyle->currentIndex()), color);
    }
    m_strikeoutInherited = false;
    emit charStyleChanged();
}

void CharacterHighlighting::backgroundColorChanged()
{
    m_backgroundColorReset = false; m_backgroundColorChanged = true;
    if (widget.enableBackground->isChecked() && widget.backgroundColor->color().isValid()) {
        emit backgroundColorChanged(widget.backgroundColor->color());
    }
    emit charStyleChanged();
}

void CharacterHighlighting::textColorChanged()
{
    m_textColorReset = false; m_textColorChanged = true;
    if (widget.enableText->isChecked() && widget.textColor->color().isValid()) {
        emit textColorChanged(widget.textColor->color());
    }
    emit charStyleChanged();
}

void CharacterHighlighting::textToggled(bool state)
{
    widget.textColor->setEnabled(state);
    widget.resetTextColor->setEnabled(state);
    emit charStyleChanged();
}

void CharacterHighlighting::backgroundToggled(bool state)
{
    widget.backgroundColor->setEnabled(state);
    widget.resetBackground->setEnabled(state);
    emit charStyleChanged();
}

void CharacterHighlighting::clearTextColor()
{
    widget.textColor->setColor(widget.textColor->defaultColor());
    m_textColorReset = true;
    emit textColorChanged(QColor(Qt::black));
    emit charStyleChanged();
}

void CharacterHighlighting::clearBackgroundColor()
{
    widget.backgroundColor->setColor(widget.backgroundColor->defaultColor());
    m_backgroundColorReset = true;
    emit backgroundColorChanged(QColor(Qt::transparent));
    emit charStyleChanged();
}

QStringList CharacterHighlighting::capitalizationList()
{
    QStringList lst;
    lst << i18n("Normal");
    lst << i18n("Small Caps");
    lst << i18n("Uppercase");
    lst << i18n("Lowercase");
    lst << i18n("Capitalize");
    return lst;
}

QStringList CharacterHighlighting::fontLayoutPositionList()
{
    QStringList lst;
    lst << i18n("Normal");
    lst << i18n("Superscript");
    lst << i18n("Subscript");
    return lst;
}
void CharacterHighlighting::setDisplay(KoCharacterStyle *style)
{
    if (style == 0) {
        return;
    }

    QFont font = style->font();
    QFontDatabase dbase;
    QStringList availableStyles = dbase.styles(font.family());
    if (font.italic() && !(availableStyles.contains(QString("Italic"))) && availableStyles.contains(QString("Oblique"))) {
        font.setStyle(QFont::StyleOblique);
    }
    m_fontChooser->setFont(font);

    m_positionInherited  = !style->hasProperty(QTextFormat::TextVerticalAlignment);
    switch (style->verticalAlignment()) {
    case QTextCharFormat::AlignSuperScript:
        widget.positionList->setCurrentIndex(1);
        break;
    case QTextCharFormat::AlignSubScript:
        widget.positionList->setCurrentIndex(2);
        break;
    default:
        // TODO check if its custom instead.
        widget.positionList->setCurrentIndex(0);
    }
    if (!m_uniqueFormat) {
        widget.positionList->setEnabled(false);
        widget.positionList->setCurrentIndex(-1);
    }

    m_underlineInherited = !style->hasProperty(KoCharacterStyle::UnderlineStyle)
                           && !style->hasProperty(KoCharacterStyle::UnderlineType)
                           && !style->hasProperty(QTextFormat::TextUnderlineColor);
    m_strikeoutInherited = !style->hasProperty(KoCharacterStyle::StrikeOutStyle)
                           && !style->hasProperty(KoCharacterStyle::StrikeOutType)
                           && !style->hasProperty(KoCharacterStyle::StrikeOutColor);
    m_mixedCaseInherited = !style->hasProperty(QFont::MixedCase);
    m_smallCapsInherited = !style->hasProperty(QFont::SmallCaps);
    m_allUpperCaseInherited = !style->hasProperty(QFont::AllUppercase);
    m_allLowerCaseInherited = !style->hasProperty(QFont::AllLowercase);
    m_capitalizInherited = !style->hasProperty(QFont::Capitalize);

    //set the underline up
    widget.underlineStyle->setCurrentIndex(1);
    widget.underlineLineStyle->setCurrentIndex(lineStyleToIndex(style->underlineStyle()));
    if (m_uniqueFormat) {
        widget.underlineStyle->setCurrentIndex(lineTypeToIndex(style->underlineType()));
    } else {
        widget.underlineStyle->setCurrentIndex(-1);
    }

    underlineTypeChanged(widget.underlineStyle->currentIndex());
    widget.underlineColor->setColor(style->underlineColor());

    //set the strikethrough up
    widget.strikethroughType->setCurrentIndex(1);
    widget.strikethroughLineStyle->setCurrentIndex(lineStyleToIndex(style->strikeOutStyle()));
    if (m_uniqueFormat) {
        widget.strikethroughType->setCurrentIndex(lineTypeToIndex(style->strikeOutType()));
    } else {
        widget.strikethroughType->setCurrentIndex(-1);
    }
    strikethroughTypeChanged(widget.strikethroughType->currentIndex());
    widget.strikethroughColor->setColor(style->strikeOutColor());

    //Now set the capitalisation
    int index;
    switch (style->fontCapitalization()) {
    case QFont::MixedCase: widget.capitalizationList->setCurrentIndex(0); index = 0; break;
    case QFont::SmallCaps: widget.capitalizationList->setCurrentIndex(1); index = 1; break;
    case QFont::AllUppercase: widget.capitalizationList->setCurrentIndex(2); index = 2; break;
    case QFont::AllLowercase: widget.capitalizationList->setCurrentIndex(3); index = 3; break;
    case QFont::Capitalize: widget.capitalizationList->setCurrentIndex(4); index = 4; break;
    default:
        widget.capitalizationList->setCurrentIndex(0);
        index = 0;
        break;
    }

    if (m_uniqueFormat) {
        capitalisationChanged(index);
    } else {
        widget.capitalizationList->setCurrentIndex(-1);
        widget.capitalizationList->setEnabled(false);
    }

    //Set font decoration display
    widget.enableText->setVisible(!m_uniqueFormat);
    widget.enableText->setChecked(m_uniqueFormat);
    textToggled(m_uniqueFormat);
    widget.enableBackground->setVisible(!m_uniqueFormat);
    widget.enableBackground->setChecked(m_uniqueFormat);
    backgroundToggled(m_uniqueFormat);

    m_textColorChanged = false;
    m_backgroundColorChanged = false;
    m_textColorReset = ! style->hasProperty(QTextFormat::ForegroundBrush);
    if (m_textColorReset || (style->foreground().style() == Qt::NoBrush)) {
        clearTextColor();
    } else {
        widget.textColor->setColor(style->foreground().color());
    }
    m_backgroundColorReset = ! style->hasProperty(QTextFormat::BackgroundBrush);
    if (m_backgroundColorReset || (style->background().style() == Qt::NoBrush)) {
        clearBackgroundColor();
    } else {
        widget.backgroundColor->setColor(style->background().color());
    }
}

void CharacterHighlighting::save(KoCharacterStyle *style)
{
    if (style == 0) {
        return;
    }

    KFontChooser::FontDiffFlags fontDiff = m_fontChooser->fontDiffFlags();
    if (m_uniqueFormat || (fontDiff & KFontChooser::FontDiffFamily)) {
        style->setFontFamily(m_fontChooser->font().family());
    }
    if (m_uniqueFormat || (fontDiff & KFontChooser::FontDiffSize)) {
        style->setFontPointSize(m_fontChooser->font().pointSize());
    }
    if (m_uniqueFormat || (fontDiff & KFontChooser::FontDiffStyle)) {
        style->setFontWeight(m_fontChooser->font().weight());
        style->setFontItalic(m_fontChooser->font().italic()); //TODO should set style instead of italic
    }

    if (!m_underlineInherited) {
        style->setUnderlineStyle(indexToLineStyle(widget.underlineLineStyle->currentIndex()));
        style->setUnderlineColor(widget.underlineColor->color());
        style->setUnderlineType(indexToLineType(widget.underlineStyle->currentIndex()));
        if (widget.underlineStyle->currentIndex() == 0) {
            style->setUnderlineStyle(KoCharacterStyle::NoLineStyle);
        }
    }

    if (!m_strikeoutInherited) {
        style->setStrikeOutStyle(indexToLineStyle(widget.strikethroughLineStyle->currentIndex()));
        style->setStrikeOutColor(widget.strikethroughColor->color());
        style->setStrikeOutType(indexToLineType(widget.strikethroughType->currentIndex()));
        if (widget.strikethroughType->currentIndex() == 0) {
            style->setStrikeOutStyle(KoCharacterStyle::NoLineStyle);
        }
    }

    if (m_uniqueFormat || widget.capitalizationList->currentIndex() >= 0) {
        if (widget.capitalizationList->currentIndex() == 0 && !m_mixedCaseInherited) {
            style->setFontCapitalization(QFont::MixedCase);
        } else if (widget.capitalizationList->currentIndex() == 1 && !m_smallCapsInherited) {
            style->setFontCapitalization(QFont::SmallCaps);
        } else if (widget.capitalizationList->currentIndex() == 2 && !m_allUpperCaseInherited) {
            style->setFontCapitalization(QFont::AllUppercase);
        } else if (widget.capitalizationList->currentIndex() == 3 && !m_allLowerCaseInherited) {
            style->setFontCapitalization(QFont::AllLowercase);
        } else if (widget.capitalizationList->currentIndex() == 4 && !m_capitalizInherited) {
            style->setFontCapitalization(QFont::Capitalize);
        }
    }

    QTextCharFormat::VerticalAlignment va;
    if (m_uniqueFormat || widget.positionList->currentIndex() >= 0) {
        if (!m_positionInherited) {
            if (widget.positionList->currentIndex() == 0) {
                va = QTextCharFormat::AlignNormal;
            } else if (widget.positionList->currentIndex() == 2) {
                va = QTextCharFormat::AlignSubScript;
            } else if (widget.positionList->currentIndex() == 1) {
                va = QTextCharFormat::AlignSuperScript;
            } else {
                va = QTextCharFormat::AlignNormal;
            }
            style->setVerticalAlignment(va);
        }
    }

    if (widget.enableBackground->isChecked() && m_backgroundColorReset) {
        style->setBackground(QBrush(Qt::NoBrush));
    } else if (widget.enableBackground->isChecked() && m_backgroundColorChanged) {
        style->setBackground(QBrush(widget.backgroundColor->color()));
    }
    if (widget.enableText->isChecked() && m_textColorReset) {
        style->setForeground(QBrush(Qt::NoBrush));
    } else if (widget.enableText->isChecked() && m_textColorChanged) {
        style->setForeground(QBrush(widget.textColor->color()));
    }
}
