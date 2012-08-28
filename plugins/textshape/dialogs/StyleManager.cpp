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

#include "StyleManager.h"

#include "StylesModel.h"

#include <KoStyleManager.h>
#include <KoStyleThumbnailer.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

#include <QListView>
#include <QModelIndex>
#include <QTabWidget>
#include <QInputDialog>
#include <QMessageBox>
#include <QInputDialog>

#include <KDebug>

StyleManager::StyleManager(QWidget *parent)
        : QWidget(parent)
         ,m_styleManager(0)
        , m_paragraphStylesModel(new StylesModel(0, StylesModel::ParagraphStyle))
        , m_characterStylesModel(new StylesModel(0, StylesModel::CharacterStyle))
        , m_thumbnailer(new KoStyleThumbnailer())
        , m_selectedParagStyle(0)
        , m_selectedCharStyle(0)
        , m_blockSignals(false)
        , m_blockStyleChangeSignals(false)
        , m_unappliedStyleChanges(false)
        , m_currentStyleChanged(false)
{
    widget.setupUi(this);
    layout()->setMargin(0);
    widget.bNew->setToolTip(i18n("Create a new style inheriting the current style"));

    // Force "Base" background of the style listviews to white, so the background
    // is consistent with the one of the preview area. Also the usual document text colors
    // are dark, because made for a white paper background, so with a dark UI
    // color scheme they are hardly seen.
    // TODO: update to background color of currently selected/focused shape/page
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, QColor(Qt::white));
    widget.paragraphStylesListView->setPalette(palette);
    widget.characterStylesListView->setPalette(palette);

    m_paragraphStylesModel->setStyleThumbnailer(m_thumbnailer);
    m_characterStylesModel->setStyleThumbnailer(m_thumbnailer);
    m_characterStylesModel->setProvideStyleNone(false);
    widget.paragraphStylesListView->setModel(m_paragraphStylesModel);
    widget.characterStylesListView->setModel(m_characterStylesModel);

    connect(widget.paragraphStylesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotStyleSelected(QModelIndex)));
    connect(widget.characterStylesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotStyleSelected(QModelIndex)));

    connect(widget.bNew, SIGNAL(pressed()), this, SLOT(buttonNewPressed()));
    connect(widget.bDelete, SIGNAL(pressed()), this, SLOT(buttonDeletePressed()));
    widget.bDelete->setVisible(false); // TODO make it visible when we can safely delete styles

    connect(widget.tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    connect(widget.paragraphStylePage, SIGNAL(styleChanged()), this, SLOT(currentStyleChanged()));
    connect(widget.characterStylePage, SIGNAL(styleChanged()), this, SLOT(currentStyleChanged()));
    connect(widget.paragraphStylePage, SIGNAL(nameChanged(const QString &)), this, SLOT(currentStyleChanged()));
    connect(widget.characterStylePage, SIGNAL(nameChanged(const QString &)), this, SLOT(currentStyleChanged()));
}

StyleManager::~StyleManager()
{
    foreach(int styleId, m_alteredParagraphStyles.keys()) {
        KoParagraphStyle *s = m_alteredParagraphStyles[styleId];
        delete s;
    }
    m_alteredParagraphStyles.clear();
    foreach(int styleId, m_alteredCharacterStyles.keys()) {
        KoCharacterStyle *s = m_alteredCharacterStyles[styleId];
        delete s;
    }
    m_alteredCharacterStyles.clear();
}

void StyleManager::setStyleManager(KoStyleManager *sm)
{
    Q_ASSERT(sm);
    m_styleManager = sm;

    //we want to disconnect this before setting the stylemanager. Populating the model apparently selects the first inserted item. We don't want this to actually set a new style.
    disconnect(widget.paragraphStylesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotStyleSelected(QModelIndex)));
    m_paragraphStylesModel->setStyleManager(m_styleManager);
    connect(widget.paragraphStylesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotStyleSelected(QModelIndex)));

    disconnect(widget.characterStylesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotStyleSelected(QModelIndex)));
    m_characterStylesModel->setStyleManager(m_styleManager);
    connect(widget.characterStylesListView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(slotStyleSelected(QModelIndex)));

    widget.paragraphStylePage->setStyleManager(m_styleManager); //also updates style combos
    widget.characterStylePage->setStyleManager(m_styleManager); //also updates style combos
    widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.paragraphStylesListView));
    connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));

    m_blockStyleChangeSignals = true;
    widget.paragraphStylesListView->setCurrentIndex(m_paragraphStylesModel->firstStyleIndex());
    widget.characterStylesListView->setCurrentIndex(m_characterStylesModel->firstStyleIndex());
    m_selectedParagStyle = m_styleManager->paragraphStyle(m_paragraphStylesModel->firstStyleIndex().internalId());
    m_selectedCharStyle = 0;
    widget.paragraphStylePage->setStyle(m_selectedParagStyle);
    widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    m_blockStyleChangeSignals = false;
}

