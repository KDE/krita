/*
 *  preferencesdlg.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Michael Koch <koch@kde.org>
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
#include <QPixmap>
#include <QPushButton>
#include <QSlider>
#include <QToolButton>
#include <QThread>
#include <QGridLayout>

#ifdef HAVE_OPENGL
#include <qgl.h>
#endif

#include <libs/main/KoDocument.h>
#include <KoColorProfile.h>

#include <kmessagebox.h>
#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <kpagewidgetmodel.h>
#include <kicon.h>
#include <kvbox.h>

#include "widgets/squeezedcombobox.h"
#include "kis_clipboard.h"
#include "widgets/kis_cmb_idlist.h"
#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_canvas_resource_provider.h"

#include "kis_factory2.h"
#include "KoID.h"

#include "KoColorProfile.h"

// for the performance update
#include "tiles/kis_tilemanager.h"

GeneralTab::GeneralTab(QWidget *_parent, const char *_name)
        : WdgGeneralSettings(_parent, _name)
{
    KisConfig cfg;

#if defined(HAVE_OPENGL)
    m_cmbCursorShape->addItem("3D Brush Model");
#endif

    m_cmbCursorShape->setCurrentIndex(cfg.cursorStyle());
    chkShowRootLayer->setChecked(cfg.showRootLayer());

    int autosaveInterval = cfg.autoSaveInterval();
    //convert to minutes
    m_autosaveSpinBox->setValue(autosaveInterval / 60);
    m_autosaveCheckBox->setChecked(autosaveInterval > 0);
}

void GeneralTab::setDefault()
{
    KisConfig cfg;

    m_cmbCursorShape->setCurrentIndex(cfg.getDefaultCursorStyle());
    chkShowRootLayer->setChecked(false);
    m_autosaveCheckBox->setChecked(true);
    //convert to minutes
    m_autosaveSpinBox->setValue(KoDocument::defaultAutoSave() / 60);
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

    m_page->cmbWorkingColorSpace->setIDList(KoColorSpaceRegistry::instance()->listKeys());
    m_page->cmbWorkingColorSpace->setCurrent(cfg.workingColorSpace());

    m_page->cmbPrintingColorSpace->setIDList(KoColorSpaceRegistry::instance()->listKeys());
    m_page->cmbPrintingColorSpace->setCurrent(cfg.printerColorSpace());

    refillMonitorProfiles(KoID("RGBA", ""));
    refillPrintProfiles(KoID(cfg.printerColorSpace(), ""));

    if (m_page->cmbMonitorProfile->contains(cfg.monitorProfile()))
        m_page->cmbMonitorProfile->setCurrent(cfg.monitorProfile());
    if (m_page->cmbPrintProfile->contains(cfg.printerProfile()))
        m_page->cmbPrintProfile->setCurrentIndex(m_page->cmbPrintProfile->findText(cfg.printerProfile()));
    m_page->chkBlackpoint->setChecked(cfg.useBlackPointCompensation());

    m_pasteBehaviourGroup.addButton(m_page->radioPasteWeb, PASTE_ASSUME_WEB);
    m_pasteBehaviourGroup.addButton(m_page->radioPasteMonitor, PASTE_ASSUME_MONITOR);
    m_pasteBehaviourGroup.addButton(m_page->radioPasteAsk, PASTE_ASK);

    QAbstractButton *button = m_pasteBehaviourGroup.button(cfg.pasteBehaviour());
    Q_ASSERT(button);

    if (button) {
        button->setChecked(true);
    }

    m_page->cmbMonitorIntent->setCurrentIndex(cfg.renderIntent());

    if (const KoColorProfile * profile = KisCanvasResourceProvider::getScreenProfile()) {
        // We've got an X11 profile, don't allow to override
        m_page->cmbMonitorProfile->hide();
        m_page->lblMonitorProfile->setText(i18n("Monitor profile: ") + profile->name());
    } else {
        m_page->cmbMonitorProfile->show();
        m_page->lblMonitorProfile->setText(i18n("&Monitor profile: "));
    }

    connect(m_page->cmbPrintingColorSpace, SIGNAL(activated(const KoID &)),
            this, SLOT(refillPrintProfiles(const KoID &)));
}

void ColorSettingsTab::setDefault()
{
    m_page->cmbWorkingColorSpace->setCurrent("RGBA");

    m_page->cmbPrintingColorSpace->setCurrent("CMYK");
    refillPrintProfiles(KoID("CMYK", ""));

    refillMonitorProfiles(KoID("RGBA", ""));

    m_page->chkBlackpoint->setChecked(false);
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

PerformanceTab::PerformanceTab(QWidget *parent, const char *name)
        : WdgPerformanceSettings(parent, name)
{
    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    KisConfig cfg;

    // it's scaled from 0 - 6, but the config is in 0 - 300
    m_swappiness->setValue(cfg.swappiness() / 50);
    m_maxTiles->setValue(cfg.maxTilesInMem());
#if 0
    m_projection->setChecked(cfg.useProjections());
    chkUseBoundingRect->setChecked(cfg.useBoundingRectInProjection());
    chkAggregateDirtyRegions->setChecked(cfg.aggregateDirtyRegionsInPainter());
    intChunkSize->setValue(cfg.projectionChunkSize());
    intNumThreads->setValue(cfg.numProjectionThreads());
    chkUpdateAllOfQPainterCanvas->setChecked(cfg.updateAllOfQPainterCanvas());
    chkUseNearestNeighbour->setChecked(cfg.useNearestNeigbour());
#endif
}

void PerformanceTab::setDefault()
{
    m_swappiness->setValue(3);
    m_maxTiles->setValue(500);
#if 0
    m_projection->setChecked(true);
    chkUseBoundingRect->setChecked(false);
    chkAggregateDirtyRegions->setChecked(true);
    intChunkSize->setValue(512);
    intNumThreads->setValue(QThread::idealThreadCount());
    chkUpdateAllOfQPainterCanvas->setChecked(true);
    chkUseNearestNeighbour->setChecked(false);
#endif
}

//---------------------------------------------------------------------------------------------------

DisplaySettingsTab::DisplaySettingsTab(QWidget *parent, const char *name)
        : WdgDisplaySettings(parent, name)
{
    KisConfig cfg;

#ifdef HAVE_OPENGL
    if (!QGLFormat::hasOpenGL()) {
        cbUseOpenGL->setEnabled(false);
        cbUseOpenGLShaders->setEnabled(false);
    } else {
        cbUseOpenGL->setChecked(cfg.useOpenGL());
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
    intCheckSize->setValue(cfg.checkSize());
    chkMoving->setChecked(cfg.scrollCheckers());
    colorChecks->setColor(cfg.checkersColor());
    chkCurveAntialiasing->setChecked(cfg.antialiasCurves());

    connect(cbUseOpenGL, SIGNAL(toggled(bool)), SLOT(slotUseOpenGLToggled(bool)));
}

void DisplaySettingsTab::setDefault()
{
    cbUseOpenGL->setChecked(false);
    cbUseOpenGLShaders->setChecked(false);
    cbUseOpenGLShaders->setEnabled(false);
    chkMoving->setChecked(true);
    intCheckSize->setValue(32);
    colorChecks->setColor(QColor(220, 220, 220));
}

void DisplaySettingsTab::slotUseOpenGLToggled(bool isChecked)
{
#ifdef HAVE_OPENGL
#ifdef HAVE_GLEW
    if (KisOpenGL::hasShadingLanguage()) {
        cbUseOpenGLShaders->setEnabled(isChecked);
    }
#endif
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
    page->setIcon(KIcon(BarIcon("configure", KIconLoader::SizeMedium)));
    addPage(page);
    m_general = new GeneralTab(vbox);

    // Display
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Display"));
    page->setHeader(i18n("Display"));
    page->setIcon(KIcon("preferences-desktop-display"));
    addPage(page);
    m_displaySettings = new DisplaySettingsTab(vbox);

    // Color
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Color Management"));
    page->setHeader(i18n("Color"));
    page->setIcon(KIcon("preferences-desktop-color"));
    addPage(page);
    m_colorSettings = new ColorSettingsTab(vbox);

    // Performance
#if 0
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Performance"));
    page->setHeader(i18n("Performance"));
    page->setIcon(KIcon("preferences-system-performance"));
    addPage(page);
    m_performanceSettings = new PerformanceTab(vbox);
#endif

    // Grid
    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Grid"));
    page->setHeader(i18n("Grid"));
    page->setIcon(KIcon(BarIcon("grid", KIconLoader::SizeMedium)));
    addPage(page);
    m_gridSettings = new GridSettingsTab(vbox);

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

        cfg.setAutoSaveInterval(dialog->m_general->autoSaveInterval());
        foreach(KoDocument* doc, *KoDocument::documentList()) {
            doc->setAutoSave(dialog->m_general->autoSaveInterval());
        }

        // Color settings
        cfg.setMonitorProfile(dialog->m_colorSettings->m_page->cmbMonitorProfile->itemHighlighted());
        cfg.setWorkingColorSpace(dialog->m_colorSettings->m_page->cmbWorkingColorSpace->currentItem().id());
        cfg.setPrinterColorSpace(dialog->m_colorSettings->m_page->cmbPrintingColorSpace->currentItem().id());
        cfg.setPrinterProfile(dialog->m_colorSettings->m_page->cmbPrintProfile->itemHighlighted());

        cfg.setUseBlackPointCompensation(dialog->m_colorSettings->m_page->chkBlackpoint->isChecked());
        cfg.setPasteBehaviour(dialog->m_colorSettings->m_pasteBehaviourGroup.checkedId());
        cfg.setRenderIntent(dialog->m_colorSettings->m_page->cmbMonitorIntent->currentIndex());

#if 0
        // it's scaled from 0 - 6, but the config is in 0 - 300
        cfg.setSwappiness(dialog->m_performanceSettings->m_swappiness->value() * 50);
        cfg.setMaxTilesInMem(dialog->m_performanceSettings->m_maxTiles->value());
        cfg.setUseProjections(dialog->m_performanceSettings->m_projection->isChecked());
        cfg.setNumProjectThreads(dialog->m_performanceSettings->intNumThreads->value());
        cfg.setProjectionChunkSize(dialog->m_performanceSettings->intChunkSize->value());
        cfg.setAggregateDirtyRegionsInPainter(dialog->m_performanceSettings->chkAggregateDirtyRegions->isChecked());
        cfg.setUseBoundingRectInProjection(dialog->m_performanceSettings->chkUseBoundingRect->isChecked());
        cfg.setUpdateAllOfQpainterCanvas(dialog->m_performanceSettings->chkUpdateAllOfQPainterCanvas->isChecked());
        cfg.setUseNearestNeighbour(dialog->m_performanceSettings->chkUseNearestNeighbour->isChecked());
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

#endif
        cfg.setCheckSize(dialog->m_displaySettings->intCheckSize->value());
        cfg.setScrollingCheckers(dialog->m_displaySettings->chkMoving->isChecked());
        cfg.setCheckersColor(dialog->m_displaySettings->colorChecks->color());
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

    }
    delete dialog;
    return baccept;
}

#include "kis_dlg_preferences.moc"
