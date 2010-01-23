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

#ifndef _KIS_DLG_PREFERENCES_H_
#define _KIS_DLG_PREFERENCES_H_

#include <QWidget>

#include <kpagedialog.h>

#include "kis_global.h"

#include "ui_wdggeneralsettings.h"
#include "ui_wdgdisplaysettings.h"
#include "ui_wdggridsettings.h"
#include "ui_wdgcolorsettings.h"
#include "ui_wdgperformancesettings.h"

class KoID;

/**
 *  "General"-tab for preferences dialog
 */

class WdgGeneralSettings : public QWidget, public Ui::WdgGeneralSettings
{
    Q_OBJECT

public:
    WdgGeneralSettings(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name); setupUi(this);
    }
};

class GeneralTab : public WdgGeneralSettings
{
    Q_OBJECT

public:

    GeneralTab(QWidget *parent = 0, const char *name = 0);

    enumCursorStyle cursorStyle();
    bool showRootLayer();
    int autoSaveInterval();
    void setDefault();

};

//=======================

class WdgColorSettings : public QWidget, public Ui::WdgColorSettings
{
    Q_OBJECT

public:
    WdgColorSettings(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class ColorSettingsTab : public QWidget
{
    Q_OBJECT

public:

    ColorSettingsTab(QWidget *parent = 0, const char * name = 0);

private slots:

    void refillMonitorProfiles(const KoID & s);
    void refillPrintProfiles(const KoID & s);

public:
    void setDefault();
    WdgColorSettings * m_page;
    QButtonGroup m_pasteBehaviourGroup;
};


/**
 *  "Performance"-tab for preferences dialog
 */

class WdgPerformanceSettings : public QWidget, public Ui::WdgPerformanceSettings
{
    Q_OBJECT

public:
    WdgPerformanceSettings(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name); setupUi(this);
    }
};

class PerformanceTab : public WdgPerformanceSettings
{
    Q_OBJECT

public:
    PerformanceTab(QWidget *parent = 0, const char *name = 0);

public:
    void setDefault();
};

//=======================

class WdgDisplaySettings : public QWidget, public Ui::WdgDisplaySettings
{
    Q_OBJECT

public:
    WdgDisplaySettings(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name); setupUi(this);
    }
};

/**
 *  Display settings tab for preferences dialog
 */
class DisplaySettingsTab : public WdgDisplaySettings
{
    Q_OBJECT

public:
    DisplaySettingsTab(QWidget *parent = 0, const char *name = 0);

public:
    void setDefault();
protected slots:
    void slotUseOpenGLToggled(bool isChecked);
};

//=======================

/**
 *  Grid settings tab for preferences dialog
 */

class WdgGridSettingsBase : public QWidget, public Ui::WdgGridSettingsBase
{
    Q_OBJECT

public:
    WdgGridSettingsBase(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class GridSettingsTab : public WdgGridSettingsBase
{
    Q_OBJECT
public:
    GridSettingsTab(QWidget* parent);
public:
    void setDefault();
private slots:
    void linkSpacingToggled(bool);
    void spinBoxHSpacingChanged(int);
    void spinBoxVSpacingChanged(int);
    void linkOffsetToggled(bool);
    void spinBoxXOffsetChanged(int);
    void spinBoxYOffsetChanged(int);
private:
    bool m_linkSpacing, m_linkOffset;
};

//=======================


/**
 *  Preferences dialog of KImageShop^WKrayon^WKrita
 */
class KisDlgPreferences : public KPageDialog
{
    Q_OBJECT

public:

    static bool editPreferences();


protected:

    KisDlgPreferences(QWidget *parent = 0, const char *name = 0);
    ~KisDlgPreferences();

protected:

    GeneralTab* m_general;
    ColorSettingsTab* m_colorSettings;
    PerformanceTab* m_performanceSettings;
    DisplaySettingsTab * m_displaySettings;
    GridSettingsTab* m_gridSettings;

protected slots:

    void slotDefault();

};

#endif
