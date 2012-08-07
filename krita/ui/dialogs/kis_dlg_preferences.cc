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
#include <KoPart.h>

#include <kapplication.h>
#include <kmessagebox.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlrequester.h>
#include <kpagewidgetmodel.h>
#include <kvbox.h>
#include <kundo2stack.h>

#include <KoIcon.h>
#include <KoConfig.h>

#ifdef NEPOMUK
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#endif

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

#include "KoColorProfile.h"

// for the performance update
#include <kis_cubic_curve.h>
#include <config-ocio.h>


GeneralTab::GeneralTab(QWidget *_parent, const char *_name)
    : WdgGeneralSettings(_parent, _name)
{
    KisConfig cfg;

    m_cmbCursorShape->addItem(i18n("Small Circle"));
#if defined(HAVE_OPENGL)
    m_cmbCursorShape->addItem("3D Brush Model");
#endif

#ifdef NEPOMUK
    grpResourceTagging->show();
#else
    grpResourceTagging->hide();
#endif

    m_cmbCursorShape->setCurrentIndex(cfg.cursorStyle());
    chkShowRootLayer->setChecked(cfg.showRootLayer());
    chkZoomWithWheel->setChecked(cfg.zoomWithWheel());

    int autosaveInterval = cfg.autoSaveInterval();
    //convert to minutes
    m_autosaveSpinBox->setValue(autosaveInterval / 60);
    m_autosaveCheckBox->setChecked(autosaveInterval > 0);
    m_undoStackSize->setValue(cfg.undoStackLimit());
    m_backupFileCheckBox->setChecked(cfg.backupFile());
    m_showOutlinePainting->setChecked(cfg.showOutlineWhilePainting());

#ifdef NEPOMUK
    KConfigGroup tagConfig = KConfigGroup( KGlobal::config(), "resource tagging" );
    bool val = tagConfig.readEntry("nepomuk_usage_for_resource_tagging", false);
    if(!val) {
        radioXml->setChecked(true);
    }
    else {
        radioNepomuk->setChecked(true);
    }

    connect(radioNepomuk,SIGNAL(toggled(bool)),SLOT(tagBackendChange(bool)));
#endif

}

