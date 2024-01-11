/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorSettings.h"
#include "ui_WdgWGSelectorSettings.h"

#include "WGConfigSelectorTypes.h"
#include "WGSelectorConfigGrid.h"
#include "WGColorSelectorDock.h"
#include "WGShadeLineEditor.h"

#include <QApplication>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QDialogButtonBox>
#include <QStringList>
#include <QToolButton>
#include <QPushButton>

WGColorSelectorSettings::WGColorSelectorSettings(QWidget *parent)
    : KisPreferenceSet(parent)
    , m_ui(new Ui::WGConfigWidget)
    , m_shadeLineGroup(new QButtonGroup(this))
{
    m_ui->setupUi(this);
    m_selectorConfigGrid = new WGSelectorConfigGrid;
    m_selectorConfigGrid->setConfigurations(WGSelectorConfigGrid::hueBasedConfigurations());
    m_ui->btnSelectorShape->setPopupWidget(m_selectorConfigGrid);
    connect(m_selectorConfigGrid, SIGNAL(sigConfigSelected(KisColorSelectorConfiguration)),
            SLOT(slotSetSelectorConfiguration(KisColorSelectorConfiguration)));
    connect(m_selectorConfigGrid, SIGNAL(sigConfigSelected(KisColorSelectorConfiguration)),
            m_ui->btnSelectorShape, SLOT(hidePopupWidget()));
    connect(m_ui->cmbColorModel, SIGNAL(currentIndexChanged(int)), SLOT(slotSetColorModel(int)));
    connect(m_ui->sbShadeLineCount, SIGNAL(valueChanged(int)), SLOT(slotSetShadeLineCount(int)));
    m_favoriteConfigGrid = new WGSelectorConfigGrid(0, true);
    m_favoriteConfigGrid->setConfigurations(WGSelectorConfigGrid::hueBasedConfigurations());
    m_ui->btnFavoriteSelectors->setPopupWidget(m_favoriteConfigGrid);

    m_shadeLineEditor = new WGShadeLineEditor(this);
    m_shadeLineEditor->hide();
    connect(m_shadeLineEditor, SIGNAL(sigEditorClosed(int)), SLOT(slotLineEdited(int)));

    slotColorSpaceSourceChanged(m_ui->cmbSelectionColorSpace->currentIndex());
    connect(m_ui->cmbSelectionColorSpace, SIGNAL(currentIndexChanged(int)), SLOT(slotColorSpaceSourceChanged(int)));

    m_shadeLineGroup->setExclusive(false);
    slotSetShadeLineCount(m_ui->sbShadeLineCount->value());
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    connect(m_shadeLineGroup, SIGNAL(idClicked(int)), SLOT(slotShowLineEditor(int)));
#else
    connect(m_shadeLineGroup, SIGNAL(buttonClicked(int)), SLOT(slotShowLineEditor(int)));
#endif
}

WGColorSelectorSettings::~WGColorSelectorSettings()
{
}

QString WGColorSelectorSettings::id()
{
    return stringID();
}

QString WGColorSelectorSettings::name()
{
    return QString(i18n("Wide Gamut Selector"));
}

QString WGColorSelectorSettings::header()
{
    return QString(i18n("Wide Gamut Color Selector Settings"));
}

QIcon WGColorSelectorSettings::icon()
{
    return QIcon();
}

QString WGColorSelectorSettings::stringID()
{
    return QString("WideGamutColorSelector");
}

