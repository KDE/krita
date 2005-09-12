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

#include <qvbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qcursor.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qbuttongroup.h>
#include <qslider.h>

#include <klocale.h>
#include <knuminput.h>
#include <kfiledialog.h>
#include <kurlrequester.h>
#include <klineedit.h>
#include <kiconloader.h>

#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_dlg_preferences.h"
#include "kis_colorspace_registry.h"
#include "kis_colorspace.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"
#include "kis_profile.h"
#include "wdgcolorsettings.h"
#include "wdgperformancesettings.h"

// for the performance update
#include "tiles/kis_tilemanager.h"

GeneralTab::GeneralTab( QWidget *_parent, const char *_name )
    : QWidget( _parent, _name )
{
    // Layout
    QGridLayout* grid = new QGridLayout( this, 3, 1, KDialog::marginHint(), KDialog::spacingHint());

    QLabel* label;
    label = new QLabel(this, i18n("Cursor shape:"), this);
    grid -> addWidget(label, 0, 0);

    m_cmbCursorShape = new QComboBox(this);

// XXX: Why doesn't insertImten with a bitmap work?
//     m_cmbCursorShape -> insertItem(*KisCursor::brushCursor().bitmap(), "Tool icon");
//     m_cmbCursorShape -> insertItem(*KisCursor::crossCursor().bitmap(), "Crosshair");
//     m_cmbCursorShape -> insertItem(*KisCursor::arrowCursor().bitmap(), "Arrow");
//     m_cmbCursorShape -> insertItem("Brush shape");
    m_cmbCursorShape -> insertItem(i18n("Tool Icon"));
    m_cmbCursorShape -> insertItem(i18n("Crosshair"));
    m_cmbCursorShape -> insertItem(i18n("Arrow"));

    KisConfig cfg;
    m_cmbCursorShape -> setCurrentItem(cfg.defCursorStyle());

    grid -> addWidget(m_cmbCursorShape, 1, 0);

    grid->setRowStretch( 2, 1 );
}

void GeneralTab::setDefault()
{
    m_cmbCursorShape -> setCurrentItem( CURSOR_STYLE_TOOLICON);
}

bool GeneralTab::saveOnExit()
{
    return m_saveOnExit->isChecked();
}

enumCursorStyle GeneralTab::cursorStyle()
{
    return (enumCursorStyle)m_cmbCursorShape -> currentItem();
}


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

    m_page -> cmbWorkingColorSpace -> setIDList(KisColorSpaceRegistry::instance() -> listKeys());
    m_page -> cmbWorkingColorSpace -> setCurrentText(cfg.workingColorSpace());

    m_page -> cmbPrintingColorSpace -> setIDList(KisColorSpaceRegistry::instance() -> listKeys());
    m_page -> cmbPrintingColorSpace -> setCurrentText(cfg.printerColorSpace());

    refillMonitorProfiles(KisID(cfg.workingColorSpace(), ""));
    refillPrintProfiles(KisID(cfg.printerColorSpace(), ""));
    refillImportProfiles(KisID(cfg.workingColorSpace(), ""));

    m_page -> cmbMonitorProfile -> setCurrentText(cfg.monitorProfile());
    m_page -> cmbImportProfile -> setCurrentText(cfg.importProfile());
    m_page -> cmbPrintProfile -> setCurrentText(cfg.printerProfile());
    m_page -> chkBlackpoint -> setChecked(cfg.useBlackPointCompensation());
    m_page -> chkDither8Bit -> setChecked(cfg.dither8Bit());
    m_page -> chkAskOpen -> setChecked(cfg.askProfileOnOpen());
    m_page -> chkAskPaste -> setChecked(cfg.askProfileOnPaste());
    m_page -> chkApplyMonitorOnCopy -> setChecked(cfg.applyMonitorProfileOnCopy());
    m_page -> grpIntent -> setButton(cfg.renderIntent());

    connect(m_page -> cmbWorkingColorSpace, SIGNAL(activated(const KisID &)),
            this, SLOT(refillMonitorProfiles(const KisID &)));

    connect(m_page -> cmbWorkingColorSpace, SIGNAL(activated(const KisID &)),
            this, SLOT(refillImportProfiles(const KisID &)));

    connect(m_page -> cmbPrintingColorSpace, SIGNAL(activated(const KisID &)),
            this, SLOT(refillPrintProfiles(const KisID &)));


}

void ColorSettingsTab::setDefault()
{
    //TODO
    m_page -> cmbWorkingColorSpace -> setCurrentText("RGBA");

    m_page -> cmbPrintingColorSpace -> setCurrentText("CMYK");

    m_page -> cmbMonitorProfile -> setCurrentText("None");
    m_page -> cmbImportProfile -> setCurrentText("None");
    m_page -> cmbPrintProfile -> setCurrentText("None");
    m_page -> chkBlackpoint -> setChecked(false);
    m_page -> chkDither8Bit -> setChecked(false);
    m_page -> chkAskOpen -> setChecked(true);
    m_page -> chkAskPaste -> setChecked(true);
    m_page -> chkApplyMonitorOnCopy -> setChecked(false);
    m_page -> grpIntent -> setButton(INTENT_PERCEPTUAL);
}