void GeneralTab::setDefault()
{
    KisConfig cfg;

    m_cmbCursorShape->setCurrentIndex(cfg.getDefaultCursorStyle());
    chkShowRootLayer->setChecked(false);
    chkZoomWithWheel->setChecked(true);
    m_autosaveCheckBox->setChecked(true);
    //convert to minutes
    m_autosaveSpinBox->setValue(KoDocument::defaultAutoSave() / 60);
    m_undoStackSize->setValue(30);
    m_backupFileCheckBox->setChecked(true);
    m_showOutlinePainting->setChecked(true);
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

void GeneralTab::tagBackendChange(bool on)
{
#ifdef NEPOMUK
    KoResourceServer<KoPattern>* tagServer = KoResourceServerProvider::instance()->patternServer();

    if(radioNepomuk->isChecked()) {
        tagServer->updateNepomukXML(on);
    }

    if (radioXml->isChecked()){
        tagServer->updateNepomukXML(on);
    }
#endif
}

//---------------------------------------------------------------------------------------------------

ColorSettingsTab::ColorSettingsTab(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(name);

    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    QGridLayout * l = new QGridLayout(this);
    l->setSpacing(KDialog::spacingHint());
    l->setMargin(0);
    m_page = new WdgColorSettings(this);
    l->addWidget(m_page, 0, 0);

    KisConfig cfg;

    m_page->chkUseSystemMonitorProfile->setChecked(cfg.useSystemMonitorProfile());

    m_page->cmbWorkingColorSpace->setIDList(KoColorSpaceRegistry::instance()->listKeys());
    m_page->cmbWorkingColorSpace->setCurrent(cfg.workingColorSpace());

    m_page->cmbPrintingColorSpace->setIDList(KoColorSpaceRegistry::instance()->listKeys());
    m_page->cmbPrintingColorSpace->setCurrent(cfg.printerColorSpace());

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

    // XXX: this needs to be available per screen!
    const KoColorProfile *profile = KisConfig::getScreenProfile();
    if (profile && profile->isSuitableForDisplay()) {
        if (cfg.useSystemMonitorProfile()) {
            // We've got an X11 profile, don't allow to override
            m_page->cmbMonitorProfile->hide();
            m_page->lblMonitorProfile->setText(i18n("Monitor profile: ") + profile->name());
        }
    } else {
        m_page->chkUseSystemMonitorProfile->setEnabled(false);
        m_page->cmbMonitorProfile->show();
        m_page->lblMonitorProfile->setText(i18n("&Monitor profile: "));
    }

    connect(m_page->cmbPrintingColorSpace, SIGNAL(activated(const KoID &)),
            this, SLOT(refillPrintProfiles(const KoID &)));

#ifndef HAVE_OCIO
    m_page->grpOcio->hide();
#endif
    m_page->grpOcio->setChecked(cfg.useOcio());
    m_page->chkOcioUseEnvironment->setChecked(cfg.useOcioEnvironmentVariable());
    enableOcioConfigPath(cfg.useOcioEnvironmentVariable());
    m_page->txtOcioConfigPath->setText(cfg.ocioConfigurationPath());
    connect(m_page->bnSelectOcioConfigPath, SIGNAL(clicked()), this, SLOT(selectOcioConfigPath()));
    connect(m_page->chkOcioUseEnvironment, SIGNAL(toggled(bool)), this, SLOT(enableOcioConfigPath(bool)));

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

    m_page->chkOcioUseEnvironment->setChecked(true);
    m_page->txtOcioConfigPath->setText(QString());

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

void ColorSettingsTab::selectOcioConfigPath()
{
    QString filename = m_page->txtOcioConfigPath->text();

    filename = KFileDialog::getOpenFileName(QDir::cleanPath(filename), "*.ocio|OpenColorIO configuration (*.ocio)", m_page);
    QFile f(filename);
    if (f.exists()) {
        m_page->txtOcioConfigPath->setText(filename);
    }
}

void ColorSettingsTab::enableOcioConfigPath(bool enable)
{
    m_page->lblOcioConfig->setEnabled(!enable);
    m_page->txtOcioConfigPath->setEnabled(!enable);
    m_page->bnSelectOcioConfigPath->setEnabled(!enable);
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
    l->setSpacing(KDialog::spacingHint());
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

DisplaySettingsTab::DisplaySettingsTab(QWidget *parent, const char *name)
    : WdgDisplaySettings(parent, name)
{
    KisConfig cfg;

    labelWarning->setPixmap(koIcon("dialog-warning").pixmap(32, 32));
#ifdef HAVE_OPENGL
    if (!QGLFormat::hasOpenGL()) {
        cbUseOpenGL->setEnabled(false);
        cbUseOpenGLShaders->setEnabled(false);
        cbUseOpenGLToolOutlineWorkaround->setEnabled(false);
        cbUseOpenGLTrilinearFiltering->setEnabled(false);
    } else {
        cbUseOpenGL->setChecked(cfg.useOpenGL());
        cbUseOpenGLToolOutlineWorkaround->setEnabled(cfg.useOpenGL());
        cbUseOpenGLToolOutlineWorkaround->setChecked(cfg.useOpenGLToolOutlineWorkaround());
        cbUseOpenGLTrilinearFiltering->setEnabled(cfg.useOpenGL());
        cbUseOpenGLTrilinearFiltering->setChecked(cfg.useOpenGLTrilinearFiltering());
#ifdef HAVE_GLEW
        if (KisOpenGL::hasShadingLanguage()) {
            cbUseOpenGLShaders->setChecked(cfg.useOpenGLShaders());
            cbUseOpenGLShaders->setEnabled(cfg.useOpenGL());
        } else {
            cbUseOpenGLShaders->setChecked(false);
            cbUseOpenGLShaders->setEnabled(false);
        }
#else
        cbUseOpenGLShaders->setChecked(false);
        cbUseOpenGLShaders->setEnabled(false);
#endif
    }
#else
    grpOpenGL->setEnabled(false);
    cbUseOpenGLShaders->setEnabled(false);
#endif

    QStringList qtVersion = QString(qVersion()).split('.');
    int versionNumber = qtVersion.at(0).toInt()*10000
            + qtVersion.at(1).toInt()*100
            + qtVersion.at(2).toInt();
    if(versionNumber>=40603)
        cbUseOpenGLToolOutlineWorkaround->hide();

    intCheckSize->setValue(cfg.checkSize());
    chkMoving->setChecked(cfg.scrollCheckers());
    colorChecks->setColor(cfg.checkersColor());
    canvasBorder->setColor(cfg.canvasBorderColor());
    chkCurveAntialiasing->setChecked(cfg.antialiasCurves());

    connect(cbUseOpenGL, SIGNAL(toggled(bool)), SLOT(slotUseOpenGLToggled(bool)));
}

void DisplaySettingsTab::setDefault()
{
    cbUseOpenGL->setChecked(false);
    cbUseOpenGLShaders->setChecked(false);
    cbUseOpenGLShaders->setEnabled(false);
    cbUseOpenGLToolOutlineWorkaround->setChecked(false);
    cbUseOpenGLToolOutlineWorkaround->setEnabled(false);
    cbUseOpenGLTrilinearFiltering->setEnabled(false);
    cbUseOpenGLTrilinearFiltering->setChecked(true);
    chkMoving->setChecked(true);
    intCheckSize->setValue(32);
    colorChecks->setColor(QColor(220, 220, 220));
    canvasBorder->setColor(QColor(Qt::gray));
}

void DisplaySettingsTab::slotUseOpenGLToggled(bool isChecked)
{
#ifdef HAVE_OPENGL
#ifdef HAVE_GLEW
    if (KisOpenGL::hasShadingLanguage()) {
        cbUseOpenGLShaders->setEnabled(isChecked);
    }
#endif
    cbUseOpenGLToolOutlineWorkaround->setEnabled(isChecked);
    cbUseOpenGLTrilinearFiltering->setEnabled(isChecked);
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
        foreach(KoPart* part, app->partList()) {
            KoDocument *doc = part->document();
            doc->setAutoSave(dialog->m_general->autoSaveInterval());
            doc->setBackupFile(dialog->m_general->m_backupFileCheckBox->isChecked());
            doc->undoStack()->setUndoLimit(dialog->m_general->undoStackSize());
        }
        cfg.setUndoStackLimit(dialog->m_general->undoStackSize());
        cfg.setZoomWithWheel(dialog->m_general->chkZoomWithWheel->isChecked());

        // Color settings
        cfg.setUseSystemMonitorProfile(dialog->m_colorSettings->m_page->chkUseSystemMonitorProfile->isChecked());
        cfg.setMonitorProfile(dialog->m_colorSettings->m_page->cmbMonitorProfile->itemHighlighted());
        cfg.setWorkingColorSpace(dialog->m_colorSettings->m_page->cmbWorkingColorSpace->currentItem().id());
        cfg.setPrinterColorSpace(dialog->m_colorSettings->m_page->cmbPrintingColorSpace->currentItem().id());
        cfg.setPrinterProfile(dialog->m_colorSettings->m_page->cmbPrintProfile->itemHighlighted());

        cfg.setUseBlackPointCompensation(dialog->m_colorSettings->m_page->chkBlackpoint->isChecked());
        cfg.setAllowLCMSOptimization(dialog->m_colorSettings->m_page->chkAllowLCMSOptimization->isChecked());
        cfg.setPasteBehaviour(dialog->m_colorSettings->m_pasteBehaviourGroup.checkedId());
        cfg.setRenderIntent(dialog->m_colorSettings->m_page->cmbMonitorIntent->currentIndex());

        cfg.setUseOcio(dialog->m_colorSettings->m_page->grpOcio->isChecked());
        cfg.setUseOcioEnvironmentVariable(dialog->m_colorSettings->m_page->chkOcioUseEnvironment->isChecked());
        cfg.setOcioConfigurationPath(dialog->m_colorSettings->m_page->txtOcioConfigPath->text());

        // Tablet settings
        cfg.setPressureTabletCurve( dialog->m_tabletSettings->m_page->pressureCurve->curve().toString() );

#if 0
        cfg.setMaxTilesInMem(dialog->m_performanceSettings->m_maxTiles->value());
        // let the tile manager know
        //KisTileManager::instance()->configChanged();
#endif

#ifdef HAVE_OPENGL
        if (dialog->m_displaySettings->cbUseOpenGL->isChecked() && cfg.canvasState() == "OPENGL_NOT_TRIED") {
            cfg.setCanvasState("TRY_OPENGL");
        }
        if (dialog->m_displaySettings->cbUseOpenGL->isChecked() && cfg.canvasState() == "OPENGL_FAILED") {
            if (KMessageBox::warningYesNo(0, i18n("You are trying to enable OpenGL\n\n"
                                                  "But Krita might have had problems with the OpenGL canvas before,\n"
                                                  "either because of driver issues, or because of issues with window effects.\n\n"
                                                  "Are you sure you want to enable OpenGL?\n"), i18n("Krita")) == KMessageBox::Yes) {
                cfg.setCanvasState("TRY_OPENGL");
            }
        }

        cfg.setUseOpenGL(dialog->m_displaySettings->cbUseOpenGL->isChecked());
        cfg.setUseOpenGLShaders(dialog->m_displaySettings->cbUseOpenGLShaders->isChecked());
        cfg.setUseOpenGLToolOutlineWorkaround(dialog->m_displaySettings->cbUseOpenGLToolOutlineWorkaround->isChecked());
        cfg.setUseOpenGLTrilinearFiltering(dialog->m_displaySettings->cbUseOpenGLTrilinearFiltering->isChecked());
#else
        cfg.setUseOpenGLToolOutlineWorkaround(false);
#endif

        cfg.setCheckSize(dialog->m_displaySettings->intCheckSize->value());
        cfg.setScrollingCheckers(dialog->m_displaySettings->chkMoving->isChecked());
        cfg.setCheckersColor(dialog->m_displaySettings->colorChecks->color());
        cfg.setCanvasBorderColor(dialog->m_displaySettings->canvasBorder->color());
        cfg.setAntialiasCurves(dialog->m_displaySettings->chkCurveAntialiasing->isChecked());
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
