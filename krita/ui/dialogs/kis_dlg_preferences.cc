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

#include <config-opengl.h>
#include <config-glew.h>

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

#include <KoImageResource.h>
#include <colorprofiles/KoIccColorProfile.h>

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

#include "kis_factory2.h"
#include "KoID.h"

#include "KoColorProfile.h"

#ifdef HAVE_OPENGL
#include "opengl/kis_opengl.h"
#endif

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
}

void GeneralTab::setDefault()
{
    KisConfig cfg;

    m_cmbCursorShape->setCurrentIndex(cfg.getDefaultCursorStyle());
    chkShowRootLayer->setChecked(false);
}

enumCursorStyle GeneralTab::cursorStyle()
{
    return (enumCursorStyle)m_cmbCursorShape->currentIndex();
}

bool GeneralTab::showRootLayer()
{
    return chkShowRootLayer->isChecked();
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

    if (KoIccColorProfile * profile = KoIccColorProfile::getScreenProfile()) {
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
    KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->value(s.id());

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
    KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->value(s.id());

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
    intSubdivision->setValue(cfg.getGridSubdivisions());
    intOffsetX->setValue(cfg.getGridOffsetX());
    intOffsetY->setValue(cfg.getGridOffsetY());

    linkSpacingToggled(true);
    connect(bnLinkSpacing, SIGNAL(toggled(bool)), this, SLOT(linkSpacingToggled(bool)));

    connect(intHSpacing, SIGNAL(valueChanged(int)), this, SLOT(spinBoxHSpacingChanged(int)));
    connect(intVSpacing, SIGNAL(valueChanged(int)), this, SLOT(spinBoxVSpacingChanged(int)));


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
    intSubdivision->setValue(1);
    intOffsetX->setValue(0);
    intOffsetY->setValue(0);
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

    KoImageResource kir;
    if (b) {
        bnLinkSpacing->setIcon(QIcon(kir.chain()));
    } else {
        bnLinkSpacing->setIcon(QIcon(kir.chainBroken()));
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
    KVBox *vbox = new KVBox();
    KPageWidgetItem *page = new KPageWidgetItem(vbox, i18n("General"));
    page->setHeader(i18n("General"));
    page->setIcon(KIcon(BarIcon("configure", KIconLoader::SizeMedium)));
    addPage(page);
    m_general = new GeneralTab(vbox);

    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Display"));
    page->setHeader(i18n("Display"));
    page->setIcon(KIcon("preferences-desktop-display"));
    addPage(page);

    m_displaySettings = new DisplaySettingsTab(vbox);

    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Color Management"));
    page->setHeader(i18n("Color"));
    page->setIcon(KIcon("preferences-desktop-color"));
    addPage(page);
    m_colorSettings = new ColorSettingsTab(vbox);

    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Performance"));
    page->setHeader(i18n("Performance"));
    page->setIcon(KIcon("preferences-system-performance"));
    addPage(page);
    m_performanceSettings = new PerformanceTab(vbox);

    vbox = new KVBox();
    page = new KPageWidgetItem(vbox, i18n("Grid"));
    page->setHeader(i18n("Grid"));
    page->setIcon(KIcon(BarIcon("grid", KIconLoader::SizeMedium)));
    addPage(page);

    m_gridSettings = new GridSettingsTab(vbox);

}

KisDlgPreferences::~KisDlgPreferences()
{
}

void KisDlgPreferences::slotDefault()
{
    m_general->setDefault();
    m_colorSettings->setDefault();
    m_performanceSettings->setDefault();
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
        KisConfig cfg;
        cfg.setCursorStyle(dialog->m_general->cursorStyle());
        cfg.setShowRootLayer(dialog->m_general->showRootLayer());
        // Color settings
        cfg.setMonitorProfile(dialog->m_colorSettings->m_page->cmbMonitorProfile->itemHighlighted());
        cfg.setWorkingColorSpace(dialog->m_colorSettings->m_page->cmbWorkingColorSpace->currentText());
        cfg.setPrinterColorSpace(dialog->m_colorSettings->m_page->cmbPrintingColorSpace->currentText());
        cfg.setPrinterProfile(dialog->m_colorSettings->m_page->cmbPrintProfile->itemHighlighted());

        cfg.setUseBlackPointCompensation(dialog->m_colorSettings->m_page->chkBlackpoint->isChecked());
        cfg.setPasteBehaviour(dialog->m_colorSettings->m_pasteBehaviourGroup.checkedId());
        cfg.setRenderIntent(dialog->m_colorSettings->m_page->cmbMonitorIntent->currentIndex());

        // it's scaled from 0 - 6, but the config is in 0 - 300
        cfg.setSwappiness(dialog->m_performanceSettings->m_swappiness->value() * 50);
        cfg.setMaxTilesInMem(dialog->m_performanceSettings->m_maxTiles->value());
#if 0
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
        cfg.setUseOpenGL(dialog->m_displaySettings->cbUseOpenGL->isChecked());
        cfg.setUseOpenGLShaders(dialog->m_displaySettings->cbUseOpenGLShaders->isChecked());
#endif
        cfg.setCheckSize(dialog->m_displaySettings->intCheckSize->value());
        cfg.setScrollingCheckers(dialog->m_displaySettings->chkMoving->isChecked());
        cfg.setCheckersColor(dialog->m_displaySettings->colorChecks->color());
        // Grid settings
        cfg.setGridMainStyle(dialog->m_gridSettings->selectMainStyle->currentIndex());
        cfg.setGridSubdivisionStyle(dialog->m_gridSettings->selectSubdivisionStyle->currentIndex());

        cfg.setGridMainColor(dialog->m_gridSettings->colorMain->color());
        cfg.setGridSubdivisionColor(dialog->m_gridSettings->colorSubdivision->color());

        cfg.setGridHSpacing(dialog->m_gridSettings->intHSpacing->value());
        cfg.setGridVSpacing(dialog->m_gridSettings->intVSpacing->value());
        cfg.setGridSubdivisions(dialog->m_gridSettings->intSubdivision->value());
        cfg.setGridOffsetX(dialog->m_gridSettings->intOffsetX->value());
        cfg.setGridOffsetY(dialog->m_gridSettings->intOffsetY->value());

    }
    delete dialog;
    return baccept;
}

#include "kis_dlg_preferences.moc"
