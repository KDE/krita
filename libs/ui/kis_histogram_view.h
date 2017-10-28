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

#include <QLabel>
#include <QPixmap>
#include <QVector>
#include <QStringList>
#include <QMouseEvent>
#include <QSharedPointer>

#include "kis_types.h"
#include "kis_histogram.h"

#include <kritaui_export.h>


class KoChannelInfo;

/**
 * This class displays a histogram. It has a list of channels it can
 * select. The easy way is to display channelStrings() to the user,
 * and then use a setActiveChannel with the integer the same as the
 * one the selected string in that stringlist has. If the selected one
 * is a producer, the histogram will automatically display all its
 * channels, and color them if that is possible.
 *
 * You can also set the channels manually, just don't forget that the
 * displayed channels all need to belong to the same producer! If you
 * set them manually, don't forget to set the (non)usage of color as
 * well.
 *
 * You can either set this to use a specific layer, or use a specific
 * histogram. With the latter, some functionality will disappear, like
 * listProducers(). Setting a histogram will discard info on the
 * layer, and setting a layer will discard info on the histogram.
 *
 **/
class KRITAUI_EXPORT KisHistogramView : public QLabel
{
    Q_OBJECT

public:

    KisHistogramView(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags f = 0);

    ~KisHistogramView() override;

    void setPaintDevice(KisPaintDeviceSP dev, KoHistogramProducer *producer, const QRect &bounds);

    void setView(double from, double size);

    KoHistogramProducer *currentProducer();

    bool hasColor();
    void setColor(bool set);
    void setProducer(KoHistogramProducer* producer);
    void setChannels(QList<KoChannelInfo*> & channels);
    void paintEvent(QPaintEvent* event) override;
    virtual void updateHistogramCalculation();
    void setSmoothHistogram(bool smoothHistogram);

public Q_SLOTS:
    virtual void setHistogramType(enumHistogramType type);
    virtual void startUpdateCanvasProjection();

Q_SIGNALS:
    void rightClicked(const QPoint& pos);

protected:

    void mousePressEvent(QMouseEvent * e) override;

private:

    void setChannels(void);

    void addProducerChannels(KoHistogramProducer *producer);
    KisHistogramSP m_histogram;
    KisPaintDeviceSP m_currentDev;
    QRect m_currentBounds;
    KoHistogramProducer *m_currentProducer;
    QList<KoChannelInfo *> m_channels;
    bool m_color;
    bool m_smoothHistogram;
    enumHistogramType m_histogram_type;
};

#endif // _KIS_HISTOGRAM_VIEW_