void WGColorSelectorSettings::savePreferences() const
{
    WGConfig::Accessor cfg(false);
    cfg.set(WGConfig::selectorRenderMode,
            static_cast<KisVisualColorSelector::RenderMode>(m_ui->cmbSelectorRenderingMode->currentIndex()));
    cfg.set(WGConfig::rgbColorModel, static_cast<KisVisualColorModel::ColorModel>(
                m_ui->cmbColorModel->currentIndex() +  KisVisualColorModel::HSV));
    cfg.setColorSelectorConfiguration(m_selectorConfigGrid->currentConfiguration());
    // General
    cfg.set(WGConfig::quickSettingsEnabled, m_ui->grpQuickSettingsMenu->isChecked());
    cfg.setFavoriteConfigurations(m_favoriteConfigGrid->selectedConfigurations());
    cfg.set(WGConfig::colorSpaceSource,
            static_cast<WGColorSelectorDock::ColorSpaceSource>(m_ui->cmbSelectionColorSpace->currentIndex()));
    cfg.setCustomSelectionColorSpace(m_ui->wdgColorspace->currentColorSpace());
    cfg.set(WGConfig::proofToPaintingColors, m_ui->chkProofColors->isChecked());
    // Pop-ups
    cfg.set(WGConfig::popupSize, m_ui->sbPopupSize->value());
    cfg.set(WGConfig::popupPatches.orientation, m_ui->rbPopupHorizontal->isChecked() ? Qt::Horizontal : Qt::Vertical);
    cfg.set(WGConfig::popupPatches.patchSize, QSize(m_ui->sbPatchWidth->value(), m_ui->sbPatchHeight->value()));
    cfg.set(WGConfig::popupPatches.maxCount, m_ui->sbPopupMaxPatches->value());
    cfg.set(WGConfig::popupPatches.rows, m_ui->sbPopupPatchesRows->value());
    cfg.set(WGConfig::popupPatches.scrolling, static_cast<WGConfig::Scrolling>(m_ui->cbPopupScrolling->currentIndex()));
    // Shade Selector
    cfg.set(WGConfig::shadeSelectorUpdateOnExternalChanges, m_ui->chkShadeSelUpdateExternal->isChecked());
    cfg.set(WGConfig::shadeSelectorUpdateOnInteractionEnd, m_ui->chkShadeSelUpdateInteraction->isChecked());
    cfg.set(WGConfig::shadeSelectorUpdateOnRightClick, m_ui->chkShadeSelUpdateOnRightClick->isChecked());
    // we don't discard shade line configurations right away so we need to trim here
    QVector<WGConfig::ShadeLine> lineConfig = m_shadeLineConfig;
    lineConfig.resize(m_ui->sbShadeLineCount->value());
    cfg.setShadeSelectorLines(lineConfig);
    cfg.set(WGConfig::shadeSelectorLineHeight, m_ui->sbShadeLineHeight->value());
    // Color History
    cfg.set(WGConfig::colorHistoryEnabled, m_ui->historyGroupBox->isChecked());
    cfg.set(WGConfig::colorHistory.orientation, m_ui->rbHistoryHorizontal->isChecked() ? Qt::Horizontal : Qt::Vertical);
    cfg.set(WGConfig::colorHistory.patchSize, { m_ui->sbHistoryPatchWidth->value(), m_ui->sbHistoryPatchHeight->value() });
    cfg.set(WGConfig::colorHistory.maxCount, m_ui->sbHistoryMaxPatches->value());
    cfg.set(WGConfig::colorHistory.rows, m_ui->sbHistoryRows->value());
    cfg.set(WGConfig::colorHistory.scrolling, static_cast<WGConfig::Scrolling>(m_ui->cbScrolling->currentIndex()));
    cfg.set(WGConfig::colorHistoryShowClearButton, m_ui->ckHistoryClearButton->isChecked());
    // Common Colors (Colors from Image)
    cfg.set(WGConfig::commonColorsEnabled, m_ui->commonColorsGroupBox->isChecked());
    cfg.set(WGConfig::commonColors.orientation, m_ui->rbCommonColorsHorizontal->isChecked() ? Qt::Horizontal : Qt::Vertical);
    cfg.set(WGConfig::commonColors.patchSize, { m_ui->sbCommonColorsPatchWidth->value(),
                                                m_ui->sbCommonColorsPatchHeight->value() });
    cfg.set(WGConfig::commonColors.maxCount, m_ui->sbCommonColorsPatches->value());
    cfg.set(WGConfig::commonColors.rows, m_ui->sbCommonColorsRows->value());
    cfg.set(WGConfig::commonColors.scrolling, static_cast<WGConfig::Scrolling>(m_ui->cbCommonColorsScrolling->currentIndex()));
    cfg.set(WGConfig::commonColorsAutoUpdate, m_ui->ckCCAutoUpdate->isChecked());

    WGConfig::notifier()->notifyConfigChanged();
    WGConfig::notifier()->notifySelectorConfigChanged();
}

void WGColorSelectorSettings::loadPreferences()
{
    loadPreferencesImpl(false);
}

void WGColorSelectorSettings::loadDefaultPreferences()
{
    loadPreferencesImpl(true);
}

