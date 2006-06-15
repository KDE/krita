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

#include <config.h>
#include <config-krita.h>

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
#include <QGridLayout>

#ifdef HAVE_OPENGL
#include <qgl.h>
#endif

#include <KoImageResource.h>

#include <kcolorbutton.h>
#include <kcombobox.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kurlrequester.h>

#include "squeezedcombobox.h"
#include "kis_clipboard.h"
#include "kis_cmb_idlist.h"
#include "KoColorSpace.h"
#include "KoColorSpaceFactoryRegistry.h"
#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_dlg_preferences.h"
#include "kis_factory.h"
#include "KoID.h"
#include "kis_meta_registry.h"
#include "KoColorProfile.h"

#include "kis_canvas.h"

// for the performance update
#include "tiles/kis_tilemanager.h"

GeneralTab::GeneralTab( QWidget *_parent, const char *_name )
    : WdgGeneralSettings( _parent, _name )
{
    KisConfig cfg;

    m_dockabilityGroup.addButton(radioAllowDocking, DOCK_ENABLED);
    m_dockabilityGroup.addButton(radioDisallowDocking, DOCK_DISABLED);
    m_dockabilityGroup.addButton(radioSmartDocking, DOCK_SMART);

    kDebug() << "Dock is " << cfg.dockability() << endl;


    QAbstractButton *button = m_dockabilityGroup.button(cfg.dockability());
    Q_ASSERT(button);
    if (button) {
        button->setChecked(true);
    }

    m_cmbCursorShape->setCurrentIndex(cfg.cursorStyle());
    numDockerFontSize->setValue((int)cfg.dockerFontSize());
}

void GeneralTab::setDefault()
{
    KisConfig cfg;

    m_cmbCursorShape->setCurrentIndex(cfg.getDefaultCursorStyle());
    QAbstractButton *button = m_dockabilityGroup.button(cfg.getDefaultDockability());
    Q_ASSERT(button);
    if (button) {
        button->setChecked(true);
    }
    numDockerFontSize->setValue((int)(cfg.getDefaultDockerFontSize()));
}

enumCursorStyle GeneralTab::cursorStyle()
{
    return (enumCursorStyle)m_cmbCursorShape->currentIndex();
}

enumKoDockability GeneralTab::dockability()
{
    return (enumKoDockability)m_dockabilityGroup.checkedId();
}

float GeneralTab::dockerFontSize()
{
    return (float)numDockerFontSize->value();
}

//---------------------------------------------------------------------------------------------------

ColorSettingsTab::ColorSettingsTab(QWidget *parent, const char *name  )
    : QWidget(parent)
{
    setObjectName(name);

    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    QGridLayout * l = new QGridLayout(this);
    l->setSpacing(KDialog::spacingHint());
    l->setMargin(0);
    m_page = new WdgColorSettings(this);
    l->addWidget( m_page, 0, 0);

    KisConfig cfg;

    m_page->cmbWorkingColorSpace->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());
    m_page->cmbWorkingColorSpace->setCurrent(cfg.workingColorSpace());

    m_page->cmbPrintingColorSpace->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());
    m_page->cmbPrintingColorSpace->setCurrent(cfg.printerColorSpace());

    refillMonitorProfiles(KoID("RGBA", ""));
    refillPrintProfiles(KoID(cfg.printerColorSpace(), ""));

    if(m_page->cmbMonitorProfile->contains(cfg.monitorProfile()))
        m_page->cmbMonitorProfile->setCurrent(cfg.monitorProfile());
    if(m_page->cmbPrintProfile->contains(cfg.printerProfile()))
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
    KoColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);

    m_page->cmbMonitorProfile->clear();

    if ( !csf )
    return;

    QList<KoColorProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );

    foreach (KoColorProfile *profile, profileList) {
        if (profile->deviceClass() == icSigDisplayClass)
            m_page->cmbMonitorProfile->addSqueezedItem(profile->productName());
    }

    m_page->cmbMonitorProfile->setCurrent(csf->defaultProfile());
}

