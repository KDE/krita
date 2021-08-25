/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorSettings.h"
#include "ui_WdgWGSelectorSettings.h"

#include "WGConfig.h"
#include "WGSelectorConfigGrid.h"
#include "WGShadeLineEditor.h"

#include "kis_config.h"

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

    m_shadeLineGroup->setExclusive(false);
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
    WGConfig cfg(false);
    cfg.writeEntry("renderMode", m_ui->cmbSelectorRenderingMode->currentIndex());
    cfg.writeEntry("rgbColorModel", m_ui->cmbColorModel->currentIndex() +  KisVisualColorModel::HSV);
    cfg.setColorSelectorConfiguration(m_selectorConfigGrid->currentConfiguration());
    cfg.setQuickSettingsEnabled(m_ui->grpQuickSettingsMenu->isChecked());
    cfg.setFavoriteConfigurations(m_favoriteConfigGrid->selectedConfigurations());
    // Pop-ups
    cfg.setPopupSize(m_ui->sbPopupSize->value());
    cfg.setPopupColorPatchOrientation(m_ui->rbPopupHorizontal->isChecked() ? Qt::Horizontal : Qt::Vertical);
    cfg.setPopupColorPatchSize(QSize(m_ui->sbPatchWidth->value(), m_ui->sbPatchHeight->value()));
    // Shade Selector
    cfg.setShadeSelectorUpdateOnExternalChanges(m_ui->chkShadeSelUpdateExternal->isChecked());
    cfg.setShadeSelectorUpdateOnInteractionEnd(m_ui->chkShadeSelUpdateInteraction->isChecked());
    cfg.setShadeSelectorUpdateOnRightClick(m_ui->chkShadeSelUpdateOnRightClick->isChecked());
    // we don't discard shade line configurations right away so we need to trim here
    QVector<WGConfig::ShadeLine> lineConfig = m_shadeLineConfig;
    lineConfig.resize(m_ui->sbShadeLineCount->value());
    cfg.setShadeSelectorLines(lineConfig);
    cfg.setShadeSelectorLineHeight(m_ui->sbShadeLineHeight->value());

    WGConfig::notifier()->notifyConfigChanged();
    WGConfig::notifier()->notifySelectorConfigChanged();
}

void WGColorSelectorSettings::loadPreferences()
{
    WGConfig cfg;
    m_ui->cmbSelectorRenderingMode->setCurrentIndex(cfg.readEntry("renderMode", 1));
    m_ui->cmbColorModel->setCurrentIndex(cfg.readEntry("rgbColorModel", 2) - KisVisualColorModel::HSV);
    KisColorSelectorConfiguration selectorCfg = cfg.colorSelectorConfiguration();
    m_selectorConfigGrid->setChecked(selectorCfg);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->generateIcon(selectorCfg, devicePixelRatioF()));
    m_ui->grpQuickSettingsMenu->setChecked(cfg.quickSettingsEnabled());
    QVector<KisColorSelectorConfiguration> favoriteConfigs = cfg.favoriteConfigurations();
    for (const KisColorSelectorConfiguration &fav: favoriteConfigs) {
        m_favoriteConfigGrid->setChecked(fav);
    }
    // Pop-ups
    m_ui->sbPopupSize->setValue(cfg.popupSize());
    Qt::Orientation patchOrientation = cfg.popupColorPatchOrientation();
    if (patchOrientation == Qt::Horizontal) {
        m_ui->rbPopupHorizontal->setChecked(true);
    } else {
        m_ui->rbPopupVertical->setChecked(true);
    }
    QSize colorPatchSize = cfg.popupColorPatchSize();
    m_ui->sbPatchWidth->setValue(colorPatchSize.width());
    m_ui->sbPatchHeight->setValue(colorPatchSize.height());
    // Shade Selector
    m_ui->chkShadeSelUpdateExternal->setChecked(cfg.shadeSelectorUpdateOnExternalChanges());
    m_ui->chkShadeSelUpdateInteraction->setChecked(cfg.shadeSelectorUpdateOnInteractionEnd());
    m_ui->chkShadeSelUpdateOnRightClick->setChecked(cfg.shadeSelectorUpdateOnRightClick());
    m_shadeLineConfig = cfg.shadeSelectorLines();
    m_ui->sbShadeLineCount->setValue(m_shadeLineConfig.size());
    m_ui->sbShadeLineHeight->setValue(cfg.shadeSelectorLineHeight());
}

void WGColorSelectorSettings::loadDefaultPreferences()
{

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
