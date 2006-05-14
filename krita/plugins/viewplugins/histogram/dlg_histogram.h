/*
 *  dlg_histogram.h -- part of KimageShop^WKrayon^WKrita
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
#ifndef DLG_HISTOGRAM
#define DLG_HISTOGRAM

#include <kdialog.h>

#include "kis_types.h"

class KisHistogramWidget;

/**
 * This dialog shows the histogram for the (selected) portion
 * of the current layer.
 *
 * XXX: Also for complete image?
 */
class DlgHistogram: public KDialog {
    typedef KDialog super;
    Q_OBJECT

public:

    DlgHistogram(QWidget * parent = 0,
             const char* name = 0);
    ~DlgHistogram();

    void setPaintDevice(KisPaintDeviceSP dev);

private slots:
    void okClicked();

private:

    KisHistogramWidget * m_page;
    KisHistogramSP m_histogram;
    KisLayerSP m_layer;
};

#endif // DLG_HISTOGRAM
