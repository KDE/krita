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
#include <QButtonGroup>
#include <QMap>
#include <QString>

#include <kpagedialog.h>
#include <kis_config.h>

#include "kis_global.h"
#include <KisSqueezedComboBox.h>

#include "ui_wdggeneralsettings.h"
#include "ui_wdgdisplaysettings.h"
#include "ui_wdgcolorsettings.h"
#include "ui_wdgtabletsettings.h"
#include "ui_wdgperformancesettings.h"
#include "ui_wdgfullscreensettings.h"
#include "KisShortcutsDialog.h"

class KoID;
class KisInputConfigurationPage;
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

    CursorStyle cursorStyle();
    OutlineStyle outlineStyle();

    KisConfig::SessionOnStartup sessionOnStartup() const;
    bool saveSessionOnQuit() const;

    bool showRootLayer();
    int autoSaveInterval();
    void setDefault();
    int undoStackSize();
    bool showOutlineWhilePainting();

    int mdiMode();
    int favoritePresets();
    bool showCanvasMessages();
    bool compressKra();
    bool useZip64();
    bool toolOptionsInDocker();
    bool kineticScrollingEnabled();
    int kineticScrollingGesture();
    int kineticScrollingSensitivity();
    bool kineticScrollingHiddenScrollbars();
    bool switchSelectionCtrlAlt();
    bool convertToImageColorspaceOnImport();

private Q_SLOTS:
    void getBackgroundImage();
    void clearBackgroundImage();

};



/**
 *  "Shortcuts" tab for preferences dialog
 */

class WdgShortcutSettings : public KisShortcutsDialog
{
    Q_OBJECT

public:
    WdgShortcutSettings(QWidget *parent)
        : KisShortcutsDialog(KisShortcutsEditor::AllActions,
                             KisShortcutsEditor::LetterShortcutsAllowed,
                             parent)
    { }
};

class KisActionsSnapshot;

class ShortcutSettingsTab : public QWidget
{
    Q_OBJECT

public:

    ShortcutSettingsTab(QWidget *parent = 0, const char *name = 0);
    ~ShortcutSettingsTab() override;

public:
    void setDefault();
    WdgShortcutSettings  *m_page;
    QScopedPointer<KisActionsSnapshot> m_snapshot;

public Q_SLOTS:
    void saveChanges();
    void cancelChanges();
};



/**
 *  "Color" tab for preferences dialog
 */

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

private Q_SLOTS:

    void refillMonitorProfiles(const KoID & s);
    void installProfile();
    void toggleAllowMonitorProfileSelection(bool useSystemProfile);

public:
    void setDefault();
    WdgColorSettings  *m_page;
    QButtonGroup m_pasteBehaviourGroup;
    QList<QLabel*> m_monitorProfileLabels;
    QList<KisSqueezedComboBox*> m_monitorProfileWidgets;
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

private Q_SLOTS:
    void slotTabletTest();

public:
    void setDefault();
    WdgTabletSettings  *m_page;


};

//=======================

/**
  * "Performance"-tab for preferences dialog
 */

class SliderAndSpinBoxSync;

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

    ~PerformanceTab() override;

    void load(bool requestDefault);
    void save();

private Q_SLOTS:

    void selectSwapDir();

    void slotThreadsLimitChanged(int value);
    void slotFrameClonesLimitChanged(int value);

private:
    int realTilesRAM();

private:
    QVector<SliderAndSpinBoxSync*> m_syncs;
    int m_lastUsedThreadsLimit;
    int m_lastUsedClonesLimit;
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
protected Q_SLOTS:
    void slotUseOpenGLToggled(bool isChecked);
    void slotPreferredSurfaceFormatChanged(int index);

public:
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
    ~KisDlgPreferences() override;

    void showEvent(QShowEvent *event) override;

protected:

    GeneralTab *m_general;
    ShortcutSettingsTab  *m_shortcutSettings;
    ColorSettingsTab *m_colorSettings;
    PerformanceTab *m_performanceSettings;
    DisplaySettingsTab  *m_displaySettings;
    TabletSettingsTab *m_tabletSettings;
    FullscreenSettingsTab *m_fullscreenSettings;
    KisInputConfigurationPage *m_inputConfiguration;
    KoConfigAuthorPage *m_authorPage;

    QList<KPageWidgetItem*> m_pages;

protected Q_SLOTS:

    void slotDefault();

};

#endif
