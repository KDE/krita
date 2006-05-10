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

#include <QWidget>

#include <kdialogbase.h>

#include <kopalettemanager.h>

#include "kis_canvas.h"

#include "ui_wdggeneralsettings.h"
#include "ui_wdgtabletsettings.h"
#include "ui_wdgtabletdevicesettings.h"
#include "ui_wdgperformancesettings.h"
#include "ui_wdgdisplaysettings.h"
#include "ui_wdggridsettings.h"
#include "ui_wdgcolorsettings.h"
#include "ui_wdgperformancesettings.h"
#include "ui_wdggeneralsettings.h"

class QLineEdit;
class QCheckBox;
class KUrlRequester;
class KisCmbIDList;
class KisID;

/**
 *  "General"-tab for preferences dialog
 */

class WdgGeneralSettings : public QWidget, public Ui::WdgGeneralSettings
{
    Q_OBJECT

    public:
        WdgGeneralSettings(QWidget *parent, const char *name) : QWidget(parent) { setObjectName(name); setupUi(this); }
};

class GeneralTab : public WdgGeneralSettings
{
    Q_OBJECT

public:

    GeneralTab( QWidget *parent = 0, const char *name = 0 );

    enumCursorStyle cursorStyle();
    enumKoDockability dockability();
    float dockerFontSize();
    
    void setDefault();
private:
    QButtonGroup m_dockabilityGroup;
};

//=======================

class WdgColorSettings : public QWidget, public Ui::WdgColorSettings
{
    Q_OBJECT

    public:
        WdgColorSettings(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class ColorSettingsTab : public QWidget
{
    Q_OBJECT

public:

    ColorSettingsTab( QWidget *parent = 0, const char * name = 0 );

private slots:

    void refillMonitorProfiles(const KisID & s);
    void refillPrintProfiles(const KisID & s);

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
        WdgPerformanceSettings(QWidget *parent, const char *name) : QWidget(parent) { setObjectName(name); setupUi(this); }
};

class PerformanceTab : public WdgPerformanceSettings
{
Q_OBJECT

public:
    PerformanceTab( QWidget *parent = 0, const char *name = 0 );

public:
    void setDefault();
};

//=======================

class WdgTabletSettings : public QWidget, public Ui::WdgTabletSettings
{
    Q_OBJECT

    public:
        WdgTabletSettings(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class WdgTabletDeviceSettings : public QWidget, public Ui::WdgTabletDeviceSettings
{
    Q_OBJECT

    public:
        WdgTabletDeviceSettings(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

/**
 *  Tablet settings tab for preferences dialog
 */
class TabletSettingsTab : public WdgTabletSettings
{
Q_OBJECT

public:
    TabletSettingsTab( QWidget *parent = 0, const char *name = 0 );

public:
    void setDefault();
    void applySettings();

private slots:
    void slotActivateDevice(int deviceIndex);
    void slotSetDeviceEnabled(bool enabled);
    void slotConfigureDevice();
    void applyTabletDeviceSettings();

#ifdef EXTENDED_X11_TABLET_SUPPORT

private:
    class DeviceSettings {
    public:
        DeviceSettings(KisCanvasWidget::X11TabletDevice *tabletDevice, bool enabled, 
                       qint32 xAxis, qint32 yAxis, qint32 pressureAxis, 
                       qint32 xTiltAxis, qint32 yTiltAxis, qint32 wheelAxis,
                       qint32 toolIDAxis, qint32 serialNumberAxis);
        DeviceSettings();

        void applySettings();

        void setEnabled(bool enabled);
        bool enabled() const;
    
        qint32 numAxes() const;

        void setXAxis(qint32 axis);
        void setYAxis(qint32 axis);
        void setPressureAxis(qint32 axis);
        void setXTiltAxis(qint32 axis);
        void setYTiltAxis(qint32 axis);
        void setWheelAxis(qint32 axis);
        void setToolIDAxis(qint32 axis);
        void setSerialNumberAxis(qint32 axis);
    
        qint32 xAxis() const;
        qint32 yAxis() const;
        qint32 pressureAxis() const;
        qint32 xTiltAxis() const;
        qint32 yTiltAxis() const;
        qint32 wheelAxis() const;
        qint32 toolIDAxis() const;
        qint32 serialNumberAxis() const;
    
    private:
        KisCanvasWidget::X11TabletDevice *m_tabletDevice;

        bool m_enabled;
        qint32 m_xAxis;
        qint32 m_yAxis;
        qint32 m_pressureAxis;
        qint32 m_xTiltAxis;
        qint32 m_yTiltAxis;
        qint32 m_wheelAxis;
        qint32 m_toolIDAxis;
        qint32 m_serialNumberAxis;
    };

    class TabletDeviceSettingsDialog : public KDialog {
        typedef KDialog super;
    
    public:
        TabletDeviceSettingsDialog(const QString& deviceName,
                                   DeviceSettings settings,
                                   QWidget *parent = 0,
                                   const char *name = 0);
        virtual ~TabletDeviceSettingsDialog();

        DeviceSettings settings();
    
    private:
        WdgTabletDeviceSettings *m_page;
        DeviceSettings m_settings;
    };
    
    void initTabletDevices();

    Q3ValueVector<DeviceSettings> m_deviceSettings;
#endif
};

//=======================

class WdgDisplaySettings : public QWidget, public Ui::WdgDisplaySettings
{
    Q_OBJECT

    public:
        WdgDisplaySettings(QWidget *parent, const char *name) : QWidget(parent) { setObjectName(name); setupUi(this); }
};

/**
 *  Display settings tab for preferences dialog
 */
class DisplaySettingsTab : public WdgDisplaySettings
{
Q_OBJECT

public:
    DisplaySettingsTab( QWidget *parent = 0, const char *name = 0 );

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
        WdgGridSettingsBase(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class GridSettingsTab : public WdgGridSettingsBase {
    Q_OBJECT
    public:
        GridSettingsTab(QWidget* parent);
    public:
        void setDefault();
    private slots:
        void linkSpacingToggled(bool);
        void spinBoxHSpacingChanged(int );
        void spinBoxVSpacingChanged(int );
    private:
        bool m_linkSpacing;
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
    TabletSettingsTab * m_tabletSettings;
    DisplaySettingsTab * m_displaySettings;
    GridSettingsTab* m_gridSettings;

protected slots:

    void slotDefault();

};

#endif
