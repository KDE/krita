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
#include <qwidget.h>
#include <qstring.h>
#include <q3valuelist.h>
#include <qpixmap.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>

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
    : super (parent, name),
      m_canvasController(view->getCanvasController())
{
    KAcceleratorManager::setNoAccel(this);

    Q_ASSERT(m_canvasController != 0);

    setCaption(i18n("Painter's Toolchest"));
    m_optionWidget = 0;
    m_paintops = new Q3ValueList<KisID>();
    m_displayedOps = new Q3ValueList<KisID>();

    m_cmbPaintops = new QComboBox(this, "KisPaintopBox::m_cmbPaintops");
    m_cmbPaintops->setMinimumWidth(150);
    m_cmbPaintops->setToolTip("Styles of painting for the painting tools");
    m_layout = new Q3HBoxLayout(this, 1, 1);
    m_layout->addWidget(m_cmbPaintops);

    connect(this, SIGNAL(selected(const KisID &, const KisPaintOpSettings *)), view, SLOT(paintopActivated(const KisID &, const KisPaintOpSettings *)));
    connect(m_cmbPaintops, SIGNAL(activated(int)), this, SLOT(slotItemSelected(int)));

    // XXX: Let's see... Are all paintops loaded and ready?
    KisIDList keys = KisPaintOpRegistry::instance()->listKeys();
    for ( KisIDList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
        // add all paintops, and show/hide them afterwards
        addItem(*it);
    }

    connect(view, SIGNAL(currentColorSpaceChanged(KisColorSpace*)),
            this, SLOT(colorSpaceChanged(KisColorSpace*)));
    connect(view, SIGNAL(sigInputDeviceChanged(const KisInputDevice&)),
            this, SLOT(slotInputDeviceChanged(const KisInputDevice&)));

    setCurrentPaintop(defaultPaintop(m_canvasController->currentInputDevice()));
}

KisPaintopBox::~KisPaintopBox()
{
    delete m_paintops;
    delete m_displayedOps;
}

void KisPaintopBox::addItem(const KisID & paintop, const QString & /*category*/)
{
    m_paintops->append(paintop);
}

void KisPaintopBox::slotItemSelected(int index)
{
    if ((uint)index > m_displayedOps->count()) {
        return;
    }

    KisID paintop = *m_displayedOps->at(index);

    setCurrentPaintop(paintop);
}

void KisPaintopBox::colorSpaceChanged(KisColorSpace *cs)
{
    Q3ValueList<KisID>::iterator it = m_paintops->begin();
    Q3ValueList<KisID>::iterator end = m_paintops->end();
    m_displayedOps->clear();
    m_cmbPaintops->clear();

    for ( ; it != end; ++it ) {
        if (KisPaintOpRegistry::instance()->userVisible(*it, cs)) {
            QPixmap pm = paintopPixmap(*it);
            if (pm.isNull()) {
                QPixmap p = QPixmap( 16, 16 );
                p.fill();
                m_cmbPaintops->insertItem(p,  (*it).name());
            }
            else {
                m_cmbPaintops->insertItem(pm, (*it).name());
            }
            m_displayedOps->append(*it);
        }
    }

    int index = m_displayedOps->findIndex(currentPaintop());

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colourspace.
        index = 0;
    }

    m_cmbPaintops->setCurrentItem( index );
    slotItemSelected( index );
}

QPixmap KisPaintopBox::paintopPixmap(const KisID & paintop)
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
    KisID paintop;
    InputDevicePaintopMap::iterator it = m_currentID.find(inputDevice);

    if (it == m_currentID.end()) {
        paintop = defaultPaintop(inputDevice);
    } else {
        paintop = (*it).second;
    }

    int index = m_displayedOps->findIndex(paintop);

    if (index == -1) {
        // Must change the paintop as the current one is not supported
        // by the new colourspace.
        index = 0;
        paintop = *m_displayedOps->at(index);
    }

    m_cmbPaintops->setCurrentItem(index);
    setCurrentPaintop(paintop);
}

void KisPaintopBox::updateOptionWidget()
{
    if (m_optionWidget != 0) {
        m_layout->remove(m_optionWidget);
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

const KisID& KisPaintopBox::currentPaintop()
{
    return m_currentID[m_canvasController->currentInputDevice()];
}

void KisPaintopBox::setCurrentPaintop(const KisID & paintop)
{
    m_currentID[m_canvasController->currentInputDevice()] = paintop;

    updateOptionWidget();

    emit selected(paintop, paintopSettings(paintop, m_canvasController->currentInputDevice()));
}

KisID KisPaintopBox::defaultPaintop(const KisInputDevice& inputDevice)
{
    if (inputDevice == KisInputDevice::eraser()) {
        return KisID("eraser","");
    } else {
        return KisID("paintbrush","");
    }
}

const KisPaintOpSettings *KisPaintopBox::paintopSettings(const KisID & paintop, const KisInputDevice & inputDevice)
{
    Q3ValueVector<KisPaintOpSettings *> settingsArray;
    InputDevicePaintopSettingsMap::iterator it = m_inputDevicePaintopSettings.find(inputDevice);

    if (it == m_inputDevicePaintopSettings.end()) {
        // Create settings for each paintop.

        for (Q3ValueList<KisID>::const_iterator pit = m_paintops->begin(); pit != m_paintops->end(); ++pit) {
            KisPaintOpSettings *settings = KisPaintOpRegistry::instance()->settings(*pit, this, inputDevice);
            settingsArray.append(settings);
            if (settings && settings->widget()) {
                settings->widget()->hide();
            }
        }
        m_inputDevicePaintopSettings[inputDevice] = settingsArray;
    } else {
        settingsArray = (*it).second;
    }

    const int index = m_paintops->findIndex(paintop);
    if (index >= 0 && index < (int)settingsArray.count())
        return settingsArray[index];
    else
        return 0;
}

#include "kis_paintop_box.moc"