void StyleManager::setParagraphStyle(KoParagraphStyle *style)
{
    m_blockStyleChangeSignals = true;
    m_selectedCharStyle = 0;
    m_selectedParagStyle = style;
    widget.characterStylePage->save();
    widget.characterStylePage->setStyle(0);
    widget.paragraphStylePage->save();
    KoParagraphStyle *localStyle;

    if (m_draftParagraphStyles.contains(style->styleId())) {
        localStyle = m_draftParagraphStyles[style->styleId()];
    }
   else if (!m_alteredParagraphStyles.contains(style->styleId())) {
        localStyle = style->clone();
        m_alteredParagraphStyles.insert(style->styleId(), localStyle);
    }
    else {
        localStyle = m_alteredParagraphStyles.value(style->styleId());
    }

    widget.paragraphStylesListView->setCurrentIndex(m_paragraphStylesModel->indexForParagraphStyle(*style));
    widget.paragraphStylePage->setStyle(localStyle);
    widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.paragraphStylesListView));
 //   widget.bDelete->setEnabled(canDelete);

    m_blockStyleChangeSignals = false;
    m_currentStyleChanged = false;
}

void StyleManager::setCharacterStyle(KoCharacterStyle *style, bool canDelete)
{
    m_blockStyleChangeSignals = true;
    m_selectedParagStyle = 0;
    m_selectedCharStyle = style;
    widget.paragraphStylePage->save();
    widget.paragraphStylePage->setStyle(0);
    widget.characterStylePage->save();
    KoCharacterStyle *localStyle;

    if (m_draftParagraphStyles.contains(style->styleId())) {
        localStyle = m_draftParagraphStyles[style->styleId()];
    }
    else if (!m_alteredCharacterStyles.contains(style->styleId())) {
        localStyle = style->clone();
        m_alteredCharacterStyles.insert(style->styleId(), localStyle);
    }
    else {
        localStyle = m_alteredCharacterStyles.value(style->styleId());
    }

    widget.characterStylesListView->setCurrentIndex(m_characterStylesModel->indexForCharacterStyle(*style));
    widget.characterStylePage->setStyle(localStyle);
    widget.stackedWidget->setCurrentWidget(widget.characterStylePage);
    widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.characterStylesListView));
 //   widget.bDelete->setEnabled(canDelete);
    m_blockStyleChangeSignals = false;
    m_currentStyleChanged = false;
}

void StyleManager::setUnit(const KoUnit &unit)
{
    widget.paragraphStylePage->setUnit(unit);
}

void StyleManager::save()
{
    if (!m_unappliedStyleChanges) {
        return;
    }
    m_blockSignals = true;
    widget.paragraphStylePage->save();
    widget.characterStylePage->save();

    m_styleManager->beginEdit();

    m_paragraphStylesModel->clearDraftStyles(); // clear draft styles in Style Model.
    foreach(KoParagraphStyle *style, m_draftParagraphStyles.values()) {
        m_styleManager->add(style);
    }
    foreach(KoCharacterStyle *style, m_draftCharacterStyles.values()) {
        m_styleManager->add(style);
    }
    m_draftParagraphStyles.clear();
    m_draftCharacterStyles.clear();

    foreach(int styleId, m_alteredCharacterStyles.keys()) {
        KoCharacterStyle *altered = m_alteredCharacterStyles[styleId];
        m_styleManager->alteredStyle(altered);
        delete altered;
    }
    m_alteredCharacterStyles.clear();

    foreach(int styleId, m_alteredParagraphStyles.keys()) {
        KoParagraphStyle *altered = m_alteredParagraphStyles[styleId];
        m_styleManager->alteredStyle(altered);
        delete altered;
    }
    m_alteredParagraphStyles.clear();

    m_styleManager->endEdit();

    //Reset the active style
    if (m_selectedCharStyle) {
        KoCharacterStyle *localStyle = m_selectedCharStyle->clone();

        m_alteredCharacterStyles.insert(m_selectedCharStyle->styleId(), localStyle);

        widget.characterStylePage->setStyle(localStyle);
    }
    else
        widget.characterStylePage->setStyle(0);

    if (m_selectedParagStyle) {
        KoParagraphStyle *localStyle = m_selectedParagStyle->clone();

        m_alteredParagraphStyles.insert(m_selectedParagStyle->styleId(), localStyle);

        widget.paragraphStylePage->setStyle(localStyle);
    }
    else
        widget.paragraphStylePage->setStyle(0);
    m_blockSignals = false;

    m_currentStyleChanged = false;
    m_unappliedStyleChanges = false;
}

