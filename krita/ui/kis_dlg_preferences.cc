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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qslider.h>
#ifdef HAVE_GL
#include <qgl.h>
#endif

#include <klocale.h>
#include <knuminput.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kcombobox.h>

#include <kis_meta_registry.h>
#include "kis_factory.h"
#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_dlg_preferences.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_colorspace.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"
#include "kis_profile.h"
#include "wdgcolorsettings.h"
#include "wdgperformancesettings.h"
#include "wdggeneralsettings.h"

// for the performance update
#include "tiles/kis_tilemanager.h"

GeneralTab::GeneralTab( QWidget *_parent, const char *_name )
    : WdgGeneralSettings( _parent, _name )
{

    KisConfig cfg;

    m_cmbCursorShape -> setCurrentItem(cfg.cursorStyle());
    grpDockability->setButton(cfg.dockability());
    numDockerFontSize->setValue((int)cfg.dockerFontSize());
}

void GeneralTab::setDefault()
{
    KisConfig cfg;

    m_cmbCursorShape->setCurrentItem( cfg.getDefaultCursorStyle());
    grpDockability->setButton(cfg.getDefaultDockability());
    numDockerFontSize->setValue((int)(cfg.getDefaultDockerFontSize()));
}

enumCursorStyle GeneralTab::cursorStyle()
{
    return (enumCursorStyle)m_cmbCursorShape -> currentItem();
}

enumKoDockability GeneralTab::dockability()
{
    return (enumKoDockability)grpDockability->selectedId();
}

float GeneralTab::dockerFontSize()
{
    return (float)numDockerFontSize->value();
}

//---------------------------------------------------------------------------------------------------

ColorSettingsTab::ColorSettingsTab(QWidget *parent, const char *name  )
    : QWidget(parent, name)
{
    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    QGridLayout * l = new QGridLayout( this, 1, 1, KDialog::marginHint(), KDialog::spacingHint());
    l->setMargin(0);
    m_page = new WdgColorSettings(this);
    l -> addWidget( m_page, 0, 0);

    KisConfig cfg;

    m_page -> cmbWorkingColorSpace -> setIDList(KisMetaRegistry::instance()->csRegistry() -> listKeys());
    m_page -> cmbWorkingColorSpace -> setCurrentText(cfg.workingColorSpace());

    m_page -> cmbPrintingColorSpace -> setIDList(KisMetaRegistry::instance()->csRegistry() -> listKeys());
    m_page -> cmbPrintingColorSpace -> setCurrentText(cfg.printerColorSpace());

    refillMonitorProfiles(KisID("RGBA", ""));
    refillPrintProfiles(KisID(cfg.printerColorSpace(), ""));

    if(m_page -> cmbMonitorProfile -> contains(cfg.monitorProfile()))
        m_page -> cmbMonitorProfile -> setCurrentText(cfg.monitorProfile());
    if(m_page -> cmbPrintProfile -> contains(cfg.printerProfile()))
        m_page -> cmbPrintProfile -> setCurrentText(cfg.printerProfile());
    m_page -> chkBlackpoint -> setChecked(cfg.useBlackPointCompensation());
    m_page -> grpPasteBehaviour -> setButton(cfg.pasteBehaviour());
    m_page -> cmbMonitorIntent -> setCurrentItem(cfg.renderIntent());

    connect(m_page -> cmbPrintingColorSpace, SIGNAL(activated(const KisID &)),
            this, SLOT(refillPrintProfiles(const KisID &)));
}

void ColorSettingsTab::setDefault()
{
    m_page -> cmbWorkingColorSpace -> setCurrentText("RGBA");

    m_page -> cmbPrintingColorSpace -> setCurrentText("CMYK");
    refillPrintProfiles(KisID("CMYK", ""));

    m_page -> chkBlackpoint -> setChecked(false);
    m_page -> cmbMonitorIntent -> setCurrentItem(INTENT_PERCEPTUAL);
    m_page -> grpPasteBehaviour -> setButton(2);
}


void ColorSettingsTab::refillMonitorProfiles(const KisID & s)
{
    KisColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry() -> get(s);

    m_page -> cmbMonitorProfile -> clear();

    if ( !csf )
    return;

    QValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
            if ((*it) -> deviceClass() == icSigDisplayClass)
                m_page -> cmbMonitorProfile -> insertItem((*it) -> productName());
    }

    m_page -> cmbMonitorProfile -> setCurrentText(csf->defaultProfile());
}

void ColorSettingsTab::refillPrintProfiles(const KisID & s)
{
    KisColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry() -> get(s);

    m_page -> cmbPrintProfile -> clear();

    if ( !csf )
        return;

    QValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
            if ((*it) -> deviceClass() == icSigOutputClass)
                m_page -> cmbPrintProfile -> insertItem((*it) -> productName());
    }

    m_page -> cmbPrintProfile -> setCurrentText(csf->defaultProfile());
}

//---------------------------------------------------------------------------------------------------

PerformanceTab::PerformanceTab(QWidget *parent, const char *name  )
    : WdgPerformanceSettings(parent, name)
{
    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    KisConfig cfg;

    // it's scaled from 0 - 6, but the config is in 0 - 300
    m_swappiness -> setValue(cfg.swappiness() / 50);
    m_maxTiles -> setValue(cfg.maxTilesInMem());
}

void PerformanceTab::setDefault()
{
    m_swappiness -> setValue(3);
    m_maxTiles -> setValue(500);
}

//---------------------------------------------------------------------------------------------------