void ColorSettingsTab::refillMonitorProfiles(const KisID & s)
{
    KisColorSpace * cs = KisColorSpaceRegistry::instance() -> get(s);

    m_page -> cmbMonitorProfile -> clear();
    m_page -> cmbMonitorProfile -> insertItem(i18n("None"));

    if ( !cs )
    return;

    QValueVector<KisProfile *>  profileList = cs -> profiles();
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
        if ((*it) -> deviceClass() == icSigDisplayClass)
            m_page -> cmbMonitorProfile -> insertItem((*it) -> productName());
    }

}

void ColorSettingsTab::refillPrintProfiles(const KisID & s)
{
    KisColorSpace * cs = KisColorSpaceRegistry::instance() -> get(s);
    m_page -> cmbPrintProfile -> clear();
    m_page -> cmbPrintProfile -> insertItem(i18n("None"));
    if ( !cs )
        return;
    QValueVector<KisProfile *>  profileList = cs -> profiles();
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
        if ((*it) -> deviceClass() == icSigOutputClass)
            m_page -> cmbPrintProfile -> insertItem((*it) -> productName());
    }

}

void ColorSettingsTab::refillImportProfiles(const KisID & s)
{
    KisColorSpace * cs = KisColorSpaceRegistry::instance() -> get(s);
    m_page -> cmbImportProfile -> clear();
    m_page -> cmbImportProfile -> insertItem(i18n("None"));
    if ( !cs )
        return;
    QValueVector<KisProfile *>  profileList = cs -> profiles();
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
        if ((*it) -> deviceClass() == icSigInputClass)
            m_page -> cmbImportProfile -> insertItem((*it) -> productName());
    }
}

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




PreferencesDialog::PreferencesDialog( QWidget* parent, const char* name )
    : KDialogBase( IconList, i18n("Preferences"), Ok | Cancel | Help | Default | Apply, Ok, parent, name, true, true )
{
    QVBox *vbox;

    vbox = addVBoxPage( i18n( "General"), i18n( "General"), BarIcon( "misc", KIcon::SizeMedium ));
    m_general = new GeneralTab( vbox );

    vbox = addVBoxPage( i18n( "Colormanagement"), i18n( "Color"), BarIcon( "colorize", KIcon::SizeMedium ));
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
    m_general->setDefault();
    //m_directories->setDefault();
    //m_undoRedo->setDefault();
    m_colorSettings->setDefault();
}

bool PreferencesDialog::editPreferences()
{
    PreferencesDialog* dialog;

    dialog = new PreferencesDialog();
        bool baccept = ( dialog->exec() == Accepted );
    if( baccept )
    {
         KisConfig cfg;
         cfg.defCursorStyle(dialog -> m_general -> cursorStyle());

        // Color settings
        cfg.setMonitorProfile( dialog -> m_colorSettings -> m_page -> cmbMonitorProfile -> currentText());
        cfg.setWorkingColorSpace( dialog -> m_colorSettings -> m_page -> cmbWorkingColorSpace -> currentText());
        cfg.setImportProfile( dialog -> m_colorSettings -> m_page -> cmbImportProfile -> currentText());
        cfg.setPrinterColorSpace( dialog -> m_colorSettings -> m_page -> cmbPrintingColorSpace -> currentText());
        cfg.setPrinterProfile( dialog -> m_colorSettings -> m_page -> cmbPrintProfile -> currentText());

        cfg.setUseBlackPointCompensation( dialog -> m_colorSettings -> m_page -> chkBlackpoint -> isChecked());
        cfg.setDither8Bit( dialog -> m_colorSettings -> m_page -> chkDither8Bit -> isChecked());
        cfg.setAskProfileOnOpen( dialog -> m_colorSettings -> m_page -> chkAskOpen -> isChecked());
        cfg.setAskProfileOnPaste( dialog -> m_colorSettings -> m_page -> chkAskPaste -> isChecked());
        cfg.setApplyMonitorProfileOnCopy( dialog -> m_colorSettings -> m_page -> chkApplyMonitorOnCopy -> isChecked());
        cfg.setRenderIntent( dialog -> m_colorSettings -> m_page -> grpIntent -> selectedId());

        // it's scaled from 0 - 6, but the config is in 0 - 300
        cfg.setSwappiness(dialog -> m_performanceSettings -> m_swappiness -> value() * 50);
        cfg.setMaxTilesInMem(dialog -> m_performanceSettings -> m_maxTiles -> value());
        // let the tile manager know
        KisTileManager::instance() -> configChanged();

        // Pressure sensitivity setting == between 0 and 99
        cfg.setPressureCorrection( 100 - dialog->m_pressureSettings->slPressure->value() );
    }
        delete dialog;
        return baccept;
}

#include "kis_dlg_preferences.moc"
