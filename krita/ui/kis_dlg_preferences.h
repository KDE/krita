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

#include <kopalettemanager.h>

#include "kis_canvas.h"

#include "wdggeneralsettings.h"
#include "wdgtabletsettings.h"
#include "wdgtabletdevicesettings.h"
#include "wdgperformancesettings.h"
#include "wdgdisplaysettings.h"
#include "wdggridsettings.h"

class QLineEdit;
class QCheckBox;
class KURLRequester;
class WdgColorSettings;
class KisCmbIDList;
class KisID;

/**
 *  "General"-tab for preferences dialog
 */
class GeneralTab : public WdgGeneralSettings
{
    Q_OBJECT

public:

    GeneralTab( QWidget *parent = 0, const char *name = 0 );

    enumCursorStyle cursorStyle();
    enumKoDockability dockability();
    float dockerFontSize();
    
    void setDefault();

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

public:
    void setDefault();
    WdgColorSettings * m_page;
};


/**
 *  "Performance"-tab for preferences dialog
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
                       Q_INT32 xAxis, Q_INT32 yAxis, Q_INT32 pressureAxis, 
                       Q_INT32 xTiltAxis, Q_INT32 yTiltAxis, Q_INT32 wheelAxis,
                       Q_INT32 toolIDAxis, Q_INT32 serialNumberAxis);
        DeviceSettings();

        void applySettings();

        void setEnabled(bool enabled);
        bool enabled() const;
    
        Q_INT32 numAxes() const;

        void setXAxis(Q_INT32 axis);
        void setYAxis(Q_INT32 axis);
        void setPressureAxis(Q_INT32 axis);
        void setXTiltAxis(Q_INT32 axis);
        void setYTiltAxis(Q_INT32 axis);
        void setWheelAxis(Q_INT32 axis);
        void setToolIDAxis(Q_INT32 axis);
        void setSerialNumberAxis(Q_INT32 axis);
    
        Q_INT32 xAxis() const;
        Q_INT32 yAxis() const;
        Q_INT32 pressureAxis() const;
        Q_INT32 xTiltAxis() const;
        Q_INT32 yTiltAxis() const;
        Q_INT32 wheelAxis() const;
        Q_INT32 toolIDAxis() const;
        Q_INT32 serialNumberAxis() const;
    
    private:
        KisCanvasWidget::X11TabletDevice *m_tabletDevice;

        bool m_enabled;
        Q_INT32 m_xAxis;
        Q_INT32 m_yAxis;
        Q_INT32 m_pressureAxis;
        Q_INT32 m_xTiltAxis;
        Q_INT32 m_yTiltAxis;
        Q_INT32 m_wheelAxis;
        Q_INT32 m_toolIDAxis;
        Q_INT32 m_serialNumberAxis;
    };

    class TabletDeviceSettingsDialog : public KDialogBase {
        typedef KDialogBase super;
    
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

    QValueVector<DeviceSettings> m_deviceSettings;
#endif
};

//=======================


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
