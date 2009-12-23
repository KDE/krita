/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shichan.karachu@gmail.com>

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
#include <kis_paintop_settings_widget.h>

#include <KoToolManager.h>
#include <KoID.h>
#include <kglobal.h>
#include "kis_paintop_box.h"
#include "widgets/kis_preset_widget.h"
#include "KoID.h"
#include "KoInputDevice.h"
#include "kis_image.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_registry.h"
#include "kis_paintop_settings_widget.h"
#include "kis_shared_ptr.h"
#include "ko_favorite_resource_manager.h"

KisPaletteManager::KisPaletteManager(KoFavoriteResourceManager *manager, KisPaintopBox *paintOpBox)
    : QDialog(paintOpBox)
    , m_model(0)
    , m_brushList (0)
    , m_saveButton(0)
    , m_removeButton(0)
    , m_resourceManager(manager)
    , m_paintOpBox(paintOpBox)
    , m_currentBrushLabel(0)

{
    setWindowTitle("Krita - Palette Manager");
    m_currentBrushLabel = new QLabel ("");

    /*SETTING MODEL*/
    m_model = new QStringListModel();
    resetDataModel();

    /*LEFT COMPONENTS*/
    QFrame *HSeparator = new QFrame();
    HSeparator->setFrameStyle(QFrame::HLine | QFrame::Sunken);

    m_saveButton = new QPushButton (tr("&Save to Palette"));
    m_saveButton->setSizePolicy(QSizePolicy::Fixed , QSizePolicy::Fixed);

    /*LEFT LAYOUT*/
    QVBoxLayout *leftLayout = new QVBoxLayout ();
    leftLayout->addWidget(new QLabel (tr("Current Brush")));
    leftLayout->addWidget(HSeparator);
    leftLayout->addWidget(m_currentBrushLabel);
//    leftLayout->addWidget(paintOpSettings->widget());
    leftLayout->addStretch();
    leftLayout->addWidget(m_saveButton);

    /*CENTER COMPONENT : Divider*/
    QFrame *VSeparator = new QFrame();
    VSeparator->setFrameStyle(QFrame::VLine | QFrame::Sunken);

    /*RIGHT COMPONENTS*/
    m_brushList = new QListView;
    m_brushList->setModel(m_model);
    m_brushList->setWordWrap(true);
    m_removeButton = new QPushButton(tr("Remove Brush"));
    m_removeButton->setSizePolicy(QSizePolicy::Fixed , QSizePolicy::Fixed);
    m_removeButton->setEnabled(false);//set the button to center

    /*RIGHT LAYOUT*/
    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel(tr("Favorite Brushes")));
    rightLayout->addWidget(m_brushList);
    rightLayout->addWidget(m_removeButton);

    /*MAIN LAYOUT*/
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addLayout(leftLayout);
    mainLayout->addWidget(VSeparator);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);
    changeCurrentBrushLabel();

    /*SIGNALS AND SLOTS*/
    connect(m_brushList, SIGNAL(pressed(QModelIndex)), this, SLOT(slotEnableRemoveButton()));
    connect(m_removeButton, SIGNAL(clicked()), this, SLOT(slotDeleteBrush()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(slotAddBrush()));
}

void KisPaletteManager::changeCurrentBrushLabel()
{
    m_currentBrushLabel->setText(m_paintOpBox->currentPaintop().id());
    //m_paintOpBox->currentPaintopKoID().name() doesnt work properly.
}


void KisPaletteManager::resetDataModel()
{
    m_nameList = m_resourceManager->favoriteBrushesStringList();
    m_model->setStringList(m_nameList);
}

void KisPaletteManager::slotAddBrush()
{

    KisPaintOpPresetSP newBrush = m_paintOpBox->paintOpPresetSP();
    int pos = m_resourceManager->addFavoriteBrush(newBrush);

    QModelIndex index;

    if (pos == -2)
    {
        return; //favorite brush list is full!
    }
    else if (pos == -1) //favorite brush is successfully saved
    {
        m_nameList.append(m_paintOpBox->currentPaintop().id());
        m_model->setStringList(m_nameList);
        index = m_model->index(m_resourceManager->favoriteBrushesTotal()-1);
    }
    else //brush has already existed
    {
        index = m_model->index(pos);
    }

    m_brushList->setCurrentIndex(index);
    slotEnableRemoveButton();
}

void KisPaletteManager::slotEnableRemoveButton()
{
    QModelIndex index = m_brushList->currentIndex();
    m_removeButton->setEnabled(index.row() != -1);
}

void KisPaletteManager::slotDeleteBrush()
{
    int pos = m_brushList->currentIndex().row();
    m_nameList.removeAt(pos);
    m_resourceManager->removeFavoriteBrush(pos);
    m_model->setStringList(m_nameList);
    m_removeButton->setEnabled(false);
}

KisPaletteManager::~KisPaletteManager()
{
    delete m_model;
    delete m_brushList;
    delete m_saveButton;
    delete m_removeButton;
    m_resourceManager = 0;
    m_paintOpBox = 0;
}

#include "kis_palette_manager.moc"
