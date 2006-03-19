/*
 * This file is part of Krita
 *
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
#include <qlayout.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qdom.h>

#include "kis_filter_configuration.h"
#include "kis_filter_config_widget.h"
#include "kis_perchannel_filter.h"
#include "wdg_perchannel.h"
#include "kis_colorspace.h"
#include "kis_paint_device.h"
#include "kis_iterators_pixel.h"
#include "kcurve.h"
#include "kis_histogram.h"
#include "kis_basic_histogram_producers.h"
#include "kis_painter.h"

KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(int n)
    : KisFilterConfiguration( "perchannel", 1 )
{
    curves = new QSortedList<QPair<double,double> >[n];
    for(int i=0;i<n;i++) {
        transfers[i] = new Q_UINT16[256];

        for (Q_UINT32 j = 0; j < 256; ++j) {
            transfers[i][j] = j * 257;
        }
    }
    nTransfers = n;
}

KisPerChannelFilterConfiguration::~KisPerChannelFilterConfiguration()
{
    delete [] curves;
    for(int i=0;i<nTransfers;i++)
        delete [] transfers[i];
}

void KisPerChannelFilterConfiguration::fromXML( const QString& s )
{
    QDomDocument doc;
    doc.setContent( s );
    QDomElement e = doc.documentElement();
    QDomNode n = e.firstChild();

    while (!n.isNull()) {
        e = n.toElement();
        if (!e.isNull()) {
            if (e.attribute("name") == "curves") {
                QDomNode curvesNode = e.firstChild();
                int count = 0;
                nTransfers = e.attribute("number").toUShort();
                curves = new QSortedList<QPair<double,double> >[nTransfers];
                while (!curvesNode.isNull()) {
                    QDomElement curvesElement = curvesNode.toElement();
                    if (!curvesElement.isNull() && 
!curvesElement.text().isEmpty()) {
                        QStringList data = QStringList::split( ";", 
curvesElement.text() );
                        QStringList::Iterator pairStart = data.begin();
                        QStringList::Iterator pairEnd = data.end();
                        for (QStringList::Iterator it = pairStart; it != pairEnd; ++it) {
                            QString pair = * it;
                            if (pair.find(",") > -1) {
                                QPair<double,double> *p = new QPair<double,double>;
                                p->first = pair.section(",", 0, 0).toDouble();
                                p->second = pair.section(",", 1, 1).toDouble();
                                curves[count].append(p);
                            }
                        }
                    }
                    count++;
                    curvesNode = curvesNode.nextSibling();
                }
            }
        }
        n = n.nextSibling();
    }

    for(int ch = 0; ch < nTransfers; ++ch)
    {
        transfers[ch] = new Q_UINT16[256];
        for(int i = 0; i < 256; ++i)
        {
            Q_INT32 val;
            val = int(0xFFFF * KCurve::getCurveValue(curves[ch], i / 
255.0));
            if(val > 0xFFFF)
                val = 0xFFFF;
            if(val < 0)
                val = 0;

            transfers[ch][i] = val;
        }
    }
}

QString KisPerChannelFilterConfiguration::toString()
{
    QDomDocument doc = QDomDocument("filterconfig");
    QDomElement root = doc.createElement( "filterconfig" );
    root.setAttribute( "name", name() );
    root.setAttribute( "version", version() );

    QDomElement c = doc.createElement("curves");
    c.setAttribute("number", nTransfers);
    c.setAttribute("name", "curves");
    for (int i = 0; i < nTransfers; ++i) {
        QDomElement t = doc.createElement("curve");
        QPtrList<QPair<double,double> > curve = curves[i];
        QString sCurve;
        QPair<double,double> * pair;
        for ( pair = curve.first(); pair; pair = curve.next() ) {
            sCurve += QString::number(pair->first);
            sCurve += ",";
            sCurve += QString::number(pair->second);
            sCurve += ";";
        }
        QDomText text = doc.createCDATASection(sCurve);
        t.appendChild(text);
        c.appendChild(t);
    }
    root.appendChild(c);


    doc.appendChild( root );
    return doc.toString();
}


KisFilterConfigWidget * KisPerChannelFilter::createConfigurationWidget(QWidget *parent, KisPaintDeviceSP dev)
{
    return new KisPerChannelConfigWidget(parent, dev);
}

KisFilterConfiguration* KisPerChannelFilter::configuration(QWidget *nwidget)
{
    KisPerChannelConfigWidget* widget = (KisPerChannelConfigWidget*)nwidget;

    if ( widget == 0 )
    {
        return 0;
    } else {
        return widget->config();
    }
}

std::list<KisFilterConfiguration*> KisPerChannelFilter::listOfExamplesConfiguration(KisPaintDeviceSP dev)
{
    std::list<KisFilterConfiguration*> list;
    list.insert(list.begin(), new KisPerChannelFilterConfiguration(dev->colorSpace()->nColorChannels()));
    return list;
}


void KisPerChannelFilter::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    if (!config) {
        kdWarning() << "No configuration object for per-channel filter\n";
        return;
    }
    
    KisPerChannelFilterConfiguration* configBC = (KisPerChannelFilterConfiguration*) config;
    if (configBC->nTransfers != src->colorSpace()->nColorChannels()) {
        // We got an illegal number of colorchannels.KisFilter
        return;
    }
    KisColorAdjustment *adj = src->colorSpace()->createPerChannelAdjustment(configBC->transfers);


    if (src!=dst) {
        KisPainter gc(dst);
        gc.bitBlt(rect.x(), rect.y(), COMPOSITE_COPY, src, rect.x(), rect.y(), rect.width(), rect.height());
        gc.end();
    }

    KisRectIteratorPixel iter = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );

    setProgressTotalSteps(rect.width() * rect.height());
    Q_INT32 pixelsProcessed = 0;

    while( ! iter.isDone()  && !cancelRequested())
    {
        Q_UINT32 npix=0, maxpix = iter.nConseqPixels();
        Q_UINT8 selectedness = iter.selectedness();
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
                Q_UINT8 *firstPixel = iter.rawData();
                while(iter.selectedness()==MAX_SELECTED && maxpix)
                {
                    --maxpix;
                    ++iter;
                    ++npix;
                }
                // adjust
                src->colorSpace()->applyAdjustment(firstPixel, firstPixel, adj, npix);
                pixelsProcessed += npix;
                break;
            }

            default:
                // adjust, but since it's partially selected we also only partially adjust
                src->colorSpace()->applyAdjustment(iter.oldRawData(), iter.rawData(), adj, 1);
                const Q_UINT8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
                Q_UINT8 weights[2] = {MAX_SELECTED - selectedness, selectedness};
                src->colorSpace()->mixColors(pixels, weights, 2, iter.rawData());
                ++iter;
                pixelsProcessed++;
                break;
        }
        setProgress(pixelsProcessed);
    }
    delete adj;
    setProgressDone();
}

void KisPerChannelConfigWidget::setActiveChannel(int ch)
{
    int i;
    int height = 256;
    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen::QPen(Qt::gray,1, Qt::SolidLine));

    m_histogram->setChannel(ch);

    double highest = (double)m_histogram->calculations().getHighest();
    Q_INT32 bins = m_histogram->producer()->numberOfBins();

    if (m_histogram->getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for( i=0; i<bins; ++i ) {
            p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for( i = 0; i < bins; ++i ) {
            p.drawLine(i, height, i, height - int(log((double)m_histogram->getValue(i)) * factor));
        }
    }

    m_curves[m_activeCh].setAutoDelete(true);
    m_curves[m_activeCh] = m_page->kCurve->getCurve();
    m_activeCh = ch;
    m_page->kCurve->setCurve(m_curves[m_activeCh]);

    m_page->kCurve->setPixmap(pix);
}

KisPerChannelConfigWidget::KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, const char * name, WFlags f)
    : KisFilterConfigWidget(parent, name, f)
{
    int i;
    int height;
    m_page = new WdgPerChannel(this);
    QHBoxLayout * l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);

    m_dev = dev;
    m_curves = new QSortedList<QPair<double,double> >[m_dev->colorSpace()->nColorChannels()];
    m_activeCh = 0;
    for(unsigned int ch=0; ch <m_dev->colorSpace()->nColorChannels(); ch++)
    {
        m_curves[ch].append(new QPair<double,double>(0, 0));
        m_curves[ch].append(new QPair<double,double>(1, 1));
    }

    l->add(m_page);
    height = 256;
    connect( m_page->kCurve, SIGNAL(modified()), SIGNAL(sigPleaseUpdatePreview()));

    // Fill in the channel chooser
    QValueVector<KisChannelInfo *> channels = dev->colorSpace()->channels();
    for(unsigned int val=0; val < dev->colorSpace()->nColorChannels(); val++)
        m_page->cmbChannel->insertItem(channels.at(val)->name());
    connect( m_page->cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));

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

    KisIDList keys =
        KisHistogramProducerFactoryRegistry::instance()->listKeysCompatibleWith(m_dev->colorSpace());
    KisHistogramProducerFactory *hpf;
    hpf = KisHistogramProducerFactoryRegistry::instance()->get(*(keys.at(0)));
    m_histogram = new KisHistogram(m_dev, hpf->generate(), LINEAR);

    setActiveChannel(0);
}

KisPerChannelFilterConfiguration * KisPerChannelConfigWidget::config()
{
    int nCh = m_dev->colorSpace()->nColorChannels();
    KisPerChannelFilterConfiguration * cfg = new KisPerChannelFilterConfiguration(nCh);

    m_curves[m_activeCh].setAutoDelete(true);
    m_curves[m_activeCh] = m_page->kCurve->getCurve();
    
    for(int ch = 0; ch < nCh; ch++)
    {
        cfg->curves[ch].setAutoDelete(true);
        cfg->curves[ch].clear();
        QPair<double, double> *p, *inpoint;
        inpoint = m_curves[ch].first();
        while(inpoint)
        {
            p = new QPair<double, double>(inpoint->first, inpoint->second);
            cfg->curves[ch].append(p);
            inpoint = m_curves[ch].next();
        }

        for(int i=0; i <256; i++)
        {
            Q_INT32 val;
            val = int(0xFFFF * m_page->kCurve->getCurveValue(m_curves[ch],  i / 255.0));
            if ( val > 0xFFFF )
                val = 0xFFFF;
            if ( val < 0 )
                val = 0;

            cfg->transfers[ch][i] = val;
        }
    }
    return cfg;
}

void KisPerChannelConfigWidget::setConfiguration(KisFilterConfiguration * config)
{
    KisPerChannelFilterConfiguration * cfg = dynamic_cast<KisPerChannelFilterConfiguration *>(config);

    for(unsigned int ch = 0; ch < cfg->nTransfers; ch++)
    {
        m_curves[ch].setAutoDelete(true);
        m_curves[ch].clear();
        QPair<double, double> *p, *inpoint;
        inpoint = cfg->curves[ch].first();
        while(inpoint)
        {
            p = new QPair<double, double>(inpoint->first, inpoint->second);
            m_curves[ch].append(p);
            inpoint = cfg->curves[ch].next();
        }
    }
    m_page->kCurve->setCurve(m_curves[m_activeCh]);
    setActiveChannel( 0 );
}

#include "kis_perchannel_filter.moc"