void WGColorSelectorSettings::loadPreferencesImpl(bool defaults)
{
    WGConfig::Accessor cfg;
    m_ui->cmbSelectorRenderingMode->setCurrentIndex(cfg.get(WGConfig::selectorRenderMode, defaults));
    m_ui->cmbColorModel->setCurrentIndex(cfg.get(WGConfig::rgbColorModel, defaults) - KisVisualColorModel::HSV);
    KisColorSelectorConfiguration selectorCfg = defaults ? cfg.defaultColorSelectorConfiguration
                                                             : cfg.colorSelectorConfiguration();
    m_selectorConfigGrid->setChecked(selectorCfg);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->generateIcon(selectorCfg, devicePixelRatioF()));
    // General
    m_ui->grpQuickSettingsMenu->setChecked(cfg.get(WGConfig::quickSettingsEnabled, defaults));
    QVector<KisColorSelectorConfiguration> favoriteConfigs = cfg.favoriteConfigurations(defaults);
    for (const KisColorSelectorConfiguration &fav : qAsConst(favoriteConfigs)) {
        m_favoriteConfigGrid->setChecked(fav);
    }
    m_ui->cmbSelectionColorSpace->setCurrentIndex(cfg.get(WGConfig::colorSpaceSource, defaults));
    m_ui->wdgColorspace->setCurrentColorSpace(cfg.customSelectionColorSpace(defaults));
    m_ui->chkProofColors->setChecked(cfg.get(WGConfig::proofToPaintingColors, defaults));
    // Pop-ups
    m_ui->sbPopupSize->setValue(cfg.get(WGConfig::popupSize, defaults));
    Qt::Orientation patchOrientation = cfg.get(WGConfig::popupPatches.orientation, defaults);
    if (patchOrientation == Qt::Horizontal) {
        m_ui->rbPopupHorizontal->setChecked(true);
    } else {
        m_ui->rbPopupVertical->setChecked(true);
    }
    QSize colorPatchSize = cfg.get(WGConfig::popupPatches.patchSize, defaults);
    m_ui->sbPatchWidth->setValue(colorPatchSize.width());
    m_ui->sbPatchHeight->setValue(colorPatchSize.height());
    m_ui->sbPopupMaxPatches->setValue(cfg.get(WGConfig::popupPatches.maxCount, defaults));
    m_ui->sbPopupPatchesRows->setValue(cfg.get(WGConfig::popupPatches.rows, defaults));
    m_ui->cbPopupScrolling->setCurrentIndex(static_cast<int>(cfg.get(WGConfig::popupPatches.scrolling, defaults)));
    // Shade Selector
    m_ui->chkShadeSelUpdateExternal->setChecked(cfg.get(WGConfig::shadeSelectorUpdateOnExternalChanges, defaults));
    m_ui->chkShadeSelUpdateInteraction->setChecked(cfg.get(WGConfig::shadeSelectorUpdateOnInteractionEnd, defaults));
    m_ui->chkShadeSelUpdateOnRightClick->setChecked(cfg.get(WGConfig::shadeSelectorUpdateOnRightClick, defaults));
    m_shadeLineConfig = cfg.shadeSelectorLines(defaults);
    // update shade lines we will re-use, the rest is handled by slotSetShadeLineCount()
    for (int i = 0, end = qMin(m_shadeLineButtons.size(), m_shadeLineConfig.size()); i < end; i++) {
        m_shadeLineButtons.at(i)->setIcon(m_shadeLineEditor->generateIcon(m_shadeLineConfig.at(i)));
    }
    m_ui->sbShadeLineCount->setValue(m_shadeLineConfig.size());
    m_ui->sbShadeLineHeight->setValue(cfg.get(WGConfig::shadeSelectorLineHeight, defaults));
    // Color History
    m_ui->historyGroupBox->setChecked(cfg.get(WGConfig::colorHistoryEnabled, defaults));
    patchOrientation = cfg.get(WGConfig::colorHistory.orientation, defaults);
    if (patchOrientation == Qt::Horizontal) {
        m_ui->rbHistoryHorizontal->setChecked(true);
    } else {
        m_ui->rbHistoryVertical->setChecked(true);
    }
    colorPatchSize = cfg.get(WGConfig::colorHistory.patchSize, defaults);
    m_ui->sbHistoryPatchWidth->setValue(colorPatchSize.width());
    m_ui->sbHistoryPatchHeight->setValue(colorPatchSize.height());
    m_ui->sbHistoryMaxPatches->setValue(cfg.get(WGConfig::colorHistory.maxCount));
    m_ui->sbHistoryRows->setValue(cfg.get(WGConfig::colorHistory.rows, defaults));
    m_ui->cbScrolling->setCurrentIndex(static_cast<int>(cfg.get(WGConfig::colorHistory.scrolling, defaults)));
    m_ui->ckHistoryClearButton->setChecked(cfg.get(WGConfig::colorHistoryShowClearButton, defaults));
    // Common Colors (Colors from Image)
    m_ui->commonColorsGroupBox->setChecked(cfg.get(WGConfig::commonColorsEnabled, defaults));
    patchOrientation = cfg.get(WGConfig::commonColors.orientation, defaults);
    if (patchOrientation == Qt::Horizontal) {
        m_ui->rbCommonColorsHorizontal->setChecked(true);
    } else {
        m_ui->rbCommonColorsVertical->setChecked(true);
    }
    colorPatchSize = cfg.get(WGConfig::commonColors.patchSize, defaults);
    m_ui->sbCommonColorsPatchWidth->setValue(colorPatchSize.width());
    m_ui->sbCommonColorsPatchHeight->setValue(colorPatchSize.height());
    m_ui->sbCommonColorsPatches->setValue(cfg.get(WGConfig::commonColors.maxCount, defaults));
    m_ui->sbCommonColorsRows->setValue(cfg.get(WGConfig::commonColors.rows, defaults));
    m_ui->cbCommonColorsScrolling->setCurrentIndex(static_cast<int>(cfg.get(WGConfig::commonColors.scrolling, defaults)));
    m_ui->ckCCAutoUpdate->setChecked(cfg.get(WGConfig::commonColorsAutoUpdate, defaults));
}

