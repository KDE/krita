/* This file is part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_statusbar.h"

#include <QLabel>
#include <QFontMetrics>

#include <ksqueezedtextlabel.h>
#include <kstatusbar.h>
#include <klocale.h>

#include <KoColorProfile.h>
#include <KoColorSpace.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_paint_device.h>

#include "kis_label_progress.h"
#include "kis_view2.h"


KisStatusBar::KisStatusBar(KStatusBar * sb, KisView2 * view )
    : m_view( view )
    , m_statusbar( sb )
{
    // XXX: Use the KStatusbar fixed size labels!
    m_statusBarZoomLabel = new KStatusBarLabel(QString::null, 0, sb);
    m_statusBarZoomLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    m_statusBarZoomLabel->setMinimumWidth( 50 );
    //addStatusBarItem(m_statusBarZoomLabel,1);

    m_statusBarSelectionLabel = new KSqueezedTextLabel(sb);
    //addStatusBarItem(m_statusBarSelectionLabel,2);

    m_statusBarProfileLabel = new KSqueezedTextLabel(sb);
    //addStatusBarItem(m_statusBarProfileLabel,3);

    //int height = m_statusBarProfileLabel->height();

    m_progress = new KisLabelProgress(sb);
    m_progress->setMaximumWidth(225);
    m_progress->setMinimumWidth(225);
    m_progress->setMaximumHeight(sb->fontMetrics().height() );
    //addStatusBarItem(m_progress, 2, true);

    m_progress->hide();

}


KisStatusBar::~KisStatusBar()
{
}

#define EPSILON 1e-6

void KisStatusBar::setZoom( int zoom )
{
    if (zoom < 1 - EPSILON) {
        m_statusBarZoomLabel->setText(i18n("Zoom %1%",zoom * 100, 0, 'g', 4));
    } else {
        m_statusBarZoomLabel->setText(i18n("Zoom %1%",zoom * 100, 0, 'f', 0));
    }
}

void KisStatusBar::setPosition( int x, int y )
{
}

void KisStatusBar::setSize( int w, int h )
{
}

void KisStatusBar::setSelection( KisImageSP img )
{
    if (m_statusBarSelectionLabel == 0) {
        return;
    }

    if (img) {
        KisPaintDeviceSP dev = img->activeDevice();
        if (dev) {
            if (dev->hasSelection()) {
                QRect r = dev->selection()->selectedExactRect();
                m_statusBarSelectionLabel->setText( i18n("Selection Active: x = %1 y = %2 width = %3 height = %4",r.x(),r.y(), r.width(), r.height()));
                return;
            }
        }
    }

    m_statusBarSelectionLabel->setText(i18n("No Selection"));

}

void KisStatusBar::setProfile( KisImageSP img )
{
    if (m_statusBarProfileLabel == 0) {
        return;
    }

    if (!img) return;

    if (img->profile() == 0) {
        m_statusBarProfileLabel->setText(i18n("No profile"));
    }
    else {
        m_statusBarProfileLabel->setText(img->colorSpace()->name() + "  " + img->profile()->productName());
    }

}

void KisStatusBar::setHelp( const QString &t )
{
}

void KisStatusBar::updateStatusBarProfileLabel()
{
    setProfile( m_view->image() );
}

#include "kis_statusbar.moc"
