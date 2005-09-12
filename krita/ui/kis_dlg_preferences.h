/*
 *  preferencesdlg.h - part of KImageShop^WKrita
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

#ifndef __preferencesdlg_h__
#define __preferencesdlg_h__

#include <qwidget.h>

#include <kdialogbase.h>

#include "wdgpressuresettings.h"
#include "wdgperformancesettings.h"

class QLineEdit;
class QCheckBox;
class KURLRequester;
class WdgColorSettings;
class KisCmbIDList;
class KisID;

/**
 *  "General"-tab for preferences dialog
 */
class GeneralTab : public QWidget
{
    Q_OBJECT

public:

    GeneralTab( QWidget *parent = 0, const char *name = 0 );

    bool saveOnExit();
    enumCursorStyle cursorStyle();
    void setDefault();

private:

    QCheckBox *m_saveOnExit;
    QComboBox *m_cmbCursorShape;
};

//=======================

class ColorSettingsTab : public QWidget
{
    Q_OBJECT

public:

    ColorSettingsTab( QWidget *parent = 0, const char * name = 0 );

private slots:

    void refillMonitorProfiles(const KisID & s);
    void refillPrintProfiles(const KisID & s);
    void refillImportProfiles(const KisID & s);

public:
    void setDefault();
    WdgColorSettings * m_page;
};


/**
 *  "Directories"-tab for preferences dialog
 */
class PerformanceTab : public WdgPerformanceSettings
{
Q_OBJECT

public:
    PerformanceTab( QWidget *parent = 0, const char *name = 0 );

public:
    void setDefault();
};

//=======================


/**
 *  pressure settings tab for preferences dialog
 */
class PressureSettingsTab : public WdgPressureSettings
{
Q_OBJECT

public:
    PressureSettingsTab( QWidget *parent = 0, const char *name = 0 );

public:
    void setDefault();
};

//=======================



/**
 *  Preferences dialog of KImageShop^WKrayon^WKrita
 */
class PreferencesDialog : public KDialogBase
{
    Q_OBJECT

public:

    static bool editPreferences();


protected:

    PreferencesDialog( QWidget *parent = 0, const char *name = 0 );
    ~PreferencesDialog();

protected:

    GeneralTab* m_general;
    ColorSettingsTab* m_colorSettings;
    PerformanceTab* m_performanceSettings;
    PressureSettingsTab * m_pressureSettings;

protected slots:

    void slotDefault();

};

#endif