void WGColorSelectorSettings::slotSetSelectorConfiguration(const KisColorSelectorConfiguration &cfg)
{
    Q_UNUSED(cfg);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->currentIcon());
}

void WGColorSelectorSettings::slotSetColorModel(int index)
{
    KisVisualColorModel::ColorModel model;
    switch (index) {
    case 0:
    default:
        model = KisVisualColorModel::HSV;
        break;
    case 1:
        model = KisVisualColorModel::HSL;
        break;
    case 2:
        model = KisVisualColorModel::HSI;
        break;
    case 3:
        model = KisVisualColorModel::HSY;
    }
    m_selectorConfigGrid->setColorModel(model);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->currentIcon());
}

void WGColorSelectorSettings::slotColorSpaceSourceChanged(int index)
{
    WGColorSelectorDock::ColorSpaceSource csSource = static_cast<WGColorSelectorDock::ColorSpaceSource>(index);
    m_ui->wdgColorspace->setEnabled(csSource == WGColorSelectorDock::FixedColorSpace);
}

void WGColorSelectorSettings::slotSetShadeLineCount(int count)
{
    if (m_shadeLineConfig.size() < count) {
        m_shadeLineConfig.resize(count);
    }
    while (m_shadeLineButtons.size() < count) {
        QToolButton *lineButton = new QToolButton(this);
        lineButton->setIconSize(QSize(128, 10));
        // TODO: update icon
        lineButton->setIcon(m_shadeLineEditor->generateIcon(m_shadeLineConfig.at(m_shadeLineButtons.size())));
        m_shadeLineGroup->addButton(lineButton, m_shadeLineButtons.size());
        m_shadeLineButtons.append(lineButton);
        m_ui->shadeLineLayout->addWidget(lineButton);
    }
    while (m_shadeLineButtons.size() > count)
    {
        m_ui->shadeLineLayout->removeWidget(m_shadeLineButtons.last());
        delete m_shadeLineButtons.last();
        m_shadeLineButtons.removeLast();
    }
}

void WGColorSelectorSettings::slotShowLineEditor(int lineNum)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(lineNum >= 0 && lineNum < m_shadeLineConfig.size());

    m_shadeLineEditor->setConfiguration(m_shadeLineConfig[lineNum], lineNum);
    m_shadeLineEditor->show();

    QWidget *btn = m_shadeLineButtons.at(lineNum);
    QRect fitRect = kisGrowRect(QApplication::desktop()->availableGeometry(btn), -10);
    QRect popupRect = m_shadeLineEditor->rect();
    popupRect.moveTo(btn->mapToGlobal(QPoint()));
    popupRect = kisEnsureInRect(popupRect, fitRect);
    m_shadeLineEditor->move(popupRect.topLeft());
}

void WGColorSelectorSettings::slotLineEdited(int lineNum)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(lineNum >= 0 && lineNum < m_shadeLineConfig.size());

    m_shadeLineConfig[lineNum] = m_shadeLineEditor->configuration();
    m_shadeLineButtons[lineNum]->setIcon(m_shadeLineEditor->generateIcon(m_shadeLineConfig.at(lineNum)));
}


WGColorSelectorSettingsDialog::WGColorSelectorSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_widget(new WGColorSelectorSettings(this))
{
    QLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    m_widget->loadPreferences();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults,
                                                       Qt::Horizontal,
                                                       this);
    layout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), m_widget, SLOT(savePreferences()));
    connect(buttonBox, SIGNAL(accepted()), this,     SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this,     SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults),
            SIGNAL(clicked()),  m_widget, SLOT(loadDefaultPreferences()));
}
