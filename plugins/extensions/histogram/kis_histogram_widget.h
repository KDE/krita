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
#ifndef KIS_HISTOGRAM_WIDGET_
#define KIS_HISTOGRAM_WIDGET_

#include "kis_types.h"
#include "ui_wdghistogram.h"
#include "KoHistogramProducer.h"

class WdgHistogram : public QWidget, public Ui::WdgHistogram
{
    Q_OBJECT

public:
    WdgHistogram(QWidget *parent) : QWidget(parent) {
        setupUi(this);
    }
};

/**
 * The histogram widget takes a paint device or an image and
 * draws a histogram for the given KisHistogram.
 */
class KisHistogramWidget : public WdgHistogram
{

    Q_OBJECT

public:
    KisHistogramWidget(QWidget *parent, const char *name);
    virtual ~KisHistogramWidget();

    void setPaintDevice(KisPaintDeviceSP dev, const QRect &bounds);

    /** Sets the currently displayed channels to channels of the producer with producerID as ID*/
    void setCurrentChannels(const KoID& producerID, QList<KoChannelInfo *> channels);

    /** Be careful, producer will be modified */
    void setCurrentChannels(KoHistogramProducer *producer, QList<KoChannelInfo *> channels);

private Q_SLOTS:
    void setActiveChannel(int channel);
    void slotTypeSwitched(void);
    void slotZoomIn();
    void slotZoomOut();
    void slide(int val);

private:
    void setChannels(void);
    void addProducerChannels(KoHistogramProducer *producer);

    typedef struct {
        bool isProducer;
        KoHistogramProducer *producer;
        KoChannelInfo * channel;
    } ComboboxInfo;

    QVector<ComboboxInfo> m_comboInfo;
    // Maps the channels in m_channels to a real channel offset in the producer->channels()
    QVector<qint32> m_channelToOffset;
    QStringList m_channelStrings;
    QList<KoChannelInfo *> m_channels;
    const KoColorSpace* m_cs;

    QStringList channelStrings();
    /** Lists all producers currently available */
    QList<QString> producers();

    void setView(double from, double size);
    void updateEnabled();
    double m_from, m_width;
    KoHistogramProducer* m_currentProducer;
    bool m_color;
};


#endif // KIS_HISTOGRAM_WIDGET_
