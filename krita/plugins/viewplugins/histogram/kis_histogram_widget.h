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

#include "kis_types.h"
#include "ui_wdghistogram.h"

class KoColorSpace;

class WdgHistogram : public QWidget, public Ui::WdgHistogram
{
    Q_OBJECT

    public:
        WdgHistogram(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

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

    void setPaintDevice(KisPaintDeviceSP dev);

private slots:
    void setActiveChannel(int channel);
    void slotTypeSwitched(int id);
    void slotZoomIn();
    void slotZoomOut();
    void slide(int val);

private:
    void setView(double from, double size);
    void updateEnabled();
    double m_from, m_width;
};


#endif // KIS_HISTOGRAM_WIDGET_