void ColorSettingsTab::refillPrintProfiles(const KoID & s)
{
    KoColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);

    m_page->cmbPrintProfile->clear();

    if ( !csf )
        return;

    QList<KoColorProfile *> profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );

    foreach (KoColorProfile *profile, profileList) {
        if (profile->deviceClass() == icSigOutputClass)
            m_page->cmbPrintProfile->addSqueezedItem(profile->productName());
    }

    m_page->cmbPrintProfile->setCurrent(csf->defaultProfile());
}

//---------------------------------------------------------------------------------------------------

PerformanceTab::PerformanceTab(QWidget *parent, const char *name  )
    : WdgPerformanceSettings(parent, name)
{
    // XXX: Make sure only profiles that fit the specified color model
    // are shown in the profile combos

    KisConfig cfg;

    // it's scaled from 0 - 6, but the config is in 0 - 300
    m_swappiness->setValue(cfg.swappiness() / 50);
    m_maxTiles->setValue(cfg.maxTilesInMem());
}

void PerformanceTab::setDefault()
{
    m_swappiness->setValue(3);
    m_maxTiles->setValue(500);
}

//---------------------------------------------------------------------------------------------------

TabletSettingsTab::TabletSettingsTab( QWidget *parent, const char *name)
    : WdgTabletSettings( parent )
{
    setObjectName(name);
    KisConfig cfg;
    // XXX: Bad me -- hard-coded constant.
    slPressure->setValue( 100 - cfg.getPressureCorrection() );

#ifdef EXTENDED_X11_TABLET_SUPPORT
    initTabletDevices();
#else
    grpTabletDevices->hide();
#endif
}

void TabletSettingsTab::setDefault()
{
    KisConfig cfg;
    // XXX: Bad me -- hard-coded constant.
    slPressure->setValue(100 - cfg.getDefaultPressureCorrection());
}

void TabletSettingsTab::applySettings()
{
    KisConfig cfg;

    // Pressure sensitivity setting == between 0 and 99
    cfg.setPressureCorrection(100 - slPressure->value());

#ifdef EXTENDED_X11_TABLET_SUPPORT
    applyTabletDeviceSettings();
#endif
}

#ifdef EXTENDED_X11_TABLET_SUPPORT
TabletSettingsTab::DeviceSettings::DeviceSettings(KisCanvasWidget::X11TabletDevice *tabletDevice, bool enabled,
                                                  qint32 xAxis, qint32 yAxis, qint32 pressureAxis,
                                                  qint32 xTiltAxis, qint32 yTiltAxis, qint32 wheelAxis,
                                                  qint32 toolIDAxis, qint32 serialNumberAxis)
    : m_tabletDevice(tabletDevice),
      m_enabled(enabled),
      m_xAxis(xAxis),
      m_yAxis(yAxis),
      m_pressureAxis(pressureAxis),
      m_xTiltAxis(xTiltAxis),
      m_yTiltAxis(yTiltAxis),
      m_wheelAxis(wheelAxis),
      m_toolIDAxis(toolIDAxis),
      m_serialNumberAxis(serialNumberAxis)
{
}

TabletSettingsTab::DeviceSettings::DeviceSettings()
    : m_tabletDevice(0),
      m_enabled(false),
      m_xAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_yAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_pressureAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_xTiltAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_yTiltAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_wheelAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_toolIDAxis(KisCanvasWidget::X11TabletDevice::NoAxis),
      m_serialNumberAxis(KisCanvasWidget::X11TabletDevice::NoAxis)
{
}

void TabletSettingsTab::DeviceSettings::applySettings()
{
    m_tabletDevice->setEnabled(enabled());
    m_tabletDevice->setXAxis(xAxis());
    m_tabletDevice->setYAxis(yAxis());
    m_tabletDevice->setPressureAxis(pressureAxis());
    m_tabletDevice->setXTiltAxis(xTiltAxis());
    m_tabletDevice->setYTiltAxis(yTiltAxis());
    m_tabletDevice->setWheelAxis(wheelAxis());
    m_tabletDevice->setToolIDAxis(toolIDAxis());
    m_tabletDevice->setSerialNumberAxis(serialNumberAxis());
    m_tabletDevice->writeSettingsToConfig();
}

