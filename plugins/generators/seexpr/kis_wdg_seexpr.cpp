/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KisDialogStateSaver.h>
#include <KisGlobalResourcesInterface.h>
#include <KoColor.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KSeExprUI/ErrorMessages.h>
#include <filter/kis_filter_configuration.h>
#include <kis_icon.h>
#include <kis_config.h>

#include "SeExprExpressionContext.h"
#include "generator.h"
#include "kis_wdg_seexpr.h"
#include "ui_wdgseexpr.h"

KisWdgSeExpr::KisWdgSeExpr(QWidget *parent)
    : KisConfigWidget(parent)
    , updateCompressor(1000, KisSignalCompressor::Mode::POSTPONE)
    , m_currentPreset(new KisSeExprScript(i18n("Untitled")))
    , m_saveDialog(new KisWdgSeExprPresetsSave(this))
    , m_isCreatingPresetFromScratch(true)
{
    m_widget = new Ui_WdgSeExpr();
    m_widget->setupUi(this);
    m_widget->txtEditor->setControlCollectionWidget(m_widget->wdgControls);

    m_widget->renameBrushPresetButton->setIcon(KisIconUtils::loadIcon("document-edit"));

    m_widget->reloadPresetButton->setIcon(KisIconUtils::loadIcon("reload-preset-16"));
    m_widget->reloadPresetButton->setToolTip(i18n("Reload the preset"));
    m_widget->dirtyPresetIndicatorButton->setIcon(KisIconUtils::loadIcon("warning"));
    m_widget->dirtyPresetIndicatorButton->setToolTip(i18n("The settings for this preset have changed from their default."));

    KisDialogStateSaver::restoreState(m_widget->txtEditor, "krita/generators/seexpr");
    // Manually restore SeExpr state. KisDialogStateSaver uses setPlainText, not text itself
    m_widget->txtEditor->setExpr(m_widget->txtEditor->exprTe->toPlainText());

    m_widget->txtEditor->registerExtraVariable("$u", i18nc("SeExpr variable", "Normalized X axis coordinate of the image from its top-left corner"));
    m_widget->txtEditor->registerExtraVariable("$v", i18nc("SeExpr variable", "Normalized Y axis coordinate of the image from its top-left corner"));
    m_widget->txtEditor->registerExtraVariable("$w", i18nc("SeExpr variable", "Image width"));
    m_widget->txtEditor->registerExtraVariable("$h", i18nc("SeExpr variable", "Image height"));

    m_widget->txtEditor->updateCompleter();

    m_widget->txtEditor->exprTe->setFont(QFontDatabase().systemFont(QFontDatabase::FixedFont));

    connect(m_widget->scriptSelectorWidget, SIGNAL(resourceSelected(KoResourceSP)), this, SLOT(slotResourceSelected(KoResourceSP)));
    connect(m_saveDialog, SIGNAL(resourceSelected(KoResourceSP)), this, SLOT(slotResourceSaved(KoResourceSP)));

    connect(m_widget->renameBrushPresetButton, SIGNAL(clicked(bool)),
            this, SLOT(slotRenamePresetActivated()));
    connect(m_widget->cancelBrushNameUpdateButton, SIGNAL(clicked(bool)),
            this, SLOT(slotRenamePresetDeactivated()));
    connect(m_widget->updateBrushNameButton, SIGNAL(clicked(bool)),
            this, SLOT(slotSaveRenameCurrentPreset()));
    connect(m_widget->renameBrushNameTextField, SIGNAL(returnPressed()),
            this, SLOT(slotSaveRenameCurrentPreset()));

    connect(m_widget->saveBrushPresetButton, SIGNAL(clicked()),
        this, SLOT(slotSaveBrushPreset()));
    connect(m_widget->saveNewBrushPresetButton, SIGNAL(clicked()),
        this, SLOT(slotSaveNewBrushPreset()));

    connect(m_widget->reloadPresetButton, SIGNAL(clicked()),
        this, SLOT(slotReloadPresetClicked()));

    connect(m_widget->txtEditor, SIGNAL(apply()),
            &updateCompressor, SLOT(start()));
    connect(m_widget->txtEditor, SIGNAL(preview()),
            &updateCompressor, SLOT(start()));

    connect(&updateCompressor, SIGNAL(timeout()), this, SLOT(isValid()));

    togglePresetRenameUIActive(false); // reset the UI state of renaming a preset if we are changing presets
    slotUpdatePresetSettings();        // disable everything until a preset is selected

    m_widget->splitter->restoreState(KisConfig(true).readEntry("seExpr/splitLayoutState", QByteArray())); // restore splitter state
    m_widget->tabWidget->setCurrentIndex(KisConfig(true).readEntry("seExpr/selectedTab",  -1));               // save currently selected tab
}

KisWdgSeExpr::~KisWdgSeExpr()
{
    KisDialogStateSaver::saveState(m_widget->txtEditor, "krita/generators/seexpr");
    KisConfig(false).writeEntry("seExpr/splitLayoutState", m_widget->splitter->saveState()); // save splitter state
    KisConfig(false).writeEntry("seExpr/selectedTab", m_widget->tabWidget->currentIndex()); // save currently selected tab

    delete m_saveDialog;
    delete m_widget;
}

