/*
 *  dlg_histogram.cc - part of KimageShop^WKrayon^WKrita
 *
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
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>

#include "kis_types.h"
#include "kis_histogram.h"
#include "kis_layer.h"
#include "kis_paint_device_impl.h"

#include "dlg_histogram.h"
#include "kis_histogram_widget.h"


DlgHistogram::DlgHistogram( QWidget *  parent, const char * name)
    : super (parent, name, true, i18n("Histogram"), Ok | Cancel, Ok)
{
    m_page = new KisHistogramWidget(this, "histogram");
    Q_CHECK_PTR(m_page);

    setCaption(i18n("Histogram"));
    setMainWidget(m_page);
    resize(m_page -> sizeHint());
}

DlgHistogram::~DlgHistogram()
{
    delete m_page;
}

void DlgHistogram::setHistogram(KisHistogramSP histogram) 
{
    m_histogram = histogram;
    m_page -> setHistogram(histogram);
}

void DlgHistogram::setLayer(KisLayerSP layer)
{
    m_layer = layer;

    // XXX: depth() - 1: compensate for the alpha channel which isn't in channels info.
    // We need to rationalize Krita here: the typeWithAlpha, typeWithoutAlpha, ChannelsWithAlpha
    // and WithoutAlpha are very confusing.
    m_page -> setChannels(layer -> colorSpace() -> channels(), layer -> colorSpace() -> nColorChannels());
    KisChannelInfo* channel = layer -> colorSpace() -> channels()[0];
    KisHistogramSP histogram = new KisHistogram(layer, *channel, LINEAR);
    setHistogram(histogram);

    connect(m_page -> grpType, SIGNAL(clicked(int)), SLOT(slotTypeSwitched(int)));
    connect(m_page -> cmbChannel,
        SIGNAL(activated(const QString &)),
        this,
        SLOT(slotChannelSelected(const QString &)));
}

void DlgHistogram::okClicked()
{
    accept();
}

void DlgHistogram::slotChannelSelected(const QString & channelName)
{
    vKisChannelInfoSP channels = m_layer -> colorSpace() -> channels();
    for (int i = 0; i < m_layer -> colorSpace() -> nColorChannels(); i++) {
        KisChannelInfo* channel = channels[i];
        if (channel -> name() == channelName) {
            KisHistogramSP histogram;
            
            if (m_page -> grpType -> selectedId() == LINEAR)
                histogram = new KisHistogram(m_layer, *channel, LINEAR);
            else
                histogram = new KisHistogram(m_layer, *channel, LOGARITHMIC);
            
            setHistogram(histogram);
            return;
        }
    }
}

void DlgHistogram::slotTypeSwitched(int id)
{
    if (id == LINEAR)
        m_histogram -> setHistogramType(LINEAR);
    else if (id == LOGARITHMIC)
        m_histogram -> setHistogramType(LOGARITHMIC);
    m_page -> setHistogram(m_histogram);
}

#include "dlg_histogram.moc"
