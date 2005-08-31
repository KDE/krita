/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#include <qapplication.h>
#include <qclipboard.h>
#include "qobject.h"

#include "kdebug.h"

#include "kis_types.h"
#include "kis_paint_device_impl.h"
#include "kis_config.h"
#include "kis_colorspace_registry.h"

#include "kis_clipboard.h"

KisClipboard *KisClipboard::m_singleton = 0;

KisClipboard::KisClipboard()
{
    Q_ASSERT(KisClipboard::m_singleton == 0);
    KisClipboard::m_singleton = this;

    m_pushedClipboard = false;
    m_clip = 0;

    connect( QApplication::clipboard(), SIGNAL( dataChanged() ),
         this, SLOT( clipboardDataChanged() ) );


}

KisClipboard::~KisClipboard()
{
}

KisClipboard* KisClipboard::instance()
{
    if(KisClipboard::m_singleton == 0)
    {
        KisClipboard::m_singleton = new KisClipboard();
        Q_CHECK_PTR(KisClipboard::m_singleton);
    }
    return KisClipboard::m_singleton;
}

void KisClipboard::setClip(KisPaintDeviceImplSP selection)
{
    m_clip = selection;

    if (selection) {
        KisConfig cfg;
        QImage qimg;

        if (cfg.applyMonitorProfileOnCopy()) {
            // XXX: Is this a performance problem?
            KisConfig cfg;
            QString monitorProfileName = cfg.monitorProfile();
            KisProfileSP monitorProfile = KisColorSpaceRegistry::instance() -> getProfileByName(monitorProfileName);
            qimg = selection -> convertToQImage(monitorProfile);
        }
        else {
            qimg = selection -> convertToQImage(0);
        }
        QClipboard *cb = QApplication::clipboard();

        cb -> setImage(qimg);
        m_pushedClipboard = true;
    }
}

KisPaintDeviceImplSP KisClipboard::clip()
{
    return m_clip;
}

void KisClipboard::clipboardDataChanged()
{
    if (!m_pushedClipboard) {
        QClipboard *cb = QApplication::clipboard();
        QImage qimg = cb -> image();

        if (!qimg.isNull()) {
            KisAbstractColorSpace * cs = KisColorSpaceRegistry::instance()->get(KisID("RGBA",""));

            m_clip =
                new KisPaintDeviceImpl(cs,
                           "KisClipboard created clipboard selection");
            Q_CHECK_PTR(m_clip);
            m_clip -> convertFromImage(qimg);
        }
    }

    m_pushedClipboard = false;
}


bool KisClipboard::hasClip() 
{
    if (m_clip != 0) {
        return true;
    }
    return false;
}

#include "kis_clipboard.moc"
