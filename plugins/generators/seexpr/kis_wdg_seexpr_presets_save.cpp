/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <KisImportExportManager.h>
#include <KoColorSpaceRegistry.h>
#include <KoFileDialog.h>
#include <KoResourceServerProvider.h>
#include <QDate>
#include <QTime>
#include <kis_fill_painter.h>
#include <kis_paint_device.h>

#include "kis_wdg_seexpr_presets_save.h"

KisWdgSeExprPresetsSave::KisWdgSeExprPresetsSave(QWidget *parent)
    : KisWdgSeExprSavePreset(parent)
    , m_currentPreset(nullptr)
{
    // we will default to reusing the previous preset thumbnail
    // have that checked by default, hide the other elements, and load the last preset image
    connect(loadExistingThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadExistingThumbnail()));
    connect(loadImageIntoThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadImageFromFile()));
    connect(loadRenderFromScriptButton, SIGNAL(clicked(bool)), this, SLOT(renderScriptToThumbnail()));
    connect(clearPresetThumbnailButton, SIGNAL(clicked(bool)), presetThumbnailWidget, SLOT(clear()));

    connect(buttons, SIGNAL(accepted()), this, SLOT(savePreset()));
    connect(buttons, SIGNAL(rejected()), this, SLOT(close()));
}

KisWdgSeExprPresetsSave::~KisWdgSeExprPresetsSave()
{
}

void KisWdgSeExprPresetsSave::setCurrentPreset(KisSeExprScriptSP resource)
{
    m_currentPreset = resource;
}

void KisWdgSeExprPresetsSave::setCurrentRenderConfiguration(KisFilterConfigurationSP config)
{
    m_currentConfiguration = config;
}

void KisWdgSeExprPresetsSave::showDialog()
{
    setModal(true);

    // set the name of the current preset area.
    KisSeExprScriptSP preset = m_currentPreset;

    // UI will look a bit different if we are saving a new preset
    if (m_useNewPresetDialog) {
        setWindowTitle(i18n("Save New SeExpr Preset"));
        newPresetNameTextField->setVisible(true);
        clearPresetThumbnailButton->setVisible(true);
        loadImageIntoThumbnailButton->setVisible(true);

        if (preset) {
            if (!preset->name().isEmpty()) {
                newPresetNameTextField->setText(preset->name().append(" ").append(i18n("Copy")));
            } else {
                newPresetNameTextField->clear();
            }
        }

    } else {
        setWindowTitle(i18n("Save SeExpr Preset"));
        newPresetNameTextField->setVisible(false);

        if (preset) {
            newPresetNameTextField->setText(preset->name());
        }
    }

    if (preset) {
        presetThumbnailWidget->setPixmap(QPixmap::fromImage(preset->image()));
    }

    show();
}

void KisWdgSeExprPresetsSave::loadImageFromFile()
{
    // create a dialog to retrieve an image file.
    KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    QString filename = dialog.filename(); // the filename() returns the entire path & file name, not just the file name

    if (!filename.isEmpty()) { // empty if "cancel" is pressed
        // take that file and load it into the thumbnail are
        const QImage imageToLoad(filename);

        presetThumbnailWidget->clear(); // clear the background in case our new image has transparency
        presetThumbnailWidget->setPixmap(QPixmap::fromImage(imageToLoad));
    }
}

void KisWdgSeExprPresetsSave::loadExistingThumbnail()
{
    presetThumbnailWidget->setPixmap(QPixmap::fromImage(m_currentPreset->image()));
}

void KisWdgSeExprPresetsSave::renderScriptToThumbnail()
{
    if (m_currentConfiguration) {
        // TODO add some sort of progress marker?
        KisDefaultBoundsBaseSP bounds(new KisWrapAroundBoundsWrapper(new KisDefaultBounds(), QRect(0, 0, 256, 256)));
        KisPaintDeviceSP src = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        src->setDefaultBounds(bounds);
        KisFillPainter fillPainter(src);
        fillPainter.fillRect(0, 0, 256, 256, m_currentConfiguration);

        QImage thumbnail = src->convertToQImage(nullptr, 0, 0, 256, 256);
        presetThumbnailWidget->setPixmap(QPixmap::fromImage(thumbnail));
    }
}

void KisWdgSeExprPresetsSave::savePreset()
{
    KisSeExprScriptSP curPreset = m_currentPreset;
    if (!curPreset) {
        return;
    }

    auto *rServer = KoResourceServerProvider::instance()->seExprScriptServer();
    QString saveLocation = rServer->saveLocation();

    // if we are saving a new preset, use what we type in for the input
    QString presetName = m_useNewPresetDialog ? newPresetNameTextField->text() : curPreset->name();
    // We don't want dots or spaces in the filenames
    QString presetFileName = presetName.replace(' ', '_').replace('.', '_');
    QString extension = curPreset->defaultFileExtension();

    if (m_useNewPresetDialog) {
        KisSeExprScriptSP newPreset = curPreset->clone().staticCast<KisSeExprScript>();
        if (!presetFileName.endsWith(extension)) {
            presetFileName.append(extension);
        }
        newPreset->setFilename(presetFileName);
        newPreset->setName(presetName);
        newPreset->setImage(presetThumbnailWidget->pixmap()->toImage());
        newPreset->setScript(m_currentConfiguration->getString("script"));
        newPreset->setDirty(false);
        newPreset->setValid(true);

        rServer->addResource(newPreset);

        // trying to get brush preset to load after it is created
        emit resourceSelected(newPreset);
    } else { // saving a preset that is replacing an existing one
        curPreset->setName(presetName);

        if (curPreset->image().isNull()) {
            curPreset->setImage(presetThumbnailWidget->pixmap()->toImage());
        }

        curPreset->setScript(m_currentConfiguration->getString("script"));

        rServer->updateResource(curPreset);
    }

    // HACK ALERT! the server does not notify the observers
    // automatically, so we need to call theupdate manually!
    // rServer->tagCategoryMembersChanged();

    close(); // we are done... so close the save brush dialog
}

void KisWdgSeExprPresetsSave::useNewPresetDialog(bool show)
{
    m_useNewPresetDialog = show;
}

#include "moc_kis_wdg_seexpr_presets_save.cpp"