void StyleManager::currentStyleChanged()
{
    if (!m_blockStyleChangeSignals) {
        m_currentStyleChanged = true;
        m_unappliedStyleChanges = true;
    }
}

void StyleManager::addParagraphStyle(KoParagraphStyle *style)
{
    if (m_blockSignals) return;

    widget.paragraphStylePage->setStyleManager(m_styleManager); //updates style combos
    setParagraphStyle(style);
    m_unappliedStyleChanges = true;
}

void StyleManager::addCharacterStyle(KoCharacterStyle *style)
{
    if (m_blockSignals) return;

    widget.characterStylePage->setStyleManager(m_styleManager); //updates style combos
    setCharacterStyle(style);
    m_unappliedStyleChanges = true;
}

void StyleManager::removeParagraphStyle(KoParagraphStyle* style)
{
    if (m_alteredParagraphStyles.contains(style->styleId()))
        m_alteredParagraphStyles.remove(style->styleId());
    widget.paragraphStylePage->setStyleManager(m_styleManager); //updates style combos
}

void StyleManager::removeCharacterStyle(KoCharacterStyle* style)
{
    if (m_alteredCharacterStyles.contains(style->styleId()))
        m_alteredCharacterStyles.remove(style->styleId());
    widget.characterStylePage->setStyleManager(m_styleManager); //updates style combos
}

void StyleManager::slotStyleSelected(QModelIndex index)
{
    if (m_blockSignals) return;

    if (!checkUniqueStyleName()) {
        if (widget.tabs->indexOf(widget.paragraphStylesListView) == widget.tabs->currentIndex()){
            widget.paragraphStylesListView->setCurrentIndex(m_paragraphStylesModel->indexForParagraphStyle(*m_selectedParagStyle));
            widget.paragraphStylePage->selectName();
        } else {
            widget.characterStylesListView->setCurrentIndex(m_characterStylesModel->indexForCharacterStyle(*m_selectedCharStyle));
            widget.characterStylePage->selectName();
        }
        return;
    }
    KoParagraphStyle *paragraphStyle = m_styleManager->paragraphStyle(index.internalId());;
    if (!paragraphStyle && m_draftParagraphStyles.contains(index.internalId())) {
        paragraphStyle = m_draftParagraphStyles[index.internalId()];
    }
    if (paragraphStyle) {
        setParagraphStyle(paragraphStyle);
        return;
    }
    KoCharacterStyle *characterStyle = m_styleManager->characterStyle(index.internalId());
    if (!characterStyle && m_draftCharacterStyles.contains(index.internalId()))
        characterStyle = m_draftCharacterStyles.value(index.internalId());
    if (characterStyle) {
        setCharacterStyle(characterStyle, false);
        return;
    }
}

void StyleManager::buttonNewPressed()
{
    if (!checkUniqueStyleName()) {
        m_blockSignals = true;
        if (widget.tabs->indexOf(widget.paragraphStylesListView) == widget.tabs->currentIndex()){
            widget.paragraphStylePage->selectName();
        } else {
            widget.characterStylePage->selectName();
        }
        m_blockSignals = false;
        return;
    }
    if (widget.tabs->indexOf(widget.paragraphStylesListView) == widget.tabs->currentIndex()){
        KoParagraphStyle *newStyle = m_selectedParagStyle->clone();
        newStyle->setName(i18n("New Style"));
        m_paragraphStylesModel->addDraftParagraphStyle(newStyle);
        m_draftParagraphStyles.insert(newStyle->styleId(), newStyle);
        addParagraphStyle(newStyle);
        widget.paragraphStylePage->selectName();
    }
    else {
        KoCharacterStyle *newStyle = m_selectedCharStyle->clone();
        newStyle->setName(i18n("New Style"));
        m_characterStylesModel->addDraftCharacterStyle(newStyle);
        m_draftCharacterStyles.insert(newStyle->styleId(), newStyle);
        addCharacterStyle(newStyle);
        widget.characterStylePage->selectName();
    }
}

