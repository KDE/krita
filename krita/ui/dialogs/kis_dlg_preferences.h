/*
 *  preferencesdlg.h - part of KImageShop^WKrita
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

#ifndef _KIS_DLG_PREFERENCES_H_
#define _KIS_DLG_PREFERENCES_H_

#include <QWidget>

#include <kpagedialog.h>

#include "kis_global.h"

#include "ui_wdggeneralsettings.h"
#include "ui_wdgdisplaysettings.h"
#include "ui_wdggridsettings.h"
#include "ui_wdgcolorsettings.h"
#include "ui_wdgtabletsettings.h"
#include "ui_wdgperformancesettings.h"
#include "ui_wdgfullscreensettings.h"

class KoID;
class KoConfigAuthorPage;

/**
 *  "General"-tab for preferences dialog
 */

class WdgGeneralSettings : public QWidget, public Ui::WdgGeneralSettings
{
    Q_OBJECT

public:
    WdgGeneralSettings(QWidget *parent, const char *name) : QWidget(parent) {
        setObjectName(name);
        setupUi(this);
        chkShowRootLayer->setVisible(false);
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
    int undoStackSize();
    bool showOutlineWhilePainting();

 private slots:

    void tagBackendChange(bool on);

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

    ColorSettingsTab(QWidget *parent = 0, const char  *name = 0);

private slots:

    void refillMonitorProfiles(const KoID & s);
    void refillPrintProfiles(const KoID & s);
    void selectOcioConfigPath();
    void enableOcioConfigPath(bool);
public:
    void setDefault();
    WdgColorSettings  *m_page;
    QButtonGroup m_pasteBehaviourGroup;
};

//=======================

class WdgTabletSettings : public QWidget, public Ui::WdgTabletSettings {

    Q_OBJECT

public:
    WdgTabletSettings(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }

};

class TabletSettingsTab : public QWidget {
    Q_OBJECT
public:
    TabletSettingsTab(QWidget *parent = 0, const char  *name = 0);

public:
    void setDefault();
    WdgTabletSettings  *m_page;


};

//=======================

/**
  * "Performance"-tab for preferences dialog
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
    GridSettingsTab(QWidget *parent);
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
 *  Full screen settings tab for preferences dialog
 */

class WdgFullscreenSettingsBase : public QWidget, public Ui::WdgFullscreenSettings
{
    Q_OBJECT

public:
    WdgFullscreenSettingsBase(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

class FullscreenSettingsTab : public WdgFullscreenSettingsBase
{
    Q_OBJECT
public:
    FullscreenSettingsTab(QWidget *parent);
public:
    void setDefault();
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

    GeneralTab *m_general;
    ColorSettingsTab *m_colorSettings;
    PerformanceTab *m_performanceSettings;
    DisplaySettingsTab  *m_displaySettings;
    GridSettingsTab *m_gridSettings;
    TabletSettingsTab *m_tabletSettings;
    FullscreenSettingsTab *m_fullscreenSettings;
    KoConfigAuthorPage *m_authorSettings;

protected slots:

    void slotDefault();

};

#endif