void TabletSettingsTab::DeviceSettings::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool TabletSettingsTab::DeviceSettings::enabled() const
{
    return m_enabled;
}

qint32 TabletSettingsTab::DeviceSettings::numAxes() const
{
    return m_tabletDevice->numAxes();
}

void TabletSettingsTab::DeviceSettings::setXAxis(qint32 axis)
{
    m_xAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setYAxis(qint32 axis)
{
    m_yAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setPressureAxis(qint32 axis)
{
    m_pressureAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setXTiltAxis(qint32 axis)
{
    m_xTiltAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setYTiltAxis(qint32 axis)
{
    m_yTiltAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setWheelAxis(qint32 axis)
{
    m_wheelAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setToolIDAxis(qint32 axis)
{
    m_toolIDAxis = axis;
}

void TabletSettingsTab::DeviceSettings::setSerialNumberAxis(qint32 axis)
{
    m_serialNumberAxis = axis;
}

qint32 TabletSettingsTab::DeviceSettings::xAxis() const
{
    return m_xAxis;
}

qint32 TabletSettingsTab::DeviceSettings::yAxis() const
{
    return m_yAxis;
}

qint32 TabletSettingsTab::DeviceSettings::pressureAxis() const
{
    return m_pressureAxis;
}

qint32 TabletSettingsTab::DeviceSettings::xTiltAxis() const
{
    return m_xTiltAxis;
}

qint32 TabletSettingsTab::DeviceSettings::yTiltAxis() const
{
    return m_yTiltAxis;
}

qint32 TabletSettingsTab::DeviceSettings::wheelAxis() const
{
    return m_wheelAxis;
}

qint32 TabletSettingsTab::DeviceSettings::toolIDAxis() const
{
    return m_toolIDAxis;
}

qint32 TabletSettingsTab::DeviceSettings::serialNumberAxis() const
{
    return m_serialNumberAxis;
}

TabletSettingsTab::TabletDeviceSettingsDialog::TabletDeviceSettingsDialog(const QString& deviceName, DeviceSettings settings,
                                                                          QWidget *parent, const char *name)
    : super(parent, i18n("Configure %1",deviceName), Ok | Cancel)
{
    setObjectName(name);

    m_page = new WdgTabletDeviceSettings(this);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    for (qint32 axis = 0; axis < settings.numAxes(); axis++) {
        QString axisString;

        axisString.setNum(axis);

        m_page->cbX->addItem(axisString);
        m_page->cbY->addItem(axisString);
        m_page->cbPressure->addItem(axisString);
        m_page->cbXTilt->addItem(axisString);
        m_page->cbYTilt->addItem(axisString);
        m_page->cbWheel->addItem(axisString);
//         m_page->cbToolID->addItem(axisString);
//         m_page->cbSerialNumber->addItem(axisString);
    }

    m_page->cbX->addItem(i18n("None"));
    m_page->cbY->addItem(i18n("None"));
    m_page->cbPressure->addItem(i18n("None"));
    m_page->cbXTilt->addItem(i18n("None"));
    m_page->cbYTilt->addItem(i18n("None"));
    m_page->cbWheel->addItem(i18n("None"));
//     m_page->cbToolID->addItem(i18n("None"));
//     m_page->cbSerialNumber->addItem(i18n("None"));

    if (settings.xAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
        m_page->cbX->setCurrentIndex(settings.xAxis());
    } else {
        m_page->cbX->setCurrentIndex(settings.numAxes());
    }

    if (settings.yAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
        m_page->cbY->setCurrentIndex(settings.yAxis());
    } else {
        m_page->cbY->setCurrentIndex(settings.numAxes());
    }

    if (settings.pressureAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
        m_page->cbPressure->setCurrentIndex(settings.pressureAxis());
    } else {
        m_page->cbPressure->setCurrentIndex(settings.numAxes());
    }

    if (settings.xTiltAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
        m_page->cbXTilt->setCurrentIndex(settings.xTiltAxis());
    } else {
        m_page->cbXTilt->setCurrentIndex(settings.numAxes());
    }

    if (settings.yTiltAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
        m_page->cbYTilt->setCurrentIndex(settings.yTiltAxis());
    } else {
        m_page->cbYTilt->setCurrentIndex(settings.numAxes());
    }

    if (settings.wheelAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
        m_page->cbWheel->setCurrentIndex(settings.wheelAxis());
    } else {
        m_page->cbWheel->setCurrentIndex(settings.numAxes());
    }

//     if (settings.toolIDAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
//         m_page->cbToolID->setCurrentIndex(settings.toolIDAxis());
//     } else {
//         m_page->cbToolID->setCurrentIndex(settings.numAxes());
//     }
//
//     if (settings.serialNumberAxis() != KisCanvasWidget::X11TabletDevice::NoAxis) {
//         m_page->cbSerialNumber->setCurrentIndex(settings.serialNumberAxis());
//     } else {
//         m_page->cbSerialNumber->setCurrentIndex(settings.numAxes());
//     }

    m_settings = settings;
}

TabletSettingsTab::TabletDeviceSettingsDialog::~TabletDeviceSettingsDialog()
{
    delete m_page;
}

TabletSettingsTab::DeviceSettings TabletSettingsTab::TabletDeviceSettingsDialog::settings()
{
    const qint32 noAxis = m_settings.numAxes();

    if (m_page->cbX->currentIndex() != noAxis ) {
        m_settings.setXAxis(m_page->cbX->currentIndex());
    } else {
        m_settings.setXAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
    }

    if (m_page->cbY->currentIndex() != noAxis ) {
        m_settings.setYAxis(m_page->cbY->currentIndex());
    } else {
        m_settings.setYAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
    }

    if (m_page->cbPressure->currentIndex() != noAxis ) {
        m_settings.setPressureAxis(m_page->cbPressure->currentIndex());
    } else {
        m_settings.setPressureAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
    }

    if (m_page->cbXTilt->currentIndex() != noAxis ) {
        m_settings.setXTiltAxis(m_page->cbXTilt->currentIndex());
    } else {
        m_settings.setXTiltAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
    }

    if (m_page->cbYTilt->currentIndex() != noAxis ) {
        m_settings.setYTiltAxis(m_page->cbYTilt->currentIndex());
    } else {
        m_settings.setYTiltAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
    }

    if (m_page->cbWheel->currentIndex() != noAxis ) {
        m_settings.setWheelAxis(m_page->cbWheel->currentIndex());
    } else {
        m_settings.setWheelAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
    }

//     if (m_page->cbToolID->currentIndex() != noAxis ) {
//         m_settings.setToolIDAxis(m_page->cbToolID->currentIndex());
//     } else {
//         m_settings.setToolIDAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
//     }
//
//     if (m_page->cbSerialNumber->currentIndex() != noAxis ) {
//         m_settings.setSerialNumberAxis(m_page->cbSerialNumber->currentIndex());
//     } else {
//         m_settings.setSerialNumberAxis(KisCanvasWidget::X11TabletDevice::NoAxis);
//     }

    return m_settings;
}

void TabletSettingsTab::initTabletDevices()
{
    connect(cbTabletDevice, SIGNAL(activated(int)), SLOT(slotActivateDevice(int)));
    connect(chkEnableTabletDevice, SIGNAL(toggled(bool)), SLOT(slotSetDeviceEnabled(bool)));
    connect(btnConfigureTabletDevice, SIGNAL(clicked()), SLOT(slotConfigureDevice()));

    KisCanvasWidget::X11XIDTabletDeviceMap& tabletDevices = KisCanvasWidget::tabletDeviceMap();

    cbTabletDevice->clear();

    if (!tabletDevices.empty()) {
        KisCanvasWidget::X11XIDTabletDeviceMap::iterator it;

        for (it = tabletDevices.begin(); it != tabletDevices.end(); ++it) {
            KisCanvasWidget::X11TabletDevice& device = (*it).second;

            m_deviceSettings.append(DeviceSettings(&device, device.enabled(), device.xAxis(), device.yAxis(),
                                                   device.pressureAxis(), device.xTiltAxis(), device.yTiltAxis(), device.wheelAxis(),
                                                   device.toolIDAxis(), device.serialNumberAxis()));
            cbTabletDevice->addItem(device.name());
        }
        slotActivateDevice(0);
    } else {
        cbTabletDevice->addItem(i18n("No devices detected"));
        cbTabletDevice->setEnabled(false);
        chkEnableTabletDevice->setEnabled(false);
        btnConfigureTabletDevice->setEnabled(false);
    }
}

void TabletSettingsTab::slotActivateDevice(int deviceIndex)
{
    bool deviceEnabled = m_deviceSettings[deviceIndex].enabled();

    chkEnableTabletDevice->setChecked(deviceEnabled);
    slotSetDeviceEnabled(deviceEnabled);
}

void TabletSettingsTab::slotSetDeviceEnabled(bool enabled)
{
    btnConfigureTabletDevice->setEnabled(enabled);
    m_deviceSettings[cbTabletDevice->currentIndex()].setEnabled(enabled);
}

void TabletSettingsTab::slotConfigureDevice()
{
    TabletDeviceSettingsDialog dialog(cbTabletDevice->currentText(), m_deviceSettings[cbTabletDevice->currentIndex()],
                                      this, "TabletDeviceSettings");

    if (dialog.exec() == QDialog::Accepted)
    {
        m_deviceSettings[cbTabletDevice->currentIndex()] = dialog.settings();
    }
}

void TabletSettingsTab::applyTabletDeviceSettings()
{
    for (qint32 deviceIndex = 0; deviceIndex < m_deviceSettings.count(); ++deviceIndex) {
        m_deviceSettings[deviceIndex].applySettings();
    }
}

#else // EXTENDED_X11_TABLET_SUPPORT

// Fix compilation. moc seems to not see the undefined symbol needed
// for these slots to be declared.
void TabletSettingsTab::slotActivateDevice(int /*deviceIndex*/)
{
}

void TabletSettingsTab::slotSetDeviceEnabled(bool /*enabled*/)
{
}

void TabletSettingsTab::slotConfigureDevice()
{
}

void TabletSettingsTab::applyTabletDeviceSettings()
{
}

#endif

//---------------------------------------------------------------------------------------------------

DisplaySettingsTab::DisplaySettingsTab( QWidget *parent, const char *name)
    : WdgDisplaySettings( parent, name )
{
#ifdef HAVE_OPENGL
    KisConfig cfg;

    if (!QGLFormat::hasOpenGL()) {
        cbUseOpenGL->setEnabled(false);
        //cbUseOpenGLShaders->setEnabled(false);
    } else {
        cbUseOpenGL->setChecked(cfg.useOpenGL());
        //cbUseOpenGLShaders->setChecked(cfg.useOpenGLShaders());
        //cbUseOpenGLShaders->setEnabled(cfg.useOpenGL());
    }
#else
    cbUseOpenGL->setEnabled(false);
    //cbUseOpenGLShaders->setEnabled(false);
#endif

    connect(cbUseOpenGL, SIGNAL(toggled(bool)), SLOT(slotUseOpenGLToggled(bool)));
}

void DisplaySettingsTab::setDefault()
{
    cbUseOpenGL->setChecked(false);
    //cbUseOpenGLShaders->setChecked(false);
    //cbUseOpenGLShaders->setEnabled(false);
}

void DisplaySettingsTab::slotUseOpenGLToggled(bool /*isChecked*/)
{
    //cbUseOpenGLShaders->setEnabled(isChecked);
}

//---------------------------------------------------------------------------------------------------
GridSettingsTab::GridSettingsTab(QWidget* parent) : WdgGridSettingsBase(parent)
{
    KisConfig cfg;
    selectMainStyle->setCurrentIndex(cfg.getGridMainStyle());
    selectSubdivisionStyle->setCurrentIndex(cfg.getGridSubdivisionStyle());

    colorMain->setColor(cfg.getGridMainColor());
    colorSubdivision->setColor(cfg.getGridSubdivisionColor());

    intHSpacing->setValue( cfg.getGridHSpacing() );
    intVSpacing->setValue( cfg.getGridVSpacing() );
    intSubdivision->setValue( cfg.getGridSubdivisions());
    intOffsetX->setValue( cfg.getGridOffsetX());
    intOffsetY->setValue( cfg.getGridOffsetY());

    linkSpacingToggled(true);
    connect(bnLinkSpacing, SIGNAL(toggled(bool)), this, SLOT(linkSpacingToggled( bool )));

    connect(intHSpacing, SIGNAL(valueChanged(int)),this,SLOT(spinBoxHSpacingChanged(int)));
    connect(intVSpacing, SIGNAL(valueChanged(int)),this,SLOT(spinBoxVSpacingChanged(int)));


}

void GridSettingsTab::setDefault()
{
    KisConfig cfg;
    selectMainStyle->setCurrentIndex(0);
    selectSubdivisionStyle->setCurrentIndex(1);

    colorMain->setColor(QColor(99,99,99));
    colorSubdivision->setColor(QColor(199,199,199));

    intHSpacing->setValue( 10 );
    intVSpacing->setValue( 10 );
    intSubdivision->setValue( 1 );
    intOffsetX->setValue( 0 );
    intOffsetY->setValue( 0 );
}

void GridSettingsTab::spinBoxHSpacingChanged(int v)
{
    if(m_linkSpacing)
    {
        intVSpacing->setValue(v);
    }
}

void GridSettingsTab::spinBoxVSpacingChanged(int v )
{
    if(m_linkSpacing)
    {
        intHSpacing->setValue(v);
    }
}


void GridSettingsTab::linkSpacingToggled(bool b)
{
    m_linkSpacing = b;

    KoImageResource kir;
    if (b) {
        bnLinkSpacing->setIcon(QIcon(kir.chain()));
    }
    else {
        bnLinkSpacing->setIcon(QIcon(kir.chainBroken()));
    }
}


//---------------------------------------------------------------------------------------------------

PreferencesDialog::PreferencesDialog( QWidget* parent, const char* name )
    : KDialog( IconList, i18n("Preferences"), Ok | Cancel | Help | Default /*| Apply*/, Ok, parent, name, true, true )
{
    setCaption( i18n("Preferences") );
    setButtons( Ok | Cancel | Help | Default );
    setDefaultButton( Ok );
    enableButtonSeparator( true );
    setFaceType( KPageDialog::List );
    KVBox *vbox = new KVBox();
    KPageWidgetItem *page = new KPageWidgetItem( vbox, i18n( "General"));
    page->setHeader( i18n( "General") );
    page->setIcon(  BarIcon( "misc", K3Icon::SizeMedium ) );
    addPage( page );
    m_general = new GeneralTab( vbox );
#ifdef HAVE_OPENGL
    vbox = new KVBox();
    page = new KPageWidgetItem( vbox, i18n( "Display" ));
    page->setHeader( i18n( "Display" ) );
    page->setIcon(  BarIcon( "kscreensaver", K3Icon::SizeMedium ) );
    addPage( page );

    m_displaySettings = new DisplaySettingsTab( vbox );
#endif
    vbox = new KVBox();
    page = new KPageWidgetItem( vbox, i18n( "Color Management"));
    page->setHeader( i18n( "Color") );
    page->setIcon(  BarIcon( "colorize", K3Icon::SizeMedium ));
    addPage( page );
    m_colorSettings = new ColorSettingsTab( vbox );

    vbox = new KVBox();
    page = new KPageWidgetItem( vbox, i18n( "Performance"));
    page->setHeader( i18n( "Performance") );
    page->setIcon(  BarIcon( "fork", K3Icon::SizeMedium ));
    addPage( page );

    m_performanceSettings = new PerformanceTab ( vbox );
    vbox = new KVBox();
    page = new KPageWidgetItem( vbox, i18n( "Tablet" ));
    page->setHeader( i18n( "Tablet" ));
    page->setIcon(  BarIcon( "tablet", K3Icon::SizeMedium ));
    addPage( page );

    m_tabletSettings = new TabletSettingsTab( vbox );

    vbox = new KVBox();
    page = new KPageWidgetItem( vbox, i18n( "Grid" ));
    page->setHeader( i18n( "Grid" ));
    page->setIcon(  BarIcon( "grid", K3Icon::SizeMedium ));
    addPage( page );

    m_gridSettings = new GridSettingsTab( vbox );

}

PreferencesDialog::~PreferencesDialog()
{
}

void PreferencesDialog::slotDefault()
{
    m_general->setDefault();
    m_colorSettings->setDefault();
    m_tabletSettings->setDefault();
    m_performanceSettings->setDefault();
#ifdef HAVE_OPENGL
    m_displaySettings->setDefault();
#endif
    m_gridSettings->setDefault();
}

bool PreferencesDialog::editPreferences()
{
    PreferencesDialog* dialog;

    dialog = new PreferencesDialog();
        bool baccept = ( dialog->exec() == Accepted );
    if( baccept )
    {
        KisConfig cfg;
        cfg.setCursorStyle(dialog->m_general->cursorStyle());
        cfg.setDockability( dialog->m_general->dockability() );
        cfg.setDockerFontSize( dialog->m_general->dockerFontSize() );

        // Color settings
        cfg.setMonitorProfile( dialog->m_colorSettings->m_page->cmbMonitorProfile->currentText());
        cfg.setWorkingColorSpace( dialog->m_colorSettings->m_page->cmbWorkingColorSpace->currentText());
        cfg.setPrinterColorSpace( dialog->m_colorSettings->m_page->cmbPrintingColorSpace->currentText());
        cfg.setPrinterProfile( dialog->m_colorSettings->m_page->cmbPrintProfile->currentText());

        cfg.setUseBlackPointCompensation( dialog->m_colorSettings->m_page->chkBlackpoint->isChecked());
        cfg.setPasteBehaviour( dialog->m_colorSettings->m_pasteBehaviourGroup.checkedId());
        cfg.setRenderIntent( dialog->m_colorSettings->m_page->cmbMonitorIntent->currentIndex());

        // it's scaled from 0 - 6, but the config is in 0 - 300
        cfg.setSwappiness(dialog->m_performanceSettings->m_swappiness->value() * 50);
        cfg.setMaxTilesInMem(dialog->m_performanceSettings->m_maxTiles->value());
        // let the tile manager know
        KisTileManager::instance()->configChanged();

        dialog->m_tabletSettings->applySettings();

#ifdef HAVE_OPENGL
        cfg.setUseOpenGL(dialog->m_displaySettings->cbUseOpenGL->isChecked());
        //cfg.setUseOpenGLShaders(dialog->m_displaySettings->cbUseOpenGLShaders->isChecked());
#endif

        // Grid settings
        cfg.setGridMainStyle( dialog->m_gridSettings->selectMainStyle->currentIndex() );
        cfg.setGridSubdivisionStyle( dialog->m_gridSettings->selectSubdivisionStyle->currentIndex() );

        cfg.setGridMainColor( dialog->m_gridSettings->colorMain->color() );
        cfg.setGridSubdivisionColor(dialog->m_gridSettings->colorSubdivision->color() );

        cfg.setGridHSpacing( dialog->m_gridSettings->intHSpacing->value( ));
        cfg.setGridVSpacing( dialog->m_gridSettings->intVSpacing->value( ));
        cfg.setGridSubdivisions( dialog->m_gridSettings->intSubdivision->value( ));
        cfg.setGridOffsetX( dialog->m_gridSettings->intOffsetX->value( ));
        cfg.setGridOffsetY( dialog->m_gridSettings->intOffsetY->value( ));

    }
    delete dialog;
    return baccept;
}

#include "kis_dlg_preferences.moc"