inline const Ui_WdgSeExpr *KisWdgSeExpr::widget() const
{
    return m_widget;
}

void KisWdgSeExpr::setConfiguration(const KisPropertiesConfigurationSP config)
{
    auto rserver = KoResourceServerProvider::instance()->seExprScriptServer();
    auto name = config->getString("seexpr", "Disney_noisecolor2");
    auto pattern = rserver->resource("", "", name);
    if (pattern) {
        m_widget->scriptSelectorWidget->setCurrentScript(pattern);
    }

    QString script = config->getString("script");

    if (!script.isNull()) {
        m_widget->txtEditor->setExpr(script, true);
    }
}

KisPropertiesConfigurationSP KisWdgSeExpr::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("seexpr", 1, KisGlobalResourcesInterface::instance());

    if (m_widget->scriptSelectorWidget->currentResource()) {
        QVariant v;
        v.setValue(m_widget->scriptSelectorWidget->currentResource()->name());
        config->setProperty("pattern", v);
    }
    config->setProperty("script", QVariant(m_widget->txtEditor->getExpr()));

    return config;
}

void KisWdgSeExpr::slotResourceSaved(KoResourceSP resource)
{
    if (resource) {
        m_widget->scriptSelectorWidget->setCurrentScript(resource);
        slotResourceSelected(resource);
    }
}

