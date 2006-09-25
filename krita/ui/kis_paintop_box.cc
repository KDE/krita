/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <QWidget>
#include <QString>
#include <QPixmap>
#include <QLayout>
#include <QHBoxLayout>

#include <klocale.h>
#include <kactioncollection.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kacceleratormanager.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include <kis_paintop_registry.h>
#include <kis_view.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_layer.h>
#include <kis_factory.h>

#include "kis_paintop_box.h"

KisPaintopBox::KisPaintopBox (KisView * view, QWidget *parent, const char * name)
    : super (parent),
      m_canvasController(view->canvasController())
{
    setObjectName(name);

    KAcceleratorManager::setNoAccel(this);

    Q_ASSERT(m_canvasController != 0);

    setWindowTitle(i18n("Painter's Toolchest"));
    m_optionWidget = 0;

    m_cmbPaintops = new QComboBox(this);
    m_cmbPaintops->setObjectName("KisPaintopBox::m_cmbPaintops");
    m_cmbPaintops->setMinimumWidth(150);
    m_cmbPaintops->setToolTip("Styles of painting for the painting tools");
    m_layout = new QHBoxLayout(this);
    m_layout->setMargin(1);
    m_layout->setSpacing(1);
    m_layout->addWidget(m_cmbPaintops);

    connect(this, SIGNAL(selected(const KoID &, const KisPaintOpSettings *)), view, SLOT(paintopActivated(const KoID &, const KisPaintOpSettings *)));
    connect(m_cmbPaintops, SIGNAL(activated(int)), this, SLOT(slotItemSelected(int)));

    // XXX: Let's see... Are all paintops loaded and ready?
    QList<KoID> keys = KisPaintOpRegistry::instance()->listKeys();
    for ( QList<KoID>::Iterator it = keys.begin(); it != keys.end(); ++it ) {
        // add all paintops, and show/hide them afterwards
        addItem(*it);
    }

    connect(view, SIGNAL(currentColorSpaceChanged(KoColorSpace*)),
            this, SLOT(colorSpaceChanged(KoColorSpace*)));
    connect(view, SIGNAL(sigInputDeviceChanged(const KisInputDevice&)),
            this, SLOT(slotInputDeviceChanged(const KisInputDevice&)));

    setCurrentPaintop(defaultPaintop(view->currentInputDevice()));
}

KisPaintopBox::~KisPaintopBox()
{
}

void KisPaintopBox::addItem(const KoID & paintop, const QString & /*category*/)
{
    m_paintops.append(paintop);
}

void KisPaintopBox::slotItemSelected(int index)
{
    if (index < m_displayedOps.count()) {
        KoID paintop = m_displayedOps.at(index);

        setCurrentPaintop(paintop);
    }
}

void KisPaintopBox::colorSpaceChanged(KoColorSpace *cs)
{
    m_displayedOps.clear();
    m_cmbPaintops->clear();

    foreach (KoID paintopId, m_paintops) {
        if (KisPaintOpRegistry::instance()->userVisible(paintopId, cs)) {

            QPixmap pm = paintopPixmap(paintopId);

            if (pm.isNull()) {
                pm = QPixmap(16, 16);
                pm.fill();
            }

            m_cmbPaintops->addItem(QIcon(pm), paintopId.name());
            m_displayedOps.append(paintopId);
        }
    }

    int index = m_displayedOps.indexOf(currentPaintop());

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        index = 0;
    }

    m_cmbPaintops->setCurrentIndex(index);
    slotItemSelected(index);
}

QPixmap KisPaintopBox::paintopPixmap(const KoID & paintop)
{
    QString pixmapName = KisPaintOpRegistry::instance()->pixmap(paintop);

    if (pixmapName.isEmpty()) {
        return QPixmap();
    }

    QString fname = KisFactory::instance()->dirs()->findResource("kis_images", pixmapName);

    return QPixmap(fname);
}

void KisPaintopBox::slotInputDeviceChanged(const KisInputDevice & inputDevice)
{
    KoID paintop;
    InputDevicePaintopMap::iterator it = m_currentID.find(inputDevice);

    if (it == m_currentID.end()) {
        paintop = defaultPaintop(inputDevice);
    } else {
        paintop = (*it).second;
    }

    int index = m_displayedOps.indexOf(paintop);

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colorspace.
        index = 0;
        paintop = m_displayedOps.at(index);
    }

    m_cmbPaintops->setCurrentIndex(index);
    setCurrentPaintop(paintop);
}

void KisPaintopBox::updateOptionWidget()
{
    if (m_optionWidget != 0) {
        m_layout->removeWidget(m_optionWidget);
        m_optionWidget->hide();
        m_layout->invalidate();
    }

    const KisPaintOpSettings *settings = paintopSettings(currentPaintop(), m_canvasController->currentInputDevice());

    if (settings != 0) {
        m_optionWidget = settings->widget();
        Q_ASSERT(m_optionWidget != 0);

        m_layout->addWidget(m_optionWidget);
        updateGeometry();
        m_optionWidget->show();
    }
}

const KoID& KisPaintopBox::currentPaintop()
{
    return m_currentID[m_canvasController->currentInputDevice()];
}

void KisPaintopBox::setCurrentPaintop(const KoID & paintop)
{
    m_currentID[m_canvasController->currentInputDevice()] = paintop;

    updateOptionWidget();

    emit selected(paintop, paintopSettings(paintop, m_canvasController->currentInputDevice()));
}

KoID KisPaintopBox::defaultPaintop(const KisInputDevice& inputDevice)
{
    if (inputDevice == KisInputDevice::eraser()) {
        return KoID("eraser","");
    } else {
        return KoID("paintbrush","");
    }
}

const KisPaintOpSettings *KisPaintopBox::paintopSettings(const KoID & paintop, const KisInputDevice & inputDevice)
{
    QList<KisPaintOpSettings *> settingsArray;
    InputDevicePaintopSettingsMap::iterator it = m_inputDevicePaintopSettings.find(inputDevice);

    if (it == m_inputDevicePaintopSettings.end()) {
        // Create settings for each paintop.

        foreach (KoID paintopId, m_paintops) {
            KisPaintOpSettings *settings = KisPaintOpRegistry::instance()->settings(paintopId, this, inputDevice);
            settingsArray.append(settings);
            if (settings && settings->widget()) {
                settings->widget()->hide();
            }
        }
        m_inputDevicePaintopSettings[inputDevice] = settingsArray;
    } else {
        settingsArray = (*it).second;
    }

    const int index = m_paintops.indexOf(paintop);
    if (index >= 0 && index < settingsArray.count())
        return settingsArray[index];
    else
        return 0;
}

#include "kis_paintop_box.moc"

