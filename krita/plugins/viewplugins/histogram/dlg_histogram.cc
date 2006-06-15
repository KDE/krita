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

#include <QPushButton>
#include <QCheckBox>
#include <QSlider>
#include <QComboBox>
#include <QPixmap>
#include <QImage>
#include <QLabel>

#include <knuminput.h>
#include <klocale.h>
#include <kdebug.h>

#include "kis_types.h"
#include "kis_histogram.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

#include "dlg_histogram.h"
#include "kis_histogram_widget.h"


DlgHistogram::DlgHistogram( QWidget *  parent, const char * name)
    : super (parent)
{
    setCaption( i18n("Histogram") );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setObjectName(name);
    m_page = new KisHistogramWidget(this, "histogram");
    Q_CHECK_PTR(m_page);

    setCaption(i18n("Histogram"));
    setMainWidget(m_page);
    resize(m_page->sizeHint());
}

DlgHistogram::~DlgHistogram()
{
    delete m_page;
}

void DlgHistogram::setPaintDevice(KisPaintDeviceSP dev)
{
    m_page->setPaintDevice(dev);
}

void DlgHistogram::okClicked()
{
    accept();
}

#include "dlg_histogram.moc"
