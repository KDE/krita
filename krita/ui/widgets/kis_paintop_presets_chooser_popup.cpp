/* This file is part of the KDE project
 * Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
 * Copyright 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
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

#include "kis_paintop_presets_chooser_popup.h"

#include <KoResource.h>

#include <ui_wdgpaintoppresets.h>
#include <kmenu.h>
#include <kis_config.h>
#include <KoIcon.h>
#include <QCompleter>

struct KisPaintOpPresetsChooserPopup::Private
{
public:
    Ui_WdgPaintOpPresets uiWdgPaintOpPresets;
    bool firstShown;
};

KisPaintOpPresetsChooserPopup::KisPaintOpPresetsChooserPopup(QWidget * parent)
    : QWidget(parent)
    , m_d(new Private())
{
    m_d->uiWdgPaintOpPresets.setupUi(this);
    KMenu* menu = new KMenu(this);

    QActionGroup *actionGroup = new QActionGroup(this);

    KisPresetChooser::ViewMode mode = (KisPresetChooser::ViewMode)KisConfig().presetChooserViewMode();
    bool showAll = KisConfig().presetShowAllMode();

    QAction* action = menu->addAction(koIcon("view-preview"), i18n("Thumbnails"), this, SLOT(slotThumbnailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::THUMBNAIL);
    action->setActionGroup(actionGroup);

    action = menu->addAction(koIcon("view-list-details"), i18n("Details"), this, SLOT(slotDetailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::DETAIL);
    action->setActionGroup(actionGroup);

    m_d->uiWdgPaintOpPresets.viewModeButton->setIcon(koIcon("view-choose"));
    m_d->uiWdgPaintOpPresets.viewModeButton->setMenu(menu);
    m_d->uiWdgPaintOpPresets.viewModeButton->setPopupMode(QToolButton::InstantPopup);
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->setViewMode(mode);
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->showTaggingBar(false,true);

    connect(m_d->uiWdgPaintOpPresets.wdgPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(resourceSelected(KoResource*)));

    connect(m_d->uiWdgPaintOpPresets.searchBar, SIGNAL(textChanged(const QString&)),
            m_d->uiWdgPaintOpPresets.wdgPresetChooser, SLOT(searchTextChanged(const QString&)));

    connect(m_d->uiWdgPaintOpPresets.searchBar, SIGNAL(textChanged(const QString&)),
                this, SLOT(setLineEditCompleter(const QString&)));

    connect(m_d->uiWdgPaintOpPresets.searchBar, SIGNAL(returnPressed(QString)),
                this, SLOT(returnKeyPressed(QString)));

    connect(m_d->uiWdgPaintOpPresets.showAllCheckBox, SIGNAL(toggled(bool)),
            m_d->uiWdgPaintOpPresets.wdgPresetChooser, SLOT(setShowAll(bool)));
    m_d->firstShown = true;

    m_d->uiWdgPaintOpPresets.showAllCheckBox->setChecked(showAll);
}

KisPaintOpPresetsChooserPopup::~KisPaintOpPresetsChooserPopup()
{
    KisConfig().setPresetShowAllMode(m_d->uiWdgPaintOpPresets.showAllCheckBox->isChecked());
    delete m_d;
}

void KisPaintOpPresetsChooserPopup::setPresetFilter(const KoID& paintopID)
{
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->setPresetFilter(paintopID);
}

void KisPaintOpPresetsChooserPopup::slotThumbnailMode()
{
    KisConfig().setPresetChooserViewMode(KisPresetChooser::THUMBNAIL);
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->setViewMode(KisPresetChooser::THUMBNAIL);
}

void KisPaintOpPresetsChooserPopup::slotDetailMode()
{
    KisConfig().setPresetChooserViewMode(KisPresetChooser::DETAIL);
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->setViewMode(KisPresetChooser::DETAIL);
}

void KisPaintOpPresetsChooserPopup::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    //Workaround to get the column and row size right
    if(m_d->firstShown) {
        m_d->uiWdgPaintOpPresets.wdgPresetChooser->updateViewSettings();
        m_d->firstShown = false;
    }
}

void KisPaintOpPresetsChooserPopup::setLineEditCompleter(const QString& searchString)
{
    QCompleter* tagCompleter = new QCompleter(m_d->uiWdgPaintOpPresets.wdgPresetChooser->getTagNamesList(searchString),this);
    m_d->uiWdgPaintOpPresets.searchBar->setCompleter(tagCompleter);
}

void KisPaintOpPresetsChooserPopup::returnKeyPressed(QString lineEditText)
{
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->returnKeyPressed(lineEditText);
    if(!lineEditText.endsWith(", ")) {
        lineEditText.append(", ");
    }
    m_d->uiWdgPaintOpPresets.searchBar->setText(lineEditText);
    setLineEditCompleter(lineEditText);
}
