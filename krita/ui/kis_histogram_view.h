/*
 *  Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#ifndef _KIS_HISTOGRAM_VIEW_
#define _KIS_HISTOGRAM_VIEW_

#include <qlabel.h>
#include <qpixmap.h>
#include <qvaluevector.h>
#include <qstringlist.h>

#include "kis_types.h"
#include "kis_histogram_producer.h"
#include "kis_histogram.h"

class KisChannelInfo;

/**
 * This class displays a histogram. It has a list of channels it can select. The easy
 * way is to display channelStrings() to the user, and then use a setActiveChannel
 * with the integer the same as the one the selected string in that stringlist has.
 * If the selected one is a producer, the histogram will automatically display all its
 * channels, and color them if that is possible.
 *
 * You can also set the channels manually, just don't forget that the displayed channels
 * all need to belong to the same producer! If you set them manually, don't forget to set
 * the (non)usage of color as well.
 *
 * You can either set this to use a specific layer, or use a specific histogram. With the latter,
 * some functionality will disappear, like listProducers(). Setting a histogram will discard
 * info on the layer, and setting a layer will discard info on the histogram.
 **/
class KisHistogramView : public QLabel {
    Q_OBJECT
public:
    KisHistogramView(QWidget *parent = 0, const char *name = 0, WFlags f = 0);
    virtual ~KisHistogramView();

    void setPaintDevice(KisPaintDeviceSP dev);
    void setHistogram(KisHistogramSP histogram);
    void setView(double from, double size);
    KisHistogramProducerSP currentProducer();
    QStringList channelStrings();
    /** Lists all producers currently available */
    KisIDList listProducers();
    /** Sets the currently displayed channels to channels of the producer with producerID as ID*/
    void setCurrentChannels(const KisID& producerID, QValueVector<KisChannelInfo *> channels);
    /** Be careful, producer will be modified */
    void setCurrentChannels(KisHistogramProducerSP producer, QValueVector<KisChannelInfo *> channels);
    bool hasColor();
    void setColor(bool set);

public slots:
    void setActiveChannel(int channel);
    void setHistogramType(enumHistogramType type);
    void updateHistogram();

signals:
    void rightClicked(const QPoint& pos);

protected:
    virtual void mousePressEvent(QMouseEvent * e);

private:
    void setChannels();
    void addProducerChannels(KisHistogramProducerSP producer);

    typedef struct {
        bool isProducer;
        KisHistogramProducerSP producer;
        KisChannelInfo * channel;
    } ComboboxInfo;

    QValueVector<ComboboxInfo> m_comboInfo;
    QPixmap m_pix;
    KisHistogramSP m_histogram;
    KisColorSpace* m_cs;
    KisHistogramProducerSP m_currentProducer;
    QValueVector<KisChannelInfo *> m_channels;
    // Maps the channels in m_channels to a real channel offset in the producer->channels()
    QValueVector<Q_INT32> m_channelToOffset;
    QStringList m_channelStrings;
    bool m_color;
    double m_from;
    double m_width;
};

#endif // _KIS_HISTOGRAM_VIEW_
