/*
 *  preferencesdlg.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
 *  Copyright (c) 2003-2011 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_dlg_preferences.h"

#include <opengl/kis_opengl.h>

#include <QBitmap>
#include <QCheckBox>
#include <QCursor>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QThread>
#include <QDesktopServices>
#include <QGridLayout>
#include <QRadioButton>
#include <QGroupBox>

#ifdef HAVE_OPENGL
#include <qgl.h>
#endif

#include <KoDocument.h>
#include <KoColorProfile.h>
#include <KoApplication.h>
#include <KoConfigAuthorPage.h>
#include <KoFileDialog.h>
#include <KoPart.h>
#include <KoColorSpaceEngine.h>
#include <KoIcon.h>
#include <KoConfig.h>

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kpagewidgetmodel.h>
#include <kvbox.h>
#include <kundo2stack.h>
#include <kstandarddirs.h>


#include "widgets/squeezedcombobox.h"
#include "kis_clipboard.h"
#include "widgets/kis_cmb_idlist.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_canvas_resource_provider.h"
#include "kis_preference_set_registry.h"
#include "kis_factory2.h"
#include "KoID.h"

// for the performance update
#include <kis_cubic_curve.h>

#include "input/config/kis_input_configuration_page.h"


GeneralTab::GeneralTab(QWidget *_parent, const char *_name)
    : WdgGeneralSettings(_parent, _name)
{
    KisConfig cfg;

    m_cmbCursorShape->addItem(i18n("Tool Icon"));
    m_cmbCursorShape->addItem(i18n("Crosshair"));
    m_cmbCursorShape->addItem(i18n("Arrow"));
    m_cmbCursorShape->addItem(i18n("Brush Outline"));
    m_cmbCursorShape->addItem(i18n("No Cursor"));
    m_cmbCursorShape->addItem(i18n("Small Circle"));
    m_cmbCursorShape->addItem(i18n("Brush Outline with Small Circle"));
    m_cmbCursorShape->addItem(i18n("Brush Outline with Crosshair"));
    m_cmbCursorShape->addItem(i18n("Triangle Righthanded"));
    m_cmbCursorShape->addItem(i18n("Triangle Lefthanded"));
    m_cmbCursorShape->addItem(i18n("Brush Outline with Triangle Righthanded"));
    m_cmbCursorShape->addItem(i18n("Brush Outline with Triangle Lefthanded"));

    m_cmbCursorShape->setCurrentIndex(cfg.cursorStyle());
    chkShowRootLayer->setChecked(cfg.showRootLayer());

    int autosaveInterval = cfg.autoSaveInterval();
    //convert to minutes
    m_autosaveSpinBox->setValue(autosaveInterval / 60);
    m_autosaveCheckBox->setChecked(autosaveInterval > 0);
    m_undoStackSize->setValue(cfg.undoStackLimit());
    m_backupFileCheckBox->setChecked(cfg.backupFile());
    m_showOutlinePainting->setChecked(cfg.showOutlineWhilePainting());
    m_favoritePresetsSpinBox->setValue(cfg.favoritePresets());
}

void GeneralTab::setDefault()
{
    KisConfig cfg;

    m_cmbCursorShape->setCurrentIndex(cfg.getDefaultCursorStyle());
    chkShowRootLayer->setChecked(false);
    m_autosaveCheckBox->setChecked(true);
    //convert to minutes
    m_autosaveSpinBox->setValue(KoDocument::defaultAutoSave() / 60);
    m_undoStackSize->setValue(30);
    m_backupFileCheckBox->setChecked(true);
    m_showOutlinePainting->setChecked(true);
    m_favoritePresetsSpinBox->setValue(10);
}

enumCursorStyle GeneralTab::cursorStyle()
{
    return (enumCursorStyle)m_cmbCursorShape->currentIndex();
}

bool GeneralTab::showRootLayer()
{
    return chkShowRootLayer->isChecked();
}

int GeneralTab::autoSaveInterval()
{
    //convert to seconds
    return m_autosaveCheckBox->isChecked() ? m_autosaveSpinBox->value()*60 : 0;
}

int GeneralTab::undoStackSize()
{
    return m_undoStackSize->value();
}

bool GeneralTab::showOutlineWhilePainting()
{
    return m_showOutlinePainting->isChecked();
}

int GeneralTab::favoritePresets()
{
    return m_favoritePresetsSpinBox->value();
}


ColorSettingsTab::ColorSettingsTab(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);

    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    QGridLayout * l = new QGridLayout(this);
    l->setMargin(0);
    m_page = new WdgColorSettings(this);
    l->addWidget(m_page, 0, 0);

    KisConfig cfg;

    m_page->chkUseSystemMonitorProfile->setChecked(cfg.useSystemMonitorProfile());
    connect(m_page->chkUseSystemMonitorProfile, SIGNAL(toggled(bool)), this, SLOT(toggleAllowMonitorProfileSelection(bool)));

    m_page->cmbWorkingColorSpace->setIDList(KoColorSpaceRegistry::instance()->listKeys());
    m_page->cmbWorkingColorSpace->setCurrent(cfg.workingColorSpace());

    m_page->cmbPrintingColorSpace->setIDList(KoColorSpaceRegistry::instance()->listKeys());
    m_page->cmbPrintingColorSpace->setCurrent(cfg.printerColorSpace());

    m_page->bnAddColorProfile->setIcon(koIcon("document-open"));
    m_page->bnAddColorProfile->setToolTip( i18n("Open Color Profile") );
    connect(m_page->bnAddColorProfile, SIGNAL(clicked()), SLOT(installProfile()));

    refillMonitorProfiles(KoID("RGBA", ""));
    refillPrintProfiles(KoID(cfg.printerColorSpace(), ""));

    //hide printing settings
    m_page->groupBox2->hide();

    if (m_page->cmbMonitorProfile->contains(cfg.monitorProfile()))
        m_page->cmbMonitorProfile->setCurrent(cfg.monitorProfile());
    if (m_page->cmbPrintProfile->contains(cfg.printerProfile()))
        m_page->cmbPrintProfile->setCurrentIndex(m_page->cmbPrintProfile->findText(cfg.printerProfile()));

    m_page->chkBlackpoint->setChecked(cfg.useBlackPointCompensation());
    m_page->chkAllowLCMSOptimization->setChecked(cfg.allowLCMSOptimization());

    m_pasteBehaviourGroup.addButton(m_page->radioPasteWeb, PASTE_ASSUME_WEB);
    m_pasteBehaviourGroup.addButton(m_page->radioPasteMonitor, PASTE_ASSUME_MONITOR);
    m_pasteBehaviourGroup.addButton(m_page->radioPasteAsk, PASTE_ASK);

    QAbstractButton *button = m_pasteBehaviourGroup.button(cfg.pasteBehaviour());
    Q_ASSERT(button);

    if (button) {
        button->setChecked(true);
    }

    m_page->cmbMonitorIntent->setCurrentIndex(cfg.renderIntent());

    toggleAllowMonitorProfileSelection(cfg.useSystemMonitorProfile());

    connect(m_page->cmbPrintingColorSpace, SIGNAL(activated(const KoID &)),
            this, SLOT(refillPrintProfiles(const KoID &)));
}

void ColorSettingsTab::installProfile()
{
    QStringList mime;
    mime << "ICM Profile (*.icm(" <<  "ICC Profile (*.icc)";
    KoFileDialog dialog(this, KoFileDialog::OpenFiles, "OpenDocumentICC");
    dialog.setCaption(i18n("Install Color Profiles"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    dialog.setNameFilters(mime);
    QStringList profileNames = dialog.urls();

    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);

    QString saveLocation = KGlobal::mainComponent().dirs()->saveLocation("icc_profiles");

    foreach (const QString &profileName, profileNames) {
        KUrl file(profileName);
        if (!QFile::copy(profileName, saveLocation + file.fileName())) {
            kWarning() << "Could not install profile!";
            return;
        }
        iccEngine->addProfile(saveLocation + file.fileName());

    }

    KisConfig cfg;
    refillMonitorProfiles(KoID("RGBA", ""));
    refillPrintProfiles(KoID(cfg.printerColorSpace(), ""));

    if (m_page->cmbMonitorProfile->contains(cfg.monitorProfile()))
        m_page->cmbMonitorProfile->setCurrent(cfg.monitorProfile());
    if (m_page->cmbPrintProfile->contains(cfg.printerProfile()))
        m_page->cmbPrintProfile->setCurrentIndex(m_page->cmbPrintProfile->findText(cfg.printerProfile()));


}

void ColorSettingsTab::toggleAllowMonitorProfileSelection(bool useSystemProfile)
{
    // XXX: this needs to be available per screen!
    if (useSystemProfile) {
        const KoColorProfile *profile = KisConfig::getScreenProfile();
        if (profile && profile->isSuitableForDisplay()) {
            // We've got an X11 profile, don't allow to override
            m_page->cmbMonitorProfile->hide();
            m_page->lblMonitorProfile->setText(i18n("Monitor profile: ") + profile->name());
        }
    }
    else {
        m_page->cmbMonitorProfile->show();
        m_page->lblMonitorProfile->setText(i18n("&Monitor profile: "));
    }


}


void ColorSettingsTab::setDefault()
{
    m_page->cmbWorkingColorSpace->setCurrent("RGBA");

    m_page->cmbPrintingColorSpace->setCurrent("CMYK");
    refillPrintProfiles(KoID("CMYK", ""));

    refillMonitorProfiles(KoID("RGBA", ""));

    m_page->chkBlackpoint->setChecked(false);
    m_page->chkAllowLCMSOptimization->setChecked(true);
    m_page->cmbMonitorIntent->setCurrentIndex(INTENT_PERCEPTUAL);

    QAbstractButton *button = m_pasteBehaviourGroup.button(PASTE_ASK);
    Q_ASSERT(button);

    if (button) {
        button->setChecked(true);
    }
}


void ColorSettingsTab::refillMonitorProfiles(const KoID & s)
{
    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s.id());

    m_page->cmbMonitorProfile->clear();

    if (!csf)
        return;

    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

    foreach(const KoColorProfile *profile, profileList) {
        if (profile->isSuitableForDisplay())
            m_page->cmbMonitorProfile->addSqueezedItem(profile->name());
    }

    m_page->cmbMonitorProfile->setCurrent(csf->defaultProfile());
}

void ColorSettingsTab::refillPrintProfiles(const KoID & s)
{
    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s.id());

    m_page->cmbPrintProfile->clear();

    if (!csf)
        return;

    QList<const KoColorProfile *> profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

    foreach(const KoColorProfile *profile, profileList) {
        if (profile->isSuitableForPrinting())
            m_page->cmbPrintProfile->addSqueezedItem(profile->name());
    }

    m_page->cmbPrintProfile->setCurrent(csf->defaultProfile());
}

//---------------------------------------------------------------------------------------------------

void TabletSettingsTab::setDefault()
{
    KisCubicCurve curve;
    curve.fromString(DEFAULT_CURVE_STRING);
    m_page->pressureCurve->setCurve(curve);
}

TabletSettingsTab::TabletSettingsTab(QWidget* parent, const char* name): QWidget(parent)
{
    setObjectName(name);

    QGridLayout * l = new QGridLayout(this);
    l->setMargin(0);
    m_page = new WdgTabletSettings(this);
    l->addWidget(m_page, 0, 0);

    KisConfig cfg;
    KisCubicCurve curve;
    curve.fromString( cfg.pressureTabletCurve() );

    m_page->pressureCurve->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX));
    m_page->pressureCurve->setCurve(curve);
}


//---------------------------------------------------------------------------------------------------
PerformanceTab::PerformanceTab(QWidget *parent, const char *name)
    : WdgPerformanceSettings(parent, name)
{
    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    KisConfig cfg;

    m_maxTiles->setValue(cfg.maxTilesInMem());
}

void PerformanceTab::setDefault()
{
    m_maxTiles->setValue(500);
}

//---------------------------------------------------------------------------------------------------

#include "KoColor.h"
#include "KoColorPopupAction.h"

DisplaySettingsTab::DisplaySettingsTab(QWidget *parent, const char *name)
    : WdgDisplaySettings(parent, name)
{
    KisConfig cfg;

#ifdef HAVE_OPENGL
    if (!QGLFormat::hasOpenGL()) {
        cbUseOpenGL->setEnabled(false);
        chkUseTextureBuffer->setEnabled(false);
        chkDisableDoubleBuffering->setEnabled(false);
        chkDisableVsync->setEnabled(false);
        cmbFilterMode->setEnabled(false);
    } else {
        cbUseOpenGL->setChecked(cfg.useOpenGL());
        chkUseTextureBuffer->setEnabled(cfg.useOpenGL());
        chkUseTextureBuffer->setChecked(cfg.useOpenGLTextureBuffer());
        chkDisableDoubleBuffering->setVisible(cfg.showAdvancedOpenGLSettings());
        chkDisableDoubleBuffering->setEnabled(cfg.useOpenGL());
        chkDisableDoubleBuffering->setChecked(cfg.disableDoubleBuffering());
        chkDisableVsync->setVisible(cfg.showAdvancedOpenGLSettings());
        chkDisableVsync->setEnabled(cfg.useOpenGL());
        chkDisableVsync->setChecked(cfg.disableVSync());
        cmbFilterMode->setEnabled(cfg.useOpenGL());
        cmbFilterMode->setCurrentIndex(cfg.openGLFilteringMode());
        // Don't show the high quality filtering mode if it's not available
        if (!KisOpenGL::supportsGLSL13()) {
            cmbFilterMode->removeItem(3);
        }
    }
    if (qApp->applicationName() == "kritasketch" || qApp->applicationName() == "kritagemini") {
        cbUseOpenGL->setVisible(false);
        cbUseOpenGL->setMaximumHeight(0);
    }
#else
    grpOpenGL->setEnabled(false);
#endif

    KoColor c;
    c.fromQColor(cfg.selectionOverlayMaskColor());
    m_selectionOverlayColorAction = new KoColorPopupAction(this);
    m_selectionOverlayColorAction->setCurrentColor(c);
    m_selectionOverlayColorAction->setIcon(koIcon("format-stroke-color"));
    m_selectionOverlayColorAction->setToolTip(i18n("Change the background color of the image"));
    btnSelectionOverlayColor->setDefaultAction(m_selectionOverlayColorAction);


    intCheckSize->setValue(cfg.checkSize());
    chkMoving->setChecked(cfg.scrollCheckers());
    colorChecks1->setColor(cfg.checkersColor1());
    colorChecks2->setColor(cfg.checkersColor2());
    canvasBorder->setColor(cfg.canvasBorderColor());
    hideScrollbars->setChecked(cfg.hideScrollbars());
    chkCurveAntialiasing->setChecked(cfg.antialiasCurves());
    chkSelectionOutlineAntialiasing->setChecked(cfg.antialiasSelectionOutline());
    chkChannelsAsColor->setChecked(cfg.showSingleChannelAsColor());

    connect(cbUseOpenGL, SIGNAL(toggled(bool)), SLOT(slotUseOpenGLToggled(bool)));
}

void DisplaySettingsTab::setDefault()
{
    cbUseOpenGL->setChecked(true);
    chkUseTextureBuffer->setChecked(false);
    chkUseTextureBuffer->setEnabled(true);
    chkDisableDoubleBuffering->setEnabled(true);
    chkDisableDoubleBuffering->setChecked(true);
    chkDisableVsync->setEnabled(true);
    chkDisableVsync->setChecked(true);
    cmbFilterMode->setEnabled(true);
    cmbFilterMode->setCurrentIndex(1);
    chkMoving->setChecked(true);
    intCheckSize->setValue(32);
    colorChecks1->setColor(QColor(220, 220, 220));
    colorChecks2->setColor(Qt::white);
    canvasBorder->setColor(QColor(Qt::gray));
    hideScrollbars->setChecked(false);
    chkCurveAntialiasing->setChecked(true);
    chkSelectionOutlineAntialiasing->setChecked(false);
    chkChannelsAsColor->setChecked(false);
}

void DisplaySettingsTab::slotUseOpenGLToggled(bool isChecked)
{
#ifdef HAVE_OPENGL
    chkUseTextureBuffer->setEnabled(isChecked);
    chkDisableDoubleBuffering->setEnabled(isChecked);
    chkDisableVsync->setEnabled(isChecked);
    cmbFilterMode->setEnabled(isChecked);
#else
    Q_UNUSED(isChecked);
#endif
}

//---------------------------------------------------------------------------------------------------
GridSettingsTab::GridSettingsTab(QWidget* parent) : WdgGridSettingsBase(parent)
{
    KisConfig cfg;
    selectMainStyle->setCurrentIndex(cfg.getGridMainStyle());
    selectSubdivisionStyle->setCurrentIndex(cfg.getGridSubdivisionStyle());

    colorMain->setColor(cfg.getGridMainColor());
    colorSubdivision->setColor(cfg.getGridSubdivisionColor());

    intHSpacing->setValue(cfg.getGridHSpacing());
    intVSpacing->setValue(cfg.getGridVSpacing());
    spacingAspectButton->setKeepAspectRatio(cfg.getGridSpacingAspect());
    linkSpacingToggled(cfg.getGridSpacingAspect());

    intSubdivision->setValue(cfg.getGridSubdivisions());

    intXOffset->setValue(cfg.getGridOffsetX());
    intYOffset->setValue(cfg.getGridOffsetY());
    offsetAspectButton->setKeepAspectRatio(cfg.getGridOffsetAspect());
    linkOffsetToggled(cfg.getGridOffsetAspect());

    connect(spacingAspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(linkSpacingToggled(bool)));
    connect(offsetAspectButton, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(linkOffsetToggled(bool)));

    connect(intHSpacing, SIGNAL(valueChanged(int)), this, SLOT(spinBoxHSpacingChanged(int)));
    connect(intVSpacing, SIGNAL(valueChanged(int)), this, SLOT(spinBoxVSpacingChanged(int)));
    connect(intXOffset, SIGNAL(valueChanged(int)), this, SLOT(spinBoxXOffsetChanged(int)));
    connect(intYOffset, SIGNAL(valueChanged(int)), this, SLOT(spinBoxYOffsetChanged(int)));


}

void GridSettingsTab::setDefault()
{
    KisConfig cfg;
    selectMainStyle->setCurrentIndex(0);
    selectSubdivisionStyle->setCurrentIndex(1);

    colorMain->setColor(QColor(99, 99, 99));
    colorSubdivision->setColor(QColor(199, 199, 199));

    intHSpacing->setValue(10);
    intVSpacing->setValue(10);
    linkSpacingToggled(false);
    intSubdivision->setValue(1);
    intXOffset->setValue(0);
    intYOffset->setValue(0);
    linkOffsetToggled(false);
}

void GridSettingsTab::spinBoxHSpacingChanged(int v)
{
    if (m_linkSpacing) {
        intVSpacing->setValue(v);
    }
}

void GridSettingsTab::spinBoxVSpacingChanged(int v)
{
    if (m_linkSpacing) {
        intHSpacing->setValue(v);
    }
}

void GridSettingsTab::linkSpacingToggled(bool b)
{
    m_linkSpacing = b;

    if (m_linkSpacing) {
        intVSpacing->setValue(intHSpacing->value());
    }
}

void GridSettingsTab::spinBoxXOffsetChanged(int v)
{
    if (m_linkOffset) {
        intYOffset->setValue(v);
    }
}

void GridSettingsTab::spinBoxYOffsetChanged(int v)
{
    if (m_linkOffset) {
        intXOffset->setValue(v);
    }
}

void GridSettingsTab::linkOffsetToggled(bool b)
{
    m_linkOffset = b;

    if (m_linkOffset) {
        intYOffset->setValue(intXOffset->value());
    }
}

//---------------------------------------------------------------------------------------------------
FullscreenSettingsTab::FullscreenSettingsTab(QWidget* parent) : WdgFullscreenSettingsBase(parent)
{
    KisConfig cfg;

    chkDockers->setChecked(cfg.hideDockersFullscreen());
    chkMenu->setChecked(cfg.hideMenuFullscreen());
    chkScrollbars->setChecked(cfg.hideScrollbarsFullscreen());
    chkStatusbar->setChecked(cfg.hideStatusbarFullscreen());
    chkTitlebar->setChecked(cfg.hideTitlebarFullscreen());
    chkToolbar->setChecked(cfg.hideToolbarFullscreen());

}

void FullscreenSettingsTab::setDefault()
{
    chkDockers->setChecked(true);
    chkMenu->setChecked(true);
    chkScrollbars->setChecked(true);
    chkStatusbar->setChecked(true);
    chkTitlebar->setChecked(true);
    chkToolbar->setChecked(true);
}


//---------------------------------------------------------------------------------------------------

KisDlgPreferences::KisDlgPreferences(QWidget* parent, const char* name)
    : KPageDialog(parent)
{
    Q_UNUSED(name);
    setCaption(i18n("Preferences"));
    setButtons(Ok | Cancel | Help | Default);
    setDefaultButton(Ok);
    showButtonSeparator(true);
    setFaceType(KPageDialog::List);

    // General
    KVBox *vbox = new KVBox();
    KPageWidgetItem *page = new KPageWidgetItem(vbox, i18n("General"));
    page->setHeader(i18n("General"));
    page->setIcon(koIcon("configure"));
    addPage(page);
    m_general = new GeneralTab(vbox);

    // Display
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Display"));
    page->setHeader(i18n("Display"));
    page->setIcon(koIcon("preferences-desktop-display"));
    addPage(page);
    m_displaySettings = new DisplaySettingsTab(vbox);

    // Color
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Color Management"));
    page->setHeader(i18n("Color"));
    page->setIcon(koIcon("preferences-desktop-color"));
    addPage(page);
    m_colorSettings = new ColorSettingsTab(vbox);

    // Performance
#if 0
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Performance"));
    page->setHeader(i18n("Performance"));
    page->setIcon(koIcon("preferences-system-performance"));
    addPage(page);
    m_performanceSettings = new PerformanceTab(vbox);
#endif

    // Grid
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Grid"));
    page->setHeader(i18n("Grid"));
    page->setIcon(koIcon("grid"));
    addPage(page);
    m_gridSettings = new GridSettingsTab(vbox);

    // Tablet
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Tablet settings"));
    page->setHeader(i18n("Tablet"));
    page->setIcon(koIcon("input-tablet"));
    addPage(page);
    m_tabletSettings = new TabletSettingsTab(vbox);


    // full-screen mode
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Canvas-only settings"));
    page->setHeader(i18n("Canvas-only"));
    page->setIcon(koIcon("preferences-system-performance"));
    addPage(page);
    m_fullscreenSettings = new FullscreenSettingsTab(vbox);


    // author settings
    vbox = new KVBox();
    m_authorSettings = new KoConfigAuthorPage();
    page = addPage(m_authorSettings, i18nc("@title:tab Author page", "Author"));
    page->setHeader(i18n("Author"));
    page->setIcon(koIcon("user-identity"));

    m_inputConfiguration = new KisInputConfigurationPage();
    page = addPage(m_inputConfiguration, i18n("Canvas Input Settings"));
    page->setHeader(i18n("Canvas Input"));
    page->setIcon(koIcon("input-tablet"));
    connect(this, SIGNAL(okClicked()), m_inputConfiguration, SLOT(saveChanges()));
    connect(this, SIGNAL(applyClicked()), m_inputConfiguration, SLOT(saveChanges()));
    connect(this, SIGNAL(cancelClicked()), m_inputConfiguration, SLOT(revertChanges()));
    connect(this, SIGNAL(defaultClicked()), m_inputConfiguration, SLOT(setDefaults()));

    KisPreferenceSetRegistry *preferenceSetRegistry = KisPreferenceSetRegistry::instance();
    foreach (KisAbstractPreferenceSetFactory *preferenceSetFactory, preferenceSetRegistry->values()) {
        KisPreferenceSet* preferenceSet = preferenceSetFactory->createPreferenceSet();
        vbox = new KVBox();
        page = new KPageWidgetItem(vbox, preferenceSet->name());
        page->setHeader(preferenceSet->header());
        page->setIcon(preferenceSet->icon());
        addPage(page);
        preferenceSet->setParent(vbox);
        preferenceSet->loadPreferences();

        connect(this, SIGNAL(defaultClicked()), preferenceSet, SLOT(loadDefaultPreferences()), Qt::UniqueConnection);
        connect(this, SIGNAL(okClicked()),      preferenceSet, SLOT(savePreferences()),        Qt::UniqueConnection);
    }

    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefault()));

}

KisDlgPreferences::~KisDlgPreferences()
{
}

void KisDlgPreferences::slotDefault()
{
    m_general->setDefault();
    m_colorSettings->setDefault();
#if 0
    m_performanceSettings->setDefault();
#endif
#ifdef HAVE_OPENGL
    m_displaySettings->setDefault();
#endif
    m_gridSettings->setDefault();
    m_tabletSettings->setDefault();
    m_fullscreenSettings->setDefault();
}

bool KisDlgPreferences::editPreferences()
{
    KisDlgPreferences* dialog;

    dialog = new KisDlgPreferences();
    bool baccept = (dialog->exec() == Accepted);
    if (baccept) {
        // General settings
        KisConfig cfg;
        cfg.setCursorStyle(dialog->m_general->cursorStyle());
        cfg.setShowRootLayer(dialog->m_general->showRootLayer());
        cfg.setShowOutlineWhilePainting(dialog->m_general->showOutlineWhilePainting());

        cfg.setAutoSaveInterval(dialog->m_general->autoSaveInterval());
        cfg.setBackupFile(dialog->m_general->m_backupFileCheckBox->isChecked());
        KoApplication *app = qobject_cast<KoApplication*>(qApp);
        if (app) {
            foreach(KoPart* part, app->partList()) {
                if (part) {
                    KoDocument *doc = part->document();
                    if (doc) {
                        doc->setAutoSave(dialog->m_general->autoSaveInterval());
                        doc->setBackupFile(dialog->m_general->m_backupFileCheckBox->isChecked());
                        doc->undoStack()->setUndoLimit(dialog->m_general->undoStackSize());
                    }
                }
            }
        }
        cfg.setUndoStackLimit(dialog->m_general->undoStackSize());
        cfg.setFavoritePresets(dialog->m_general->favoritePresets());

        // Color settings
        cfg.setUseSystemMonitorProfile(dialog->m_colorSettings->m_page->chkUseSystemMonitorProfile->isChecked());
        cfg.setMonitorProfile(dialog->m_colorSettings->m_page->cmbMonitorProfile->itemHighlighted(),
                              dialog->m_colorSettings->m_page->chkUseSystemMonitorProfile->isChecked());
        cfg.setWorkingColorSpace(dialog->m_colorSettings->m_page->cmbWorkingColorSpace->currentItem().id());
        cfg.setPrinterColorSpace(dialog->m_colorSettings->m_page->cmbPrintingColorSpace->currentItem().id());
        cfg.setPrinterProfile(dialog->m_colorSettings->m_page->cmbPrintProfile->itemHighlighted());

        cfg.setUseBlackPointCompensation(dialog->m_colorSettings->m_page->chkBlackpoint->isChecked());
        cfg.setAllowLCMSOptimization(dialog->m_colorSettings->m_page->chkAllowLCMSOptimization->isChecked());
        cfg.setPasteBehaviour(dialog->m_colorSettings->m_pasteBehaviourGroup.checkedId());
        cfg.setRenderIntent(dialog->m_colorSettings->m_page->cmbMonitorIntent->currentIndex());

        // Tablet settings
        cfg.setPressureTabletCurve( dialog->m_tabletSettings->m_page->pressureCurve->curve().toString() );

#if 0
        cfg.setMaxTilesInMem(dialog->m_performanceSettings->m_maxTiles->value());
        // let the tile manager know
        //KisTileManager::instance()->configChanged();
#endif

#ifdef HAVE_OPENGL
        if (!cfg.useOpenGL() && dialog->m_displaySettings->cbUseOpenGL->isChecked())
            cfg.setCanvasState("TRY_OPENGL");
        cfg.setUseOpenGL(dialog->m_displaySettings->cbUseOpenGL->isChecked());
        cfg.setUseOpenGLTextureBuffer(dialog->m_displaySettings->chkUseTextureBuffer->isChecked());
        cfg.setOpenGLFilteringMode(dialog->m_displaySettings->cmbFilterMode->currentIndex());
        cfg.setDisableDoubleBuffering(dialog->m_displaySettings->chkDisableDoubleBuffering->isChecked());
        cfg.setDisableVSync(dialog->m_displaySettings->chkDisableVsync->isChecked());
#endif

        cfg.setCheckSize(dialog->m_displaySettings->intCheckSize->value());
        cfg.setScrollingCheckers(dialog->m_displaySettings->chkMoving->isChecked());
        cfg.setCheckersColor1(dialog->m_displaySettings->colorChecks1->color());
        cfg.setCheckersColor2(dialog->m_displaySettings->colorChecks2->color());
        cfg.setCanvasBorderColor(dialog->m_displaySettings->canvasBorder->color());
        cfg.setHideScrollbars(dialog->m_displaySettings->hideScrollbars->isChecked());
        cfg.setSelectionOverlayMaskColor(dialog->m_displaySettings->m_selectionOverlayColorAction->currentKoColor().toQColor());
        cfg.setAntialiasCurves(dialog->m_displaySettings->chkCurveAntialiasing->isChecked());
        cfg.setAntialiasSelectionOutline(dialog->m_displaySettings->chkSelectionOutlineAntialiasing->isChecked());
        cfg.setShowSingleChannelAsColor(dialog->m_displaySettings->chkChannelsAsColor->isChecked());
        // Grid settings
        cfg.setGridMainStyle(dialog->m_gridSettings->selectMainStyle->currentIndex());
        cfg.setGridSubdivisionStyle(dialog->m_gridSettings->selectSubdivisionStyle->currentIndex());

        cfg.setGridMainColor(dialog->m_gridSettings->colorMain->color());
        cfg.setGridSubdivisionColor(dialog->m_gridSettings->colorSubdivision->color());

        cfg.setGridHSpacing(dialog->m_gridSettings->intHSpacing->value());
        cfg.setGridVSpacing(dialog->m_gridSettings->intVSpacing->value());
        cfg.setGridSpacingAspect(dialog->m_gridSettings->spacingAspectButton->keepAspectRatio());
        cfg.setGridSubdivisions(dialog->m_gridSettings->intSubdivision->value());
        cfg.setGridOffsetX(dialog->m_gridSettings->intXOffset->value());
        cfg.setGridOffsetY(dialog->m_gridSettings->intYOffset->value());
        cfg.setGridOffsetAspect(dialog->m_gridSettings->offsetAspectButton->keepAspectRatio());

        cfg.setHideDockersFullscreen(dialog->m_fullscreenSettings->chkDockers->checkState());
        cfg.setHideMenuFullscreen(dialog->m_fullscreenSettings->chkMenu->checkState());
        cfg.setHideScrollbarsFullscreen(dialog->m_fullscreenSettings->chkScrollbars->checkState());
        cfg.setHideStatusbarFullscreen(dialog->m_fullscreenSettings->chkStatusbar->checkState());
        cfg.setHideTitlebarFullscreen(dialog->m_fullscreenSettings->chkTitlebar->checkState());
        cfg.setHideToolbarFullscreen(dialog->m_fullscreenSettings->chkToolbar->checkState());

        dialog->m_authorSettings->apply();
    }
    delete dialog;
    return baccept;
}

#include "kis_dlg_preferences.moc"
