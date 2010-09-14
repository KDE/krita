/* This file is part of the KDE project
 * Copyright (c) 2010 Sven Langkamp <sven.langkamp@gmail.com>
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

class KisPaintOpPresetsChooserPopup::Private
{
public:
    Ui_WdgPaintOpPresets uiWdgPaintOpPresets;
};

KisPaintOpPresetsChooserPopup::KisPaintOpPresetsChooserPopup(QWidget * parent)
    : QWidget(parent)
    , m_d(new Private())
{
    m_d->uiWdgPaintOpPresets.setupUi(this);
    KMenu* menu = new KMenu(this);
    
    QActionGroup *actionGroup = new QActionGroup(this);

    KisPresetChooser::ViewMode mode = (KisPresetChooser::ViewMode)KisConfig().presetChooserViewMode();
    
    
    QAction* action = menu->addAction(KIcon("view-preview"), i18n("Thumbnails"), this, SLOT(slotThumbnailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::THUMBNAIL);
    action->setActionGroup(actionGroup);

    action = menu->addAction(KIcon("view-list-details"), i18n("Details"), this, SLOT(slotDetailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::DETAIL);
    action->setActionGroup(actionGroup);
    
    m_d->uiWdgPaintOpPresets.viewModeButton->setIcon(KIcon("view-choose"));
    m_d->uiWdgPaintOpPresets.viewModeButton->setMenu(menu);
    m_d->uiWdgPaintOpPresets.viewModeButton->setPopupMode(QToolButton::InstantPopup);
    m_d->uiWdgPaintOpPresets.wdgPresetChooser->setViewMode(mode);
    
    connect(m_d->uiWdgPaintOpPresets.wdgPresetChooser, SIGNAL(resourceSelected(KoResource*)),
            this, SIGNAL(resourceSelected(KoResource*)));

    connect(m_d->uiWdgPaintOpPresets.searchBar, SIGNAL(textChanged(const QString&)),
            m_d->uiWdgPaintOpPresets.wdgPresetChooser, SLOT(searchTextChanged(const QString&)));

    connect(m_d->uiWdgPaintOpPresets.showAllCheckBox, SIGNAL(toggled(bool)),
            m_d->uiWdgPaintOpPresets.wdgPresetChooser, SLOT(setShowAll(bool)));
   
}

KisPaintOpPresetsChooserPopup::~KisPaintOpPresetsChooserPopup()
{
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
