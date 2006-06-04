/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
  * Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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

#include <math.h>

#include <klocale.h>

#include <QLayout>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <qdom.h>
#include <QString>
#include <QStringList>
#include <QPushButton>
#include <QHBoxLayout>

#include "kis_filter_config_widget.h"
#include "kis_brightness_contrast_filter.h"
#include "KoColorSpace.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "kis_iterator.h"
#include "kcurve.h"
#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"
#include "kis_painter.h"

KisBrightnessContrastFilterConfiguration::KisBrightnessContrastFilterConfiguration()
    : KisFilterConfiguration( "brightnesscontrast", 1 )
{
    for (quint32 i = 0; i < 256; ++i) {
        transfer[i] = i * 257;
    }
    m_adjustment = 0;
}

KisBrightnessContrastFilterConfiguration::~KisBrightnessContrastFilterConfiguration()
{
    delete m_adjustment;
}

void KisBrightnessContrastFilterConfiguration::fromXML( const QString& s )
{
    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    while (!n.isNull()) {
        e = n.toElement();
        if (!e.isNull()) {
            if (e.attribute("name") == "transfer") {
                QStringList data = e.text().split( "," );
                QStringList::Iterator start = data.begin();
                QStringList::Iterator end = data.end();
                int i = 0;
                for ( QStringList::Iterator it = start; it != end && i < 256; ++it ) {
                    QString s = *it;
                    transfer[i] = s.toUShort();
                    i++;
                }
            }
            else if (e.attribute("name") == "curve") {
                QStringList data = e.text().split( ";" );
                foreach (QString pair, data) {
                    if (pair.indexOf(",") > -1) {
                        QPair<double,double> p;
                        p.first = pair.section(",", 0, 0).toDouble();
                        p.second = pair.section(",", 1, 1).toDouble();
                        curve.append(p);
                    }
                }
            }
        }
        n = n.nextSibling();
    }
}

QString KisBrightnessContrastFilterConfiguration::toString()
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );
    root.setAttribute( "name", name() );
    root.setAttribute( "version", version() );

    doc.appendChild( root );

    QDomElement e = doc.createElement( "transfer" );
    QString sTransfer;
    for ( uint i = 0; i < 256 ; ++i ) {
        sTransfer += QString::number( transfer[i] );
        sTransfer += ',';
    }
    QDomText text = doc.createCDATASection(sTransfer);
    e.appendChild(text);
    root.appendChild(e);

    e = doc.createElement("curve");
    QString sCurve;
    QPair<double,double> pair;
    foreach (pair, curve) {
        sCurve += QString::number(pair.first);
        sCurve += ',';
        sCurve += QString::number(pair.second);
        sCurve += ';';
    }
    text = doc.createCDATASection(sCurve);
    e.appendChild(text);
    root.appendChild(e);

    return doc.toString();
}

KisBrightnessContrastFilter::KisBrightnessContrastFilter()
    : KisFilter( id(), "adjust", i18n("&Brightness/Contrast..."))
{

}

KisFilterConfigWidget * KisBrightnessContrastFilter::createConfigurationWidget(QWidget *parent, KisPaintDeviceSP dev)
{
    return new KisBrightnessContrastConfigWidget(parent, dev);
}

KisFilterConfiguration* KisBrightnessContrastFilter::configuration(QWidget *nwidget)
{
    KisBrightnessContrastConfigWidget* widget = (KisBrightnessContrastConfigWidget*)nwidget;

    if ( widget == 0 )
    {
        return new KisBrightnessContrastFilterConfiguration();
    } else {
        return widget->config();
    }
}

std::list<KisFilterConfiguration*> KisBrightnessContrastFilter::listOfExamplesConfiguration(KisPaintDeviceSP /*dev*/)
{
    //XXX should really come up with a list of configurations
    std::list<KisFilterConfiguration*> list;
    list.insert(list.begin(), new KisBrightnessContrastFilterConfiguration( ));
    return list;
}

bool KisBrightnessContrastFilter::workWith(KoColorSpace* cs)
{
    return (cs->getProfile() != 0);
}


void KisBrightnessContrastFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{

    if (!config) {
        kWarning() << "No configuration object for brightness/contrast filter\n";
        return;
    }

    KisBrightnessContrastFilterConfiguration* configBC = (KisBrightnessContrastFilterConfiguration*) config;

    if (src!=dst) {
        KisPainter gc(dst);
        gc.bitBlt(rect.x(), rect.y(), COMPOSITE_COPY, src, rect.x(), rect.y(), rect.width(), rect.height());
        gc.end();
    }

    if (configBC->m_adjustment == 0) {
        configBC->m_adjustment = src->colorSpace()->createBrightnessContrastAdjustment(configBC->transfer);
    }

    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    qint32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        quint32 npix=0, maxpix = iter.nConseqPixels();
        quint8 selectedness = iter.selectedness();
        // The idea here is to handle stretches of completely selected and completely unselected pixels.
        // Partially selected pixels are handled one pixel at a time.
        switch(selectedness)
        {
            case MIN_SELECTED:
                while(iter.selectedness()==MIN_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                pixelsProcessed += npix;
                break;

            case MAX_SELECTED:
            {
                quint8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, configBC->m_adjustment, npix);
                pixelsProcessed += npix;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), configBC->m_adjustment, 1);
                const quint8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                quint8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }

    setProgressDone();
}

KisBrightnessContrastConfigWidget::KisBrightnessContrastConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name, Qt::WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    int i;
    int height;
    m_page = new WdgBrightnessContrast(this);
    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);

    //Hide these buttons and labels as they are not implemented in 1.5
    m_page->pb_more_contrast->hide();
    m_page->pb_less_contrast->hide();
    m_page->pb_more_brightness->hide();
    m_page->pb_less_brightness->hide();
    m_page->textLabelBrightness->hide();
    m_page->textLabelContrast->hide();

    l->addWidget(m_page, 0, Qt::AlignTop);
    height = 256;
    connect( m_page->kCurve, SIGNAL(modified()), SIGNAL(sigPleaseUpdatePreview()));

    // Create the horizontal gradient label
    QPixmap hgradientpix(256, 1);
    QPainter hgp(&hgradientpix);
    hgp.setPen(QPen::QPen(QColor(0,0,0),1, Qt::SolidLine));
    for( i=0; i<256; ++i )
    {
        hgp.setPen(QColor(i,i,i));
        hgp.drawPoint(i, 0);
    }
    m_page->hgradient->setPixmap(hgradientpix);

    // Create the vertical gradient label
    QPixmap vgradientpix(1, 256);
    QPainter vgp(&vgradientpix);
    vgp.setPen(QPen::QPen(QColor(0,0,0),1, Qt::SolidLine));
    for( i=0; i<256; ++i )
    {
        vgp.setPen(QColor(i,i,i));
        vgp.drawPoint(0, 255-i);
    }
    m_page->vgradient->setPixmap(vgradientpix);

    KisHistogramProducerSP producer = KisHistogramProducerSP(new KisGenericLabHistogramProducer());
    KisHistogram histogram(dev, producer, LINEAR);
    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen::QPen(Qt::gray,1, Qt::SolidLine));

    double highest = (double)histogram.calculations().getHighest();
    qint32 bins = histogram.producer()->numberOfBins();

    if (histogram.getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for( i=0; i<bins; ++i ) {
            p.drawLine(i, height, i, height - int(histogram.getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for( i = 0; i < bins; ++i ) {
            p.drawLine(i, height, i, height - int(log((double)histogram.getValue(i)) * factor));
        }
    }

    m_page->kCurve->setPixmap(pix);

}

KisBrightnessContrastFilterConfiguration * KisBrightnessContrastConfigWidget::config()
{
    KisBrightnessContrastFilterConfiguration * cfg = new KisBrightnessContrastFilterConfiguration();

    for(int i=0; i <256; i++)
    {
        qint32 val;
        val = int(0xFFFF * m_page->kCurve->getCurveValue( i / 255.0));
        if(val >0xFFFF)
            val=0xFFFF;
        if(val <0)
            val = 0;

        cfg->transfer[i] = val;
    }
    cfg->curve = m_page->kCurve->getCurve();
    return cfg;
}

void KisBrightnessContrastConfigWidget::setConfiguration( KisFilterConfiguration * config )
{
    KisBrightnessContrastFilterConfiguration * cfg = dynamic_cast<KisBrightnessContrastFilterConfiguration *>(config);
    m_page->kCurve->setCurve(cfg->curve);
}

#include "kis_brightness_contrast_filter.moc"

