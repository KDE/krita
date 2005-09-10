/*
 *  Copyright (c) 2004 Boudewijn Rempt
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
#ifndef KIS_HISTOGRAM_WIDGET_
#define KIS_HISTOGRAM_WIDGET_

#include <qwidget.h>
#include <qpixmap.h>
#include <qvaluevector.h>

#include "kis_types.h"
#include "kis_channelinfo.h"
#include "kis_layer.h"
#include "wdghistogram.h"
#include "kis_histogram_producer.h"

class KisAbstractColorSpace;

/**
 * The histogram widget takes a paint device or an image and
 * draws a histogram for the given KisHistogram.
 */
class KisHistogramWidget : public WdgHistogram {

    typedef WdgHistogram super;
    Q_OBJECT


public:
    KisHistogramWidget(QWidget *parent, const char *name);
    virtual ~KisHistogramWidget();

//    void setHistogram(KisHistogramSP histogram);
//    void setChannels(vKisChannelInfoSP channels, Q_INT32 channelCount);
    void setLayer(KisLayerSP layer);
    void updateHistogram();
    void setView(double from, double size);

private slots:
    void setActiveChannel(int channel);
    void slotTypeSwitched(int id);
    void slotZoomIn();
    void slotZoomOut();
    void slide(int val);

private:
    void setChannels();
    typedef struct {
        bool isProducer;
        KisHistogramProducerSP producer;
        KisChannelInfoSP channel;
    } ComboboxInfo;
    QValueVector<ComboboxInfo> m_comboInfo;
    QPixmap m_pix;
    KisHistogramSP m_histogram;
    KisAbstractColorSpace* m_cs;
    KisHistogramProducerSP m_currentProducer;
    vKisChannelInfoSP m_channels;
    // Maps the channels in m_channels to a real channel offset in the producer -> channels()
    QValueVector<Q_INT32> m_channelToOffset;
    bool m_color;
    double m_from;
    double m_width;
};


#endif // KIS_HISTOGRAM_WIDGET_
