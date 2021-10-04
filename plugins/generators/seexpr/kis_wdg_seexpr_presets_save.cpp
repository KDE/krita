/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KStandardGuiItem>
#include <QDate>
#include <QMessageBox>
#include <QTime>

#include <KisImportExportManager.h>
#include <KisResourceUserOperations.h>
#include <KoColorSpaceRegistry.h>
#include <KoFileDialog.h>
#include <KoResourceServerProvider.h>

#include <kis_fill_painter.h>
#include <kis_paint_device.h>

#include "KisResourceTypes.h"
#include "kis_wdg_seexpr_presets_save.h"
#include "resources/KisSeExprScript.h"

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

    KGuiItem::assign(buttons->button(QDialogButtonBox::Save), KStandardGuiItem::save());
    KGuiItem::assign(buttons->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());

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

        if (preset) {
            // If the id is -1, this is a new preset that has never been saved, so it cannot be a copy
            QString name = preset->name().replace("_", " ");
            if (preset->resourceId() > -1) {
                name.append(" ").append(i18n("Copy"));
            }
            newPresetNameTextField->setText(name);
        }

        newPresetNameTextField->setVisible(true);
        clearPresetThumbnailButton->setVisible(true);
        loadImageIntoThumbnailButton->setVisible(true);
    } else {
        setWindowTitle(i18n("Save SeExpr Preset"));

        if (preset) {
            newPresetNameTextField->setText(preset->name());
        }

        newPresetNameTextField->setVisible(false);
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
    KIS_ASSERT_RECOVER_RETURN(m_currentPreset);

    KisResourceModel model(ResourceType::SeExprScripts);
    QModelIndex idx = model.indexForResourceId(m_currentPreset->resourceId());
    bool r = true;

    if (idx.isValid() && !m_useNewPresetDialog) {
        // saving a preset that is replacing an existing one
        if (presetThumbnailWidget->pixmap()) {
            m_currentPreset->setImage(presetThumbnailWidget->pixmap()->toImage());
        }
        m_currentPreset->setScript(m_currentConfiguration->getString("script"));
        m_currentPreset->setValid(true);
        r = KisResourceUserOperations::updateResourceWithUserInput(this, &model, m_currentPreset);
    } else {
        // Saving a completely new preset
        // Clone the preset, otherwise the modifications will impact the existing resource
        KisSeExprScriptSP newPreset = m_currentPreset->clone().dynamicCast<KisSeExprScript>();

        if (newPreset) {
            newPreset->setName(newPresetNameTextField->text());
            newPreset->setFilename("");
            if (presetThumbnailWidget->pixmap()) {
                newPreset->setImage(presetThumbnailWidget->pixmap()->toImage());
            }
            newPreset->setScript(m_currentConfiguration->getString("script"));
            newPreset->setValid(true);

            r = KisResourceUserOperations::addResourceWithUserInput(this, &model, newPreset);

            // trying to get brush preset to load after it is created
            if (r) {
                emit resourceSelected(newPreset);
                m_currentPreset = newPreset;
            }
        }
    }

    close(); // we are done... so close the save brush dialog
}

void KisWdgSeExprPresetsSave::useNewPresetDialog(bool show)
{
    m_useNewPresetDialog = show;
}

#include "moc_kis_wdg_seexpr_presets_save.cpp"