void KisWdgSeExpr::slotResourceSelected(KoResourceSP resource)
{
    if (resource) {
        // ALWAYS have a manageable copy of the preset
        // this is required for detecting dirty presets and reloading
        m_currentPreset = resource->clone().staticCast<KisSeExprScript>();

        m_isCreatingPresetFromScratch = false;

        m_widget->txtEditor->setExpr(m_currentPreset->script(), true);

        QString formattedBrushName = m_currentPreset->name().replace("_", " ");
        m_widget->currentBrushNameLabel->setText(formattedBrushName);
        m_widget->renameBrushNameTextField->setText(m_currentPreset->name());
        // get the preset image and pop it into the thumbnail area on the top of the brush editor
        QSize thumbSize = QSize(55, 55)*devicePixelRatioF();
        QPixmap thumbnail = QPixmap::fromImage(m_currentPreset->image().scaled(thumbSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        thumbnail.setDevicePixelRatio(devicePixelRatioF());
        m_widget->presetThumbnailicon->setPixmap(thumbnail);

        togglePresetRenameUIActive(false); // reset the UI state of renaming a brush if we are changing brush presets
        slotUpdatePresetSettings();        // check to see if the dirty preset icon needs to be shown

        updateCompressor.start();
    }
}

void KisWdgSeExpr::slotRenamePresetActivated()
{
    togglePresetRenameUIActive(true);
}

void KisWdgSeExpr::slotRenamePresetDeactivated()
{
    togglePresetRenameUIActive(false);
}

void KisWdgSeExpr::togglePresetRenameUIActive(bool isRenaming)
{
    // This function doesn't really do anything except get the UI in a state to rename a brush preset
    m_widget->renameBrushNameTextField->setVisible(isRenaming);
    m_widget->updateBrushNameButton->setVisible(isRenaming);
    m_widget->cancelBrushNameUpdateButton->setVisible(isRenaming);

    // hide these below areas while renaming
    m_widget->currentBrushNameLabel->setVisible(!isRenaming);
    m_widget->renameBrushPresetButton->setVisible(!isRenaming);
    m_widget->saveBrushPresetButton->setEnabled(!isRenaming);
    m_widget->saveBrushPresetButton->setVisible(!isRenaming);
    m_widget->saveNewBrushPresetButton->setEnabled(!isRenaming);
    m_widget->saveNewBrushPresetButton->setVisible(!isRenaming);
}

void KisWdgSeExpr::slotSaveRenameCurrentPreset()
{
    slotReloadPresetClicked();

    KisSeExprScriptSP curPreset = m_currentPreset;

    if (!curPreset)
        return;

    KoResourceServer<KisSeExprScript> *rServer = KoResourceServerProvider::instance()->seExprScriptServer();
    QString saveLocation = rServer->saveLocation();

    QString originalPresetName = curPreset->name();
    QString renamedPresetName = m_widget->renameBrushNameTextField->text();
    QString originalPresetPathAndFile = saveLocation + originalPresetName + curPreset->defaultFileExtension();
    QString renamedPresetPathAndFile = saveLocation + renamedPresetName + curPreset->defaultFileExtension();

    KisSeExprScriptSP newPreset = curPreset->clone().staticCast<KisSeExprScript>();
    newPreset->setFilename(renamedPresetPathAndFile); // this also contains the path
    newPreset->setName(renamedPresetName);
    newPreset->setImage(curPreset->image()); // use existing thumbnail (might not need to do this)
    newPreset->setDirty(false);
    rServer->updateResource(newPreset);

    slotResourceSelected(newPreset); // refresh and select our freshly renamed resource

    togglePresetRenameUIActive(false); // this returns the UI to its original state after saving

    slotUpdatePresetSettings(); // update visibility of dirty preset and icon
}

void KisWdgSeExpr::slotUpdatePresetSettings()
{
    // hide options on UI if we are creating a brush preset from scratch to prevent confusion
    if (m_isCreatingPresetFromScratch) {
        m_widget->presetThumbnailicon->setVisible(false);
        m_widget->dirtyPresetIndicatorButton->setVisible(false);
        m_widget->reloadPresetButton->setVisible(false);
        m_widget->saveBrushPresetButton->setVisible(false);
        m_widget->saveNewBrushPresetButton->setEnabled(false);
        m_widget->renameBrushPresetButton->setVisible(false);
    } else {
        // In SeExpr's case, there is never a default preset -- amyspark
        if (!m_currentPreset) {
            return;
        }

        bool isPresetDirty = m_currentPreset->isDirty();

        m_widget->presetThumbnailicon->setVisible(true);
        // don't need to reload or overwrite a clean preset
        m_widget->dirtyPresetIndicatorButton->setVisible(isPresetDirty);
        m_widget->reloadPresetButton->setVisible(isPresetDirty);
        m_widget->saveBrushPresetButton->setEnabled(isPresetDirty);
        m_widget->saveNewBrushPresetButton->setEnabled(true);
        m_widget->renameBrushPresetButton->setVisible(true);
    }
}

void KisWdgSeExpr::slotSaveBrushPreset()
{
    KisFilterConfigurationSP currentConfiguration = static_cast<KisFilterConfiguration *>(configuration().data());

    m_saveDialog->useNewPresetDialog(false); // this mostly just makes sure we keep the existing brush preset name when saving
    m_saveDialog->setCurrentPreset(m_currentPreset);
    m_saveDialog->setCurrentRenderConfiguration(currentConfiguration);
    m_saveDialog->loadExistingThumbnail(); // This makes sure we use the existing preset icon when updating the existing brush preset
    m_saveDialog->savePreset();

    // refresh the view settings so the brush doesn't appear dirty
    slotUpdatePresetSettings();
}

void KisWdgSeExpr::slotSaveNewBrushPreset()
{
    KisFilterConfigurationSP currentConfiguration = static_cast<KisFilterConfiguration *>(configuration().data());

    m_saveDialog->useNewPresetDialog(true);
    m_saveDialog->setCurrentPreset(m_currentPreset);
    m_saveDialog->setCurrentRenderConfiguration(currentConfiguration);
    m_saveDialog->showDialog();
}

void KisWdgSeExpr::slotReloadPresetClicked()
{
    auto *rserver = KoResourceServerProvider::instance()->seExprScriptServer();
    auto preset = rserver->resource("", "", m_currentPreset->name());
    if (preset) {
        preset->load(KisGlobalResourcesInterface::instance());

        KIS_ASSERT(!preset->isDirty());

        slotResourceSelected(preset);
    }
}

void KisWdgSeExpr::isValid()
{
    QString script = m_widget->txtEditor->getExpr();
    SeExprExpressionContext expression(script);

    expression.setDesiredReturnType(KSeExpr::ExprType().FP(3));

    expression.m_vars["u"] = new SeExprVariable();
    expression.m_vars["v"] = new SeExprVariable();
    expression.m_vars["w"] = new SeExprVariable();
    expression.m_vars["h"] = new SeExprVariable();

    m_widget->txtEditor->clearErrors();

    if (!expression.isValid()) {
        auto errors = expression.getErrors();

        for (auto occurrence : errors) {
            QString message = ErrorMessages::message(occurrence.error);
            for (auto arg : occurrence.ids) {
                message = message.arg(QString::fromStdString(arg));
            }
            m_widget->txtEditor->addError(occurrence.startPos, occurrence.endPos, message);
        }

        m_widget->saveBrushPresetButton->setEnabled(false);
        m_widget->saveNewBrushPresetButton->setEnabled(false);
    }
    // Should not happen now, but I've left it for completeness's sake
    else if (!expression.returnType().isFP(3)) {
        QString type = QString::fromStdString(expression.returnType().toString());
        m_widget->txtEditor->addError(1, 1, tr2i18n("Expected this script to output color, got '%1'").arg(type));

        m_widget->saveBrushPresetButton->setEnabled(false);
        m_widget->saveNewBrushPresetButton->setEnabled(false);
    } else {
        m_widget->txtEditor->clearErrors();
        emit sigConfigurationUpdated();

        if (m_currentPreset) {
            m_widget->saveBrushPresetButton->setEnabled(true);
            if (m_currentPreset->script() != m_widget->txtEditor->getExpr()) {
                m_currentPreset->setScript(m_widget->txtEditor->getExpr());
                m_currentPreset->setDirty(true);
            }
            slotUpdatePresetSettings();
        } else {
            m_widget->saveNewBrushPresetButton->setEnabled(true);
        }
    }
}
