/* This file is part of the KDE project
 * Copyright (C) 2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#include <KoImageData.h>
#include <KoImageCollection.h>
#include <KoUnit.h>
#include <KoFileDialog.h>
#include <KoDialog.h>

#include <QDebug>
#include <QUrl>

#include <kcharselect.h>

ParagraphBulletsNumbers::ParagraphBulletsNumbers(QWidget *parent)
    : QWidget(parent)
    , m_alignmentMode(false)
    , m_imageCollection(0)
    , m_data(0)
    , m_fontSize(0)
{
    widget.setupUi(this);

    Q_FOREACH (const Lists::ListStyleItem &item, Lists::genericListStyleItems()) {
        addStyle(item);
    }
    addStyle(Lists::ListStyleItem(i18n("Custom Bullet"), KoListStyle::CustomCharItem));
    m_blankCharIndex = addStyle(Lists::ListStyleItem(i18n("No Bullet"), KoListStyle::CustomCharItem));
    Q_FOREACH (const Lists::ListStyleItem &item, Lists::otherListStyleItems()) {
        addStyle(item);
    }

    widget.alignment->addItem(i18nc("Automatic horizontal alignment", "Auto"));
    widget.alignment->addItem(i18nc("Text alignment", "Left"));
    widget.alignment->addItem(i18nc("Text alignment", "Right"));
    widget.alignment->addItem(i18nc("Text alignment", "Centered"));

    widget.labelFollowedBy->addItem(i18nc("Tab follows the bullet or number", "Tab Stop"));
    widget.labelFollowedBy->addItem(i18nc("Space", "Space"));
    widget.labelFollowedBy->addItem(i18nc("None", "Nothing"));

    widget.doubleSpinBox->setSingleStep(0.05);
    widget.doubleSpinBox_2->setSingleStep(0.05);
    widget.doubleSpinBox_3->setSingleStep(0.05);

    connect(widget.labelFollowedBy, SIGNAL(currentIndexChanged(int)), this, SLOT(labelFollowedByIndexChanged(int)));
    connect(widget.listTypes, SIGNAL(currentRowChanged(int)), this, SLOT(styleChanged(int)));
    connect(widget.customCharacter, SIGNAL(clicked(bool)), this, SLOT(customCharButtonPressed()));
    connect(widget.letterSynchronization, SIGNAL(toggled(bool)), widget.startValue, SLOT(setLetterSynchronization(bool)));
    connect(widget.prefix, SIGNAL(textChanged(QString)), this, SLOT(recalcPreview()));
    connect(widget.suffix, SIGNAL(textChanged(QString)), this, SLOT(recalcPreview()));
    connect(widget.depth, SIGNAL(valueChanged(int)), this, SLOT(recalcPreview()));
    connect(widget.levels, SIGNAL(valueChanged(int)), this, SLOT(recalcPreview()));
    connect(widget.startValue, SIGNAL(valueChanged(int)), this, SLOT(recalcPreview()));
    connect(widget.insertImage, SIGNAL(clicked()), this, SLOT(selectListImage()));
    connect(widget.imageHeight, SIGNAL(valueChanged(double)), this, SLOT(recalcPreview()));
    connect(widget.imageWidth, SIGNAL(valueChanged(double)), this, SLOT(recalcPreview()));
    connect(widget.restartNumbering, SIGNAL(clicked()), this, SLOT(recalcPreview()));
}

int ParagraphBulletsNumbers::addStyle(const Lists::ListStyleItem &lsi)
{
    m_mapping.insert(widget.listTypes->count(), lsi.style);
    widget.listTypes->addItem(lsi.name);
    return widget.listTypes->count() - 1;
}

void ParagraphBulletsNumbers::setDisplay(KoParagraphStyle *style, int level)
{
    KoListStyle *listStyle = style->listStyle();
    widget.listPropertiesPane->setEnabled(listStyle != 0);
    widget.customCharacter->setText("-");
    if (listStyle == 0) {
        widget.listTypes->setCurrentRow(0);
        return;
    }

    KoListLevelProperties llp = listStyle->levelProperties(level);
    m_previousLevel = llp.level();
    widget.prefix->setText(llp.listItemPrefix());
    widget.suffix->setText(llp.listItemSuffix());
    widget.letterSynchronization->setChecked(llp.letterSynchronization());
    KoListStyle::Style s = llp.style();
    Q_FOREACH (int row, m_mapping.keys()) {
        if (m_mapping[row] == s) {
            widget.listTypes->setCurrentRow(row);
            break;
        }
    }
    int align;
    if (llp.alignment() == (Qt::AlignLeft | Qt::AlignAbsolute)) {
        align = 1;
    } else if (llp.alignment() == (Qt::AlignRight | Qt::AlignAbsolute)) {
        align = 2;
    } else if (llp.alignment() == Qt::AlignCenter) {
        align = 3;
    } else {
        align = 0;
    }

    widget.alignment->setCurrentIndex(align);
    widget.depth->setValue(llp.level());
    widget.levels->setValue(llp.displayLevel());
    widget.startValue->setValue(llp.startValue());
    if (s == KoListStyle::CustomCharItem) {
        widget.customCharacter->setText(llp.bulletCharacter());
    }

    if (s == KoListStyle::ImageItem) {
        m_data = llp.bulletImage();
        widget.imageHeight->setValue(llp.height());
        widget.imageWidth->setValue(llp.width());
    } else {
        m_data = 0;
        widget.imageHeight->setValue(0);
        widget.imageWidth->setValue(0);
    }

    if (llp.alignmentMode() == false) { //for list-level-position-and-space-mode=label-width-and-position disable the following options
        widget.label_8->setEnabled(false);
        widget.label_9->setEnabled(false);
        widget.label_10->setEnabled(false);
        widget.label_11->setEnabled(false);

        widget.labelFollowedBy->setEnabled(false);
        widget.doubleSpinBox->setEnabled(false);
        widget.doubleSpinBox_2->setEnabled(false);
        widget.doubleSpinBox_3->setEnabled(false);
    } else {
        m_alignmentMode = true;
        switch (llp.labelFollowedBy()) {
        case KoListStyle::ListTab:
            widget.doubleSpinBox->setEnabled(true);
            widget.labelFollowedBy->setCurrentIndex(0);
            widget.doubleSpinBox->setValue(KoUnit(KoUnit::Centimeter).toUserValue(llp.tabStopPosition()));
            break;
        case KoListStyle::Space:
            widget.doubleSpinBox->setEnabled(false);
            widget.labelFollowedBy->setCurrentIndex(1);
            break;
        case KoListStyle::Nothing:
            widget.doubleSpinBox->setEnabled(false);
            widget.labelFollowedBy->setCurrentIndex(2);
            break;
        default:
            Q_ASSERT(false);
        }

        widget.doubleSpinBox_2->setValue(KoUnit(KoUnit::Centimeter).toUserValue(llp.margin()));
        widget.doubleSpinBox_3->setValue(KoUnit(KoUnit::Centimeter).toUserValue(llp.margin()) + KoUnit(KoUnit::Centimeter).toUserValue(llp.textIndent()));
    }

    // *** features not in GUI;
    // character style
    // relative bullet size (percent)
    // minimum label width
    recalcPreview();
}

void ParagraphBulletsNumbers::save(KoParagraphStyle *savingStyle)
{
    Q_ASSERT(savingStyle);

    KoUnit unit(KoUnit::Centimeter);

    const int currentRow = widget.listTypes->currentRow();
    KoListStyle::Style style = m_mapping[currentRow];
    if (style == KoListStyle::None) {
        savingStyle->setListStyle(0);
        return;
    }
    if (savingStyle->listStyle() == 0) {
        KoListStyle *listStyle = new KoListStyle(savingStyle);
        savingStyle->setListStyle(listStyle);
    }
    KoListStyle *listStyle = savingStyle->listStyle();
    KoListLevelProperties llp = listStyle->levelProperties(widget.depth->value());
    llp.setStyle(style);
    llp.setLevel(widget.depth->value());
    llp.setDisplayLevel(widget.levels->value());
    llp.setStartValue(widget.startValue->value());
    llp.setListItemPrefix(widget.prefix->text());
    llp.setListItemSuffix(widget.suffix->text());
    llp.setLetterSynchronization(widget.letterSynchronization->isVisible() && widget.letterSynchronization->isChecked());

    if (m_alignmentMode == true) {
        llp.setAlignmentMode(true);
        switch (widget.labelFollowedBy->currentIndex()) {
        case 0: llp.setLabelFollowedBy(KoListStyle::ListTab);
            llp.setTabStopPosition(unit.fromUserValue(widget.doubleSpinBox->value()));
            break;
        case 1: llp.setLabelFollowedBy(KoListStyle::Space);
            break;
        case 2: llp.setLabelFollowedBy(KoListStyle::Nothing);
            break;
        default:
            Q_ASSERT(false);
        }

        llp.setMargin(unit.fromUserValue(widget.doubleSpinBox_2->value()));
        llp.setTextIndent(unit.fromUserValue(widget.doubleSpinBox_3->value()) - unit.fromUserValue(widget.doubleSpinBox_2->value()));
    }

    if (style == KoListStyle::ImageItem) {
        if (m_data) {
            llp.setBulletImage(m_data);
        }
        llp.setWidth(widget.imageWidth->value());
        llp.setHeight(widget.imageHeight->value());
    } else if (style == KoListStyle::CustomCharItem) {
        llp.setBulletCharacter((currentRow == m_blankCharIndex) ? QChar() : widget.customCharacter->text().remove('&').at(0));
    }
    // it is important to not use 45 for CustomCharItem as it is also char based
    else if (!KoListStyle::isNumberingStyle(style)) {
        llp.setRelativeBulletSize(45); //for non-numbering bullets the default relative bullet size is 45%(The spec does not say it; we take it)
    }

    Qt::Alignment align;
    switch (widget.alignment->currentIndex()) {
    case 0: align = Qt::AlignLeft; break;
    case 1: align = Qt::AlignLeft | Qt::AlignAbsolute; break;
    case 2: align = Qt::AlignRight | Qt::AlignAbsolute; break;
    case 3: align = Qt::AlignCenter; break;
    default:
        Q_ASSERT(false);
    }
    llp.setAlignment(align);

    if (llp.level() != m_previousLevel) {
        listStyle->removeLevelProperties(m_previousLevel);
    }
    listStyle->setLevelProperties(llp);
}

void ParagraphBulletsNumbers::styleChanged(int index)
{
    KoListStyle::Style style = m_mapping[index];
    bool showLetterSynchronization = false;

    if (style == KoListStyle::ImageItem) {
        widget.startValue->setValue(1);
        widget.startValue->setEnabled(false);
        widget.levels->setValue(1);
        widget.levels->setEnabled(false);
        widget.insertImage->setEnabled(true);
        widget.imageHeight->setEnabled(true);
        widget.imageWidth->setEnabled(true);

        if (widget.imageHeight->value() == 0 && widget.imageWidth->value() == 0) {
            widget.imageHeight->setValue(m_fontSize);
            widget.imageWidth->setValue(m_fontSize);
        }
    } else if (!KoListStyle::isNumberingStyle(style)) {
        widget.startValue->setCounterType(KoListStyle::DecimalItem);
        widget.startValue->setValue(1);
        widget.startValue->setEnabled(false);
        widget.levels->setValue(1);
        widget.levels->setEnabled(false);
        widget.insertImage->setEnabled(false);
        widget.imageHeight->setEnabled(false);
        widget.imageWidth->setEnabled(false);
        widget.imageHeight->setValue(0);
        widget.imageWidth->setValue(0);
    } else {
        switch (style) {
        case KoListStyle::AlphaLowerItem:
        case KoListStyle::UpperAlphaItem:
            showLetterSynchronization = true;
        Q_FALLTHROUGH();
        default:
            widget.levels->setEnabled(true);
            widget.startValue->setEnabled(true);
            widget.startValue->setCounterType(style);
            int value = widget.startValue->value();
            widget.startValue->setValue(value + 1);
            widget.startValue->setValue(value); // surely to trigger a change event.
            widget.insertImage->setEnabled(false);
            widget.imageHeight->setEnabled(false);
            widget.imageWidth->setEnabled(false);
        }
        widget.imageHeight->setValue(0);
        widget.imageWidth->setValue(0);
    }

    widget.customCharacter->setEnabled(style == KoListStyle::CustomCharItem && index != m_blankCharIndex);
    widget.letterSynchronization->setVisible(showLetterSynchronization);
    widget.listPropertiesPane->setEnabled(style != KoListStyle::None);
    recalcPreview();
}

void ParagraphBulletsNumbers::customCharButtonPressed()
{
    KoDialog *dialog = new KoDialog(this);
    dialog->setModal(true);
    dialog->setButtons(KoDialog::Ok | KoDialog::Cancel);
    dialog->setDefaultButton(KoDialog::Ok);

    KCharSelect *kcs = new KCharSelect(dialog, 0,
                                       KCharSelect::SearchLine | KCharSelect::FontCombo | KCharSelect::BlockCombos
                                       | KCharSelect::CharacterTable | KCharSelect::DetailBrowser);

    dialog->setMainWidget(kcs);
    if (dialog->exec() == KoDialog::Accepted) {
        QChar character = kcs->currentChar();
        widget.customCharacter->setText(character);

        // also switch to the custom list style.
        Q_FOREACH (int row, m_mapping.keys()) {
            if (m_mapping[row] == KoListStyle::CustomCharItem) {
                widget.listTypes->setCurrentRow(row);
                break;
            }
        }
    }
    delete dialog;
    recalcPreview();
}

void ParagraphBulletsNumbers::recalcPreview()
{
    emit parStyleChanged();
}

void ParagraphBulletsNumbers::labelFollowedByIndexChanged(int index)
{
    if (index == 1 || index == 2) {
        widget.doubleSpinBox->setEnabled(false);
    } else {
        widget.doubleSpinBox->setEnabled(true);
    }
    emit parStyleChanged();
    emit recalcPreview();
}

void ParagraphBulletsNumbers::setImageCollection(KoImageCollection *imageCollection)
{
    m_imageCollection = imageCollection;
}

void ParagraphBulletsNumbers::selectListImage()
{
    if (!m_imageCollection) {
        return;
    }

    KoFileDialog dlg(0, KoFileDialog::OpenFile, "bullets");
    dlg.setCaption(i18n("Select a list image"));
    if (!dlg.filename().isEmpty()) {
        QFile f(dlg.filename());
        if (f.exists()) {
            f.open(QIODevice::ReadOnly);
            QByteArray ba = f.readAll();
            f.close();
            if (m_imageCollection) {
                m_data = m_imageCollection->createImageData(ba);
            }
            emit recalcPreview();
        }
    }
}

void ParagraphBulletsNumbers::setFontSize(const KoCharacterStyle *style)
{
    m_fontSize = style->fontPointSize();
}