PressureSettingsTab::PressureSettingsTab( QWidget *parent, const char *name)
    : WdgPressureSettings( parent, name )
{
    KisConfig cfg;
    // XXX: Bad me -- hard-coded constant.
    slPressure->setValue( 100 - cfg.getPressureCorrection() );
}

void PressureSettingsTab::setDefault()
{
    KisConfig cfg;
    // XXX: Bad me -- hard-coded constant.
    slPressure->setValue(100 - cfg.getDefaultPressureCorrection());
}

DisplaySettingsTab::DisplaySettingsTab( QWidget *parent, const char *name)
    : WdgDisplaySettings( parent, name )
{
#ifdef HAVE_GL
    KisConfig cfg;

    if (!QGLFormat::hasOpenGL()) {
        cbUseOpenGL -> setEnabled(false);
        cbUseOpenGLShaders -> setEnabled(false);
    } else {
        cbUseOpenGL -> setChecked(cfg.useOpenGL());
        cbUseOpenGLShaders -> setChecked(cfg.useOpenGLShaders());
        cbUseOpenGLShaders -> setEnabled(cfg.useOpenGL());
    }
#else
    cbUseOpenGL -> setEnabled(false);
    cbUseOpenGLShaders -> setEnabled(false);
#endif

    connect(cbUseOpenGL, SIGNAL(toggled(bool)), SLOT(slotUseOpenGLToggled(bool)));
}

void DisplaySettingsTab::setDefault()
{
    cbUseOpenGL -> setChecked(false);
    cbUseOpenGLShaders -> setChecked(false);
    cbUseOpenGLShaders -> setEnabled(false);
}

void DisplaySettingsTab::slotUseOpenGLToggled(bool isChecked)
{
    cbUseOpenGLShaders -> setEnabled(isChecked);
}

//---------------------------------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog( QWidget* parent, const char* name )
    : KDialogBase( IconList, i18n("Preferences"), Ok | Cancel | Help | Default | Apply, Ok, parent, name, true, true )
{
    QVBox *vbox;

    vbox = addVBoxPage( i18n( "General"), i18n( "General"), BarIcon( "misc", KIcon::SizeMedium ));
    m_general = new GeneralTab( vbox );

    vbox = addVBoxPage ( i18n( "Display" ), i18n( "Display" ), BarIcon( "kscreensaver", KIcon::SizeMedium ));
    m_displaySettings = new DisplaySettingsTab( vbox );

    vbox = addVBoxPage( i18n( "Color Management"), i18n( "Color"), BarIcon( "colorize", KIcon::SizeMedium ));
    m_colorSettings = new ColorSettingsTab( vbox );

    vbox = addVBoxPage( i18n( "Performance"), i18n( "Performance"), BarIcon( "fork", KIcon::SizeMedium ));
    m_performanceSettings = new PerformanceTab ( vbox );

    vbox = addVBoxPage ( i18n( "Pressure" ), i18n( "Pressure" ), BarIcon( "tablet", KIcon::SizeMedium ));
    m_pressureSettings = new PressureSettingsTab( vbox );

}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::slotDefault()
{
    m_general -> setDefault();
    m_colorSettings -> setDefault();
    m_pressureSettings -> setDefault();
    m_performanceSettings -> setDefault();
    m_displaySettings -> setDefault();
}

bool PreferencesDialog::editPreferences()
{
    PreferencesDialog* dialog;

    dialog = new PreferencesDialog();
        bool baccept = ( dialog->exec() == Accepted );
    if( baccept )
    {
        KisConfig cfg;
        cfg.setCursorStyle(dialog -> m_general -> cursorStyle());
        cfg.setDockability( dialog->m_general->dockability() );
        cfg.setDockerFontSize( dialog->m_general->dockerFontSize() );

        // Color settings
        cfg.setMonitorProfile( dialog -> m_colorSettings -> m_page -> cmbMonitorProfile -> currentText());
        cfg.setWorkingColorSpace( dialog -> m_colorSettings -> m_page -> cmbWorkingColorSpace -> currentText());
        cfg.setPrinterColorSpace( dialog -> m_colorSettings -> m_page -> cmbPrintingColorSpace -> currentText());
        cfg.setPrinterProfile( dialog -> m_colorSettings -> m_page -> cmbPrintProfile -> currentText());

        cfg.setUseBlackPointCompensation( dialog -> m_colorSettings -> m_page -> chkBlackpoint -> isChecked());
        cfg.setPasteBehaviour( dialog -> m_colorSettings -> m_page -> grpPasteBehaviour->selectedId());
        cfg.setRenderIntent( dialog -> m_colorSettings -> m_page -> cmbMonitorIntent -> currentItem());

        // it's scaled from 0 - 6, but the config is in 0 - 300
        cfg.setSwappiness(dialog -> m_performanceSettings -> m_swappiness -> value() * 50);
        cfg.setMaxTilesInMem(dialog -> m_performanceSettings -> m_maxTiles -> value());
        // let the tile manager know
        KisTileManager::instance() -> configChanged();

        // Pressure sensitivity setting == between 0 and 99
        cfg.setPressureCorrection( 100 - dialog->m_pressureSettings->slPressure->value() );

        cfg.setUseOpenGL(dialog -> m_displaySettings -> cbUseOpenGL -> isChecked());
        cfg.setUseOpenGLShaders(dialog -> m_displaySettings -> cbUseOpenGLShaders -> isChecked());
    }
        delete dialog;
        return baccept;
}

#include "kis_dlg_preferences.moc"
