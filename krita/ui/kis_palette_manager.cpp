/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>
   Copyright 2011 Sven Langkamp <sven.langkamp@gmail.com>
   Copyright 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Known issues:
   1. Current brush settings cannot be displayed yet due to a bug in Krita.
      Open paintopBox, open the Palette Manager, open paintopBox again.
      Notice that the setting widget is gone.
      Opening the Palette Manager will crash the program.
   2. KoID.name() doesn't work properly (line 135, 157)
      Open the Palette Manager.
      Save currently active brush, close manager.
      Open the Palette Manager, the brush name changes to 'default'.
*/

#include "kis_palette_manager.h"
#include <QtGui>
#include "klocale.h"
#include <kmenu.h>
#include <KoIcon.h>
#include <kis_paintop_settings_widget.h>

#include <KoToolManager.h>
#include <KoID.h>
#include <kglobal.h>
#include "kis_paintop_box.h"
#include "KoID.h"
#include "KoInputDevice.h"
#include "kis_image.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_registry.h"
#include "kis_paintop_settings_widget.h"
#include "kis_shared_ptr.h"
#include "ko_favorite_resource_manager.h"
#include "KoResourceModel.h"
#include "KoResourceItemView.h"
#include "kis_resource_server_provider.h"
#include "KoResourceServerAdapter.h"
#include "kis_paintop_preset.h"
#include "kis_preset_chooser.h"
#include <kconfiggroup.h>

KisPaletteManager::KisPaletteManager(KoFavoriteResourceManager *manager, KisPaintopBox *paintOpBox)
    : QDialog(paintOpBox)
    , m_saveButton(0)
    , m_removeButton(0)
    , m_resourceManager(manager)
    , m_paintOpBox(paintOpBox)
{
    setWindowTitle(i18n("Palette Manager"));

    m_allPresetsView = new KisPresetChooser(this);
    m_allPresetsView->showButtons(false);
    m_allPresetsView->showTaggingBar(false,false);
    m_allPresetsView->setPresetFilter(KoID("dummy",""));
    m_allPresetsView->setShowAll(true);

    m_palettePresetsView = new KisPresetChooser(this);
    m_palettePresetsView->showButtons(false);
    m_palettePresetsView->showTaggingBar(false,false);
    m_palettePresetsView->setPresetFilter(KoID("dummy",""));
    m_palettePresetsView->setShowAll(true);

    /*LEFT COMPONENTS*/
    QFrame *HSeparator = new QFrame();
    HSeparator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    m_saveButton = new QPushButton (i18n("Add to Palette"));
    m_saveButton->setSizePolicy(QSizePolicy::Fixed , QSizePolicy::Fixed);
    m_saveButton->setEnabled(false);

    /*LEFT LAYOUT*/
    QVBoxLayout *leftLayout = new QVBoxLayout ();
    leftLayout->addWidget(new QLabel(i18n("Available Presets")));
    leftLayout->addWidget(m_allPresetsView);
    leftLayout->addWidget(m_saveButton);

    m_allPresetsView->updateViewSettings();

    /*CENTER COMPONENT : Divider*/
    QFrame *VSeparator = new QFrame();
    VSeparator->setFrameStyle(QFrame::VLine | QFrame::Sunken);

    /*RIGHT COMPONENTS*/
    m_removeButton = new QPushButton(i18n("Remove Preset"));
    m_removeButton->setSizePolicy(QSizePolicy::Fixed , QSizePolicy::Fixed);
    m_removeButton->setEnabled(false);//set the button to center

    /*RIGHT LAYOUT*/
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel(i18n("Favorite Presets")));
    rightLayout->addWidget(m_palettePresetsView);

    /*RIGHT BOTTOM LAYOUT*/
    QHBoxLayout *rightBottomLayout = new QHBoxLayout();
    rightBottomLayout->addWidget(m_removeButton);
    rightBottomLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
    QToolButton* viewButton = new QToolButton();
    viewButton->setPopupMode(QToolButton::InstantPopup);
    rightBottomLayout->addWidget(viewButton);
    rightLayout->addLayout(rightBottomLayout);

    KMenu* menu = new KMenu(this);

    QActionGroup *actionGroup = new QActionGroup(this);

    KConfigGroup group(KGlobal::config(), "favoriteList");
    KisPresetChooser::ViewMode mode = (KisPresetChooser::ViewMode)group.readEntry("presetChooserViewMode", 0);
    m_allPresetsView->setViewMode(mode);
    m_palettePresetsView->setViewMode(mode);

    QAction* action = menu->addAction(koIcon("view-preview"), i18n("Thumbnails"), this, SLOT(slotThumbnailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::THUMBNAIL);
    action->setActionGroup(actionGroup);

    action = menu->addAction(koIcon("view-list-details"), i18n("Details"), this, SLOT(slotDetailMode()));
    action->setCheckable(true);
    action->setChecked(mode == KisPresetChooser::DETAIL);
    action->setActionGroup(actionGroup);

    viewButton->setIcon(koIcon("view-choose"));
    viewButton->setMenu(menu);

    /*MAIN LAYOUT*/
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(VSeparator);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);

    /*SIGNALS AND SLOTS*/
    connect(m_allPresetsView, SIGNAL(resourceSelected(KoResource*)), this, SLOT(slotUpdateAddButton()) );
    connect(m_palettePresetsView, SIGNAL(resourceSelected(KoResource*)), this, SLOT(slotEnableRemoveButton()) );
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(slotDeleteBrush()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(slotAddBrush()));

    updatePaletteView();
}

