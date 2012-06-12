/* This file is part of the KDE project
 * Copyright (C) 2007, 2008, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009-2010 C. Boemann <cbo@boemann.dk>
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

#include "SimpleCharacterWidget.h"
#include "TextTool.h"
#include "../commands/ChangeListCommand.h"
#include "StylesModel.h"
#include <KoStyleThumbnailer.h>

#include <KAction>
#include <KSelectAction>
#include <KoTextBlockData.h>
#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocumentLayout.h>
#include <KoZoomHandler.h>
#include <KoStyleManager.h>

#include <KDebug>

#include <QTextLayout>
#include <QComboBox>

SimpleCharacterWidget::SimpleCharacterWidget(TextTool *tool, QWidget *parent)
    : QWidget(parent),
      m_styleManager(0),
      m_blockSignals(false),
      m_comboboxHasBidiItems(false),
      m_tool(tool),
      m_thumbnailer(new KoStyleThumbnailer()),
      m_stylesModel(new StylesModel(0, StylesModel::CharacterStyle))
{
    widget.setupUi(this);
    widget.bold->setDefaultAction(tool->action("format_bold"));
    widget.italic->setDefaultAction(tool->action("format_italic"));
    widget.strikeOut->setDefaultAction(tool->action("format_strike"));
    widget.underline->setDefaultAction(tool->action("format_underline"));
    widget.textColor->setDefaultAction(tool->action("format_textcolor"));
    widget.backgroundColor->setDefaultAction(tool->action("format_backgroundcolor"));
    widget.superscript->setDefaultAction(tool->action("format_super"));
    widget.subscript->setDefaultAction(tool->action("format_sub"));
    widget.moreOptions->setText("...");
    widget.moreOptions->setToolTip(i18n("Change font format"));
    connect(widget.moreOptions, SIGNAL(clicked(bool)), tool->action("format_font"), SLOT(trigger()));

    connect(widget.bold, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.italic, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.strikeOut, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.underline, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.textColor, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.backgroundColor, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.superscript, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.subscript, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));

    QComboBox *family = qobject_cast<QComboBox*> (tool->action("format_fontfamily")->requestWidget(this));
    if (family) { // kdelibs 4.1 didn't return anything here.
        widget.fontsFrame->addWidget(family,0,0);
        connect(family, SIGNAL(activated(int)), this, SIGNAL(doneWithFocus()));
        connect(family, SIGNAL(activated(int)), this, SLOT(fontFamilyActivated(int)));
    }
    QComboBox *size = qobject_cast<QComboBox*> (tool->action("format_fontsize")->requestWidget(this));
    if (size) { // kdelibs 4.1 didn't return anything here.
        widget.fontsFrame->addWidget(size,0,1);
        connect(size, SIGNAL(activated(int)), this, SIGNAL(doneWithFocus()));
        connect(size, SIGNAL(activated(int)), this, SLOT(fontSizeActivated(int)));
        QDoubleValidator* validator = new QDoubleValidator(2, 999, 1, size);
        size->setValidator(validator);
    }

    widget.fontsFrame->setColumnStretch(0,1);

    m_stylesModel->setStyleThumbnailer(m_thumbnailer);
    widget.characterStyleCombo->setStylesModel(m_stylesModel);
    connect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
    connect(widget.characterStyleCombo, SIGNAL(newStyleRequested(QString)), this, SIGNAL(newStyleRequested(QString)));
    connect(widget.characterStyleCombo, SIGNAL(newStyleRequested(QString)), this, SIGNAL(doneWithFocus()));
    connect(widget.characterStyleCombo, SIGNAL(showStyleManager(int)), this, SLOT(slotShowStyleManager(int)));

}

SimpleCharacterWidget::~SimpleCharacterWidget()
{
    //the model is set on the comboBox which takes ownership
    delete m_thumbnailer;
}

void SimpleCharacterWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
    //we want to disconnect this before setting the stylemanager. Populating the model apparently selects the first inserted item. We don't want this to actually set a new style.
    disconnect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
    m_stylesModel->setStyleManager(sm);
    connect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
}

void SimpleCharacterWidget::setCurrentFormat(const QTextCharFormat& format, const QTextCharFormat& refBlockCharFormat)
{
    if (!m_styleManager || format == m_currentCharFormat) {
        return;
    }
    m_currentCharFormat = format;

    KoCharacterStyle *style(m_styleManager->characterStyle(m_currentCharFormat.intProperty(KoCharacterStyle::StyleId)));
    bool useParagraphStyle = false;
    if (!style) {
        style = static_cast<KoCharacterStyle*>(m_styleManager->paragraphStyle(m_currentCharFormat.intProperty(KoParagraphStyle::StyleId)));
        useParagraphStyle = true;
    }
    if (style) {
        bool unchanged = true;
        QTextCharFormat comparisonFormat = refBlockCharFormat;
        style->applyStyle(comparisonFormat);
        //Here we are making quite a few assumptions:
        //i. we can set the "ensured" properties on a blank charFormat. These corresponds to Qt default. We are not creating false positive (ie. different styles showing as identical).
        //ii. a property whose toBool returns as false is identical to an unset property (this is done through the clearUnsetProperties method)
        style->ensureMinimalProperties(comparisonFormat);
        style->ensureMinimalProperties(m_currentCharFormat);
        clearUnsetProperties(comparisonFormat);
        clearUnsetProperties(m_currentCharFormat);
        if (m_currentCharFormat.properties().count() != comparisonFormat.properties().count()) {
            unchanged = false;
        }
        else {
            foreach(int property, m_currentCharFormat.properties().keys()) {
                if (m_currentCharFormat.property(property) != comparisonFormat.property(property)) {
                    unchanged = false;
                }
            }
        }
        disconnect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
        widget.characterStyleCombo->setCurrentIndex((useParagraphStyle)?0:m_stylesModel->indexForCharacterStyle(*style).row());
        widget.characterStyleCombo->setStyleIsOriginal(unchanged);
        widget.characterStyleCombo->slotUpdatePreview();
        connect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
    }
}

void SimpleCharacterWidget::clearUnsetProperties(QTextFormat &format)
{
    foreach(int property, format.properties().keys()) {
        if (!format.property(property).toBool()) {
            format.clearProperty(property);
        }
    }
}

void SimpleCharacterWidget::fontFamilyActivated(int index) {
    /**
     * Hack:
     *
     * Selecting a font that is already selected in the combobox
     * will not trigger the action, so we help it on the way by
     * manually triggering it here if that happens.
     */
    if (index == m_lastFontFamilyIndex) {
        KSelectAction *action = qobject_cast<KSelectAction*>(m_tool->action("format_fontfamily"));
        if(action->currentAction())
            action->currentAction()->trigger();
    }
    m_lastFontFamilyIndex = index;
}