void StyleManager::tabChanged(int index)
{
    if (m_blockSignals) return;

    if (!checkUniqueStyleName()) {
        m_blockSignals = true;
        if (widget.tabs->indexOf(widget.paragraphStylesListView) == widget.tabs->currentIndex()){
            widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.characterStylesListView));
        } else {
            widget.tabs->setCurrentIndex(widget.tabs->indexOf(widget.paragraphStylesListView));
        }
        widget.characterStylePage->selectName();
        m_blockSignals = false;
        return;
    }
    if (widget.tabs->indexOf(widget.paragraphStylesListView) == index) {
        m_selectedParagStyle = m_styleManager->paragraphStyle(widget.paragraphStylesListView->currentIndex().internalId());
        if (!m_selectedParagStyle && m_draftParagraphStyles.contains(widget.paragraphStylesListView->currentIndex().internalId()))
            m_selectedParagStyle = m_draftParagraphStyles[widget.paragraphStylesListView->currentIndex().internalId()];
        m_selectedCharStyle = 0;
        setParagraphStyle(m_selectedParagStyle);
        widget.stackedWidget->setCurrentWidget(widget.paragraphStylePage);
    }
    else {
        m_selectedCharStyle = m_styleManager->characterStyle(widget.characterStylesListView->currentIndex().internalId());
        if (!m_selectedCharStyle && m_draftCharacterStyles.contains(widget.characterStylesListView->currentIndex().internalId()))
            m_selectedCharStyle = m_draftCharacterStyles[widget.characterStylesListView->currentIndex().internalId()];
        m_selectedParagStyle = 0;
        setCharacterStyle(m_selectedCharStyle);
        widget.stackedWidget->setCurrentWidget(widget.characterStylePage);
    }
}

bool StyleManager::unappliedStyleChanges()
{
    return m_unappliedStyleChanges;
}

bool StyleManager::checkUniqueStyleName()
{
    if (m_selectedParagStyle) {
        QList<int> styleListPar = m_paragraphStylesModel->StyleList();
        QList<int>::iterator iterPar = styleListPar.begin();
        for ( ; iterPar != styleListPar.end(); ++iterPar) {
            KoParagraphStyle *temp = m_styleManager->paragraphStyle(*iterPar);
            if (!temp && m_draftParagraphStyles.contains(*iterPar))
                temp = m_draftParagraphStyles[*iterPar];
            if (widget.paragraphStylePage->styleName() == temp->name()) {
                if (temp != m_selectedParagStyle) {
                    QMessageBox::critical(this, i18n("Warning"), i18n("Another style named '%1' already exist. Please choose another name.").arg(temp->name()));
                    return false;
                }
            }

        }
    }
    if (m_selectedCharStyle) {
        QList<int> styleListChar = m_characterStylesModel->StyleList();
        QList<int>::iterator iterChar = styleListChar.begin();
        for ( ; iterChar != styleListChar.end(); ++iterChar) {
            KoCharacterStyle *temp = m_styleManager->characterStyle(*iterChar);;
            if (!temp && m_draftCharacterStyles.contains(*iterChar))
                temp = m_draftCharacterStyles[*iterChar];

            if (widget.characterStylePage->styleName() == temp->name()) {
                if (temp != m_selectedCharStyle) {
                    QMessageBox::critical(this, i18n("Warning"), i18n("Another style named '%1' already exist. Please choose another name.").arg(temp->name()));
                    widget.characterStylesListView->setCurrentIndex(m_characterStylesModel->indexForCharacterStyle(*m_selectedCharStyle));
                    return false;
                }
            }

        }
    }

    return true;
}

/* TODO
    On new move focus to name text field.
*/

#include <StyleManager.moc>