KisPaletteManager::~KisPaletteManager()
{
    m_resourceManager = 0;
    m_paintOpBox = 0;
}

void KisPaletteManager::slotAddBrush()
{

    KisPaintOpPreset* newPreset = static_cast<KisPaintOpPreset*>(m_allPresetsView->currentResource());
    int pos = m_resourceManager->addFavoritePreset(newPreset->name());

    QModelIndex index;

    if (pos == -2)
    {
        return; //favorite brush list is full!
    }
    else if (pos == -1) //favorite brush is successfully saved
    {
        updatePaletteView();
    }
}

void KisPaletteManager::slotUpdateAddButton()
{
    KoResource * resource = m_allPresetsView-> currentResource();
    if( resource ) {
        m_saveButton->setEnabled(true);
    } else {
        m_saveButton->setEnabled(false);
    }
}

void KisPaletteManager::slotEnableRemoveButton()
{
    KoResource * resource = m_allPresetsView->currentResource();
    m_removeButton->setEnabled(resource != 0);
}

void KisPaletteManager::slotDeleteBrush()
{
    KoResource * resource = m_palettePresetsView->currentResource();
    m_resourceManager->removeFavoritePreset(resource->name());
    m_removeButton->setEnabled(false);
    updatePaletteView();
}

void KisPaletteManager::showEvent(QShowEvent* e)
{
    QDialog::showEvent(e);
    m_allPresetsView->updateViewSettings();
    m_palettePresetsView->updateViewSettings();
}

void KisPaletteManager::updatePaletteView()
{
    m_palettePresetsView->setFilteredNames(m_resourceManager->favoritePresetList());
}

void KisPaletteManager::slotThumbnailMode()
{
    KConfigGroup group(KGlobal::config(), "favoriteList");
    group.writeEntry("presetChooserViewMode", (int)KisPresetChooser::THUMBNAIL);

    m_allPresetsView->setViewMode(KisPresetChooser::THUMBNAIL);
    m_palettePresetsView->setViewMode(KisPresetChooser::THUMBNAIL);
}

void KisPaletteManager::slotDetailMode()
{
    KConfigGroup group(KGlobal::config(), "favoriteList");
    group.writeEntry("presetChooserViewMode", (int)KisPresetChooser::DETAIL);

    m_allPresetsView->setViewMode(KisPresetChooser::DETAIL);
    m_palettePresetsView->setViewMode(KisPresetChooser::DETAIL);
}

#include "kis_palette_manager.moc"