void SimpleCharacterWidget::fontSizeActivated(int index) {
    /**
     * Hack:
     *
     * Selecting a font size that is already selected in the
     * combobox will not trigger the action, so we help it on
     * the way by manually triggering it here if that happens.
     */
    if (index == m_lastFontSizeIndex) {
        KSelectAction *action = qobject_cast<KSelectAction*>(m_tool->action("format_fontsize"));
        action->currentAction()->trigger();
    }
    m_lastFontSizeIndex = index;
}

void SimpleCharacterWidget::setCurrentBlockFormat(const QTextBlockFormat &format)
{
    if (format == m_currentBlockFormat)
        return;
    m_currentBlockFormat = format;

    m_stylesModel->setCurrentParagraphStyle(format.intProperty(KoParagraphStyle::StyleId));
    disconnect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
    widget.characterStyleCombo->slotUpdatePreview();
    connect(widget.characterStyleCombo, SIGNAL(selected(int)), this, SLOT(styleSelected(int)));
}

void SimpleCharacterWidget::styleSelected(int index)
{
    KoCharacterStyle *charStyle = m_styleManager->characterStyle(m_stylesModel->index(index).internalId());

    //if the selected item correspond to a null characterStyle, send the null pointer. the tool should set the characterStyle as per paragraph
    emit characterStyleSelected(charStyle);
    emit doneWithFocus();
}

void SimpleCharacterWidget::slotShowStyleManager(int index)
{
    int styleId = m_stylesModel->index(index).internalId();
    emit showStyleManager(styleId);
    emit doneWithFocus();
}

#include <SimpleCharacterWidget.moc>
