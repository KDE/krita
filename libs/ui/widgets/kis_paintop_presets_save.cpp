/* This file is part of the KDE project
 * Copyright (C) 2017 Scott Petrovic <scottpetrovic@gmail.com>
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

#include "widgets/kis_paintop_presets_save.h"
#include <QDebug>
#include <QDate>
#include <QTime>
#include <QVBoxLayout>
#include <QDialogButtonBox>


#include <KoFileDialog.h>
#include "KisImportExportManager.h"
#include "QDesktopServices"
#include "KisResourceServerProvider.h"
#include <kis_paintop_preset_icon_library.h>


KisPresetSaveWidget::KisPresetSaveWidget(QWidget * parent)
    : KisPaintOpPresetSaveDialog(parent)
{
    // this is setting the area we will "capture" for saving the brush preset. It can potentially be a different
    // area that the entire scratchpad
    brushPresetThumbnailWidget->setCutoutOverlayRect(QRect(0, 0, brushPresetThumbnailWidget->height(), brushPresetThumbnailWidget->width()));


    // we will default to reusing the previous preset thumbnail
    // have that checked by default, hide the other elements, and load the last preset image
    connect(clearBrushPresetThumbnailButton, SIGNAL(clicked(bool)), brushPresetThumbnailWidget, SLOT(fillDefault()));
    connect(loadImageIntoThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadImageFromFile()));

    connect(loadScratchPadThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadScratchpadThumbnail()));
    connect(loadExistingThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadExistingThumbnail()));
    connect(loadIconLibraryThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadImageFromLibrary()));

    connect(savePresetButton, SIGNAL(clicked(bool)), this, SLOT(savePreset()));
    connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
}

KisPresetSaveWidget::~KisPresetSaveWidget()
{

}

void KisPresetSaveWidget::scratchPadSetup(KisCanvasResourceProvider* resourceProvider)
{
    m_resourceProvider = resourceProvider;

    brushPresetThumbnailWidget->setupScratchPad(m_resourceProvider, Qt::white);
}

void KisPresetSaveWidget::showDialog()
{
    setModal(true);

    // set the name of the current brush preset area.
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    // UI will look a bit different if we are saving a new brush
    if (m_useNewBrushDialog) {
           setWindowTitle(i18n("Save New Brush Preset"));
           newBrushNameTexField->setVisible(true);
           clearBrushPresetThumbnailButton->setVisible(true);
           loadImageIntoThumbnailButton->setVisible(true);
           currentBrushNameLabel->setVisible(false);

           if (preset) {
               newBrushNameTexField->setText(preset->name().append(" ").append(i18n("Copy")));
           }


    } else {
        setWindowTitle(i18n("Save Brush Preset"));

        if (preset) {
            currentBrushNameLabel->setText(preset->name());
        }

        newBrushNameTexField->setVisible(false);
        currentBrushNameLabel->setVisible(true);
    }

     brushPresetThumbnailWidget->paintPresetImage();

    show();
}

void KisPresetSaveWidget::loadImageFromFile()
{
    // create a dialog to retrieve an image file.
    KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    QString filename = dialog.filename(); // the filename() returns the entire path & file name, not just the file name


    if (filename != "") { // empty if "cancel" is pressed
        // take that file and load it into the thumbnail are
        const QImage imageToLoad(filename);

        brushPresetThumbnailWidget->fillTransparent(); // clear the background in case our new image has transparency
        brushPresetThumbnailWidget->paintCustomImage(imageToLoad);
    }

}

void KisPresetSaveWidget::loadScratchpadThumbnail()
{
    brushPresetThumbnailWidget->paintCustomImage(scratchPadThumbnailArea);
}

void KisPresetSaveWidget::loadExistingThumbnail()
{
    brushPresetThumbnailWidget->paintPresetImage();
}

void KisPresetSaveWidget::loadImageFromLibrary()
{
    //add dialog code here.
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle(i18n("Preset Icon Library"));
    QVBoxLayout *layout = new QVBoxLayout();
    dlg->setLayout(layout);
    KisPaintopPresetIconLibrary *libWidget = new KisPaintopPresetIconLibrary(dlg);
    layout->addWidget(libWidget);
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, dlg);
    connect(buttons, SIGNAL(accepted()), dlg, SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), dlg, SLOT(reject()));
    layout->addWidget(buttons);

    //if dialog accepted, get image.
    if (dlg->exec()==QDialog::Accepted) {

        QImage presetImage = libWidget->getImage();
        brushPresetThumbnailWidget->paintCustomImage(presetImage);
    }
}

void KisPresetSaveWidget::setFavoriteResourceManager(KisFavoriteResourceManager * favManager)
{
    m_favoriteResourceManager = favManager;
}

void KisPresetSaveWidget::savePreset()
{
    KisPaintOpPresetSP curPreset = m_resourceProvider->currentPreset();
    if (!curPreset)
        return;

    m_favoriteResourceManager->setBlockUpdates(true);

    KisPaintOpPresetSP oldPreset = curPreset->clone(); // tags are not cloned with this
    oldPreset->load();
    KisPaintOpPresetResourceServer * rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QString saveLocation = rServer->saveLocation();

    // if we are saving a new brush, use what we type in for the input
    QString presetName = m_useNewBrushDialog ? newBrushNameTexField->text() : curPreset->name();
    QString currentPresetFileName = saveLocation + presetName.replace(" ", "_") + curPreset->defaultFileExtension();
    bool isSavingOverExistingPreset = rServer->resourceByName(presetName);

    // make a back up of the existing preset if we are saving over it
    if (isSavingOverExistingPreset) {
        QString currentDate = QDate::currentDate().toString(Qt::ISODate);
        QString currentTime = QTime::currentTime().toString(Qt::ISODate).remove(QChar(':'));
        QString presetFilename = saveLocation + presetName.replace(" ", "_") + "_backup_" + currentDate + "-" + currentTime + oldPreset->defaultFileExtension();
        oldPreset->setFilename(presetFilename);
        oldPreset->setName(presetName);
        oldPreset->setDirty(false);
        oldPreset->setValid(true);

        // add backup resource to the blacklist
        rServer->addResource(oldPreset);
        rServer->removeResourceAndBlacklist(oldPreset.data());


        QStringList tags;
        tags = rServer->assignedTagsList(curPreset.data());
        Q_FOREACH (const QString & tag, tags) {
            rServer->addTag(oldPreset.data(), tag);
        }
    }


    if (m_useNewBrushDialog) {
        KisPaintOpPresetSP newPreset = curPreset->clone();
        newPreset->setFilename(currentPresetFileName);
        newPreset->setName(presetName);
        newPreset->setImage(brushPresetThumbnailWidget->cutoutOverlay());
        newPreset->setDirty(false);
        newPreset->setValid(true);


        // keep tags if we are saving over existing preset
        if (isSavingOverExistingPreset) {
            QStringList tags;
            tags = rServer->assignedTagsList(curPreset.data());
            Q_FOREACH (const QString & tag, tags) {
                rServer->addTag(newPreset.data(), tag);
            }
        }

        rServer->addResource(newPreset);

        // trying to get brush preset to load after it is created
        emit resourceSelected(newPreset.data());
    }
    else { // saving a preset that is replacing an existing one

        if (curPreset->filename().contains(saveLocation) == false || curPreset->filename().contains(presetName) == false) {
            rServer->removeResourceAndBlacklist(curPreset.data());
            curPreset->setFilename(currentPresetFileName);
            curPreset->setName(presetName);
        }

        if (!rServer->resourceByFilename(curPreset->filename())){
            //this is necessary so that we can get the preset afterwards.
            rServer->addResource(curPreset, false, false);
            rServer->removeFromBlacklist(curPreset.data());
        }
        if (curPreset->image().isNull()) {
            curPreset->setImage(brushPresetThumbnailWidget->cutoutOverlay());
        }

        // we should not load() the brush right after saving because it will reset all our saved
        // eraser size and opacity values
        curPreset->save();
    }


    // HACK ALERT! the server does not notify the observers
    // automatically, so we need to call theupdate manually!
    rServer->tagCategoryMembersChanged();

    m_favoriteResourceManager->setBlockUpdates(false);

    close(); // we are done... so close the save brush dialog

}



void KisPresetSaveWidget::saveScratchPadThumbnailArea(QImage image)
{
    scratchPadThumbnailArea = image;
}


void KisPresetSaveWidget::useNewBrushDialog(bool show)
{
    m_useNewBrushDialog = show;
}


#include "moc_kis_paintop_presets_save.cpp"
