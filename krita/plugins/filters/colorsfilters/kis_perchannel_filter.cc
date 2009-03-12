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

#include "kis_perchannel_filter.h"

#include <Qt>
#include <QLayout>
#include <QPixmap>
#include <QPainter>
#include <QLabel>
#include <QComboBox>
#include <qdom.h>
#include <QHBoxLayout>

#include "KoChannelInfo.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorSpace.h"
#include "KoColorTransformation.h"
#include "KoCompositeOp.h"
#include "KoID.h"
#include <KoProgressUpdater.h>

#include "kis_bookmarked_configuration_manager.h"
#include "kis_config_widget.h"
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "kis_iterators_pixel.h"
#include "widgets/kcurve.h"
#include "kis_histogram.h"
#include "kis_painter.h"

#define bounds(x,a,b) (x<a ? a : (x>b ? b :x))

KisPerChannelConfigWidget::KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WFlags f)
        : KisConfigWidget(parent, f)
{
    Q_ASSERT(dev);
    m_page = new WdgPerChannel(this);

    QHBoxLayout * layout = new QHBoxLayout(this);
    Q_CHECK_PTR(layout);
    layout->addWidget(m_page);

    m_dev = dev;
    m_activeCh = 0;

    KisPerChannelFilterConfiguration::initDefaultCurves(m_curves,
                                                        m_dev->colorSpace()->colorChannelCount());

    /* fill in the channel chooser */
    QList<KoChannelInfo *> channels = dev->colorSpace()->channels();
    for (unsigned int ch = 0; ch < dev->colorSpace()->colorChannelCount(); ch++)
        m_page->cmbChannel->addItem(channels.at(ch)->name());

    connect(m_page->cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));

    // create the horizontal and vertical gradient labels
    m_page->hgradient->setPixmap(createGradient(Qt::Horizontal));
    m_page->vgradient->setPixmap(createGradient(Qt::Vertical));

    // init histogram calculator
    QList<KoID> keys =
        KoHistogramProducerFactoryRegistry::instance()->listKeysCompatibleWith(m_dev->colorSpace());
    KoHistogramProducerFactory *hpf;
    hpf = KoHistogramProducerFactoryRegistry::instance()->get(keys.at(0).id());
    m_histogram = new KisHistogram(m_dev, hpf->generate(), LINEAR);

    connect(m_page->kCurve, SIGNAL(modified()), this, SIGNAL(sigConfigChanged()));
    connect(m_page->cbPreview, SIGNAL(stateChanged(int)), this, SLOT(setPreview(int)));

    m_page->kCurve->setupInOutControls(m_page->intIn, m_page->intOut, 0, 100);

    setActiveChannel(0);
}

void KisPerChannelConfigWidget::setPreview(int state)
{
    if(state) {
        connect(m_page->kCurve, SIGNAL(modified()), this, SIGNAL(sigConfigChanged()));
        emit sigConfigChanged();
    }
    else
        disconnect(m_page->kCurve, SIGNAL(modified()), this, SIGNAL(sigConfigChanged()));
}


inline QPixmap KisPerChannelConfigWidget::createGradient(Qt::Orientation orient /*, int invert (not used yet) */)
{
    int width;
    int height;
    int *i, inc, col;
    int x=0,y=0;

    if(orient == Qt::Horizontal) {
        i=&x; inc=1; col=0;
        width=256; height=1;
    }
    else {
        i=&y; inc=-1; col=255;
        width=1; height=256;
    }

    QPixmap gradientpix(width, height);
    QPainter p(&gradientpix);
    p.setPen(QPen::QPen(QColor(0, 0, 0), 1, Qt::SolidLine));
    for (; *i < 256; (*i)++, col+=inc) {
        p.setPen(QColor(col, col, col));
        p.drawPoint(x,y);
    }
    return gradientpix;
}

inline QPixmap KisPerChannelConfigWidget::getHistogram()
{
    int i;
    int height = 256;
    QPixmap pix(256, height);
    pix.fill();
    QPainter p(&pix);
    p.setPen(QPen::QPen(Qt::gray, 1, Qt::SolidLine));

    m_histogram->setChannel(m_activeCh);

    double highest = (double)m_histogram->calculations().getHighest();
    qint32 bins = m_histogram->producer()->numberOfBins();

    if (m_histogram->getHistogramType() == LINEAR) {
        double factor = (double)height / highest;
        for (i = 0; i < bins; ++i) {
            p.drawLine(i, height, i, height - int(m_histogram->getValue(i) * factor));
        }
    } else {
        double factor = (double)height / (double)log(highest);
        for (i = 0; i < bins; ++i) {
            p.drawLine(i, height, i, height - int(log((double)m_histogram->getValue(i)) * factor));
        }
    }
    return pix;
}

#define BITS_PER_BYTE 8
#define pwr2(p) (1<<p)

void KisPerChannelConfigWidget::setActiveChannel(int ch)
{
    m_curves[m_activeCh] = m_page->kCurve->getCurve();
    m_activeCh = ch;
    m_page->kCurve->setCurve(m_curves[m_activeCh]);
    m_page->kCurve->setPixmap(getHistogram());
    m_page->cmbChannel->setCurrentIndex(ch);

    // Getting range accepted by chahhel
    KoChannelInfo *channel = m_dev->colorSpace()->channels()[m_activeCh];
    int order = BITS_PER_BYTE*channel->size();
    int maxValue = pwr2(order);
    int min;
    int max;

    m_page->kCurve->dropInOutControls();

    switch(channel->channelValueType())
      {
      case KoChannelInfo::UINT8: 
      case KoChannelInfo::UINT16: 
      case KoChannelInfo::UINT32:
        m_shift = 0;
        m_scale = double(maxValue);
        min=0;
        max=maxValue-1;
        break;
      case KoChannelInfo::INT8:
      case KoChannelInfo::INT16:
        m_shift = 0.5;
        m_scale = double(maxValue);
        min=-maxValue/2;
        max=maxValue/2 - 1;
        break;
      case KoChannelInfo::FLOAT16:
      case KoChannelInfo::FLOAT32:
      case KoChannelInfo::FLOAT64:
      default:
        m_shift = 0;
        m_scale = 100.0;
        //Hack Alert: shoud be changed to float
        min=0;
        max=100;
        break;
      }

    m_page->kCurve->setupInOutControls(m_page->intIn, m_page->intOut, min, max);
}


KisPropertiesConfiguration * KisPerChannelConfigWidget::configuration() const
{
    int nCh = m_dev->colorSpace()->colorChannelCount();
    KisPerChannelFilterConfiguration * cfg = new KisPerChannelFilterConfiguration(nCh);

    // updating current state
    m_curves[m_activeCh] = m_page->kCurve->getCurve();

    cfg->setCurves(m_curves);

    /* Cached version */
    cfg->updateTransfersCached(*m_page->kCurve);
    //cfg->updateTransfers();

    return cfg;
}

void KisPerChannelConfigWidget::setConfiguration(const KisPropertiesConfiguration * config)
{
    const KisPerChannelFilterConfiguration * cfg = dynamic_cast<const KisPerChannelFilterConfiguration *>(config);
    if (!cfg)
        return;

    if(!cfg->m_nTransfers) {
        /**
         * HACK ALERT: our configuration factory generates
         * default configuration with nTransfers==0.
         * Catching it here.
         */

        KisPerChannelFilterConfiguration::initDefaultCurves(m_curves,
                                                        m_dev->colorSpace()->colorChannelCount());
    } else if (cfg->m_nTransfers != m_dev->colorSpace()->colorChannelCount()) {
        return;
    } else {
        for (unsigned int ch = 0; ch < cfg->m_nTransfers; ch++)
            m_curves[ch] = cfg->m_curves[ch];
    }

    m_page->kCurve->setCurve(m_curves[m_activeCh]);
    setActiveChannel(0);
}


class KisPerChannelFilterConfigurationFactory : public KisFilterConfigurationFactory
{
public:
    KisPerChannelFilterConfigurationFactory() : KisFilterConfigurationFactory("perchannel", 1) {}
    virtual ~KisPerChannelFilterConfigurationFactory() { }
    virtual KisSerializableConfiguration* createDefault() {
        return new KisPerChannelFilterConfiguration(0);
    }
    virtual KisSerializableConfiguration* create(const QDomElement& e) {
        KisFilterConfiguration* fc = new KisPerChannelFilterConfiguration(0);
        fc->fromXML(e);
        return fc;
    }
};

KisPerChannelFilterConfiguration::KisPerChannelFilterConfiguration(int nCh)
        : KisFilterConfiguration("perchannel", 1)
{
    m_transfers=NULL;
    createTransfers(nCh);
    initDefaultCurves(m_curves, nCh);
    updateTransfers();
    oldCs = 0;
}

KisPerChannelFilterConfiguration::~KisPerChannelFilterConfiguration()
{
    deleteTransfers();
}

bool KisPerChannelFilterConfiguration::isCompatible(const KisPaintDeviceSP dev) const
{
    if (!oldCs) return false;
    return *dev->colorSpace() == *oldCs;
}

void KisPerChannelFilterConfiguration::setCurves(QList<KisCurve> &curves)
{
    m_curves.clear();
    m_curves=curves;
    createTransfers(m_curves.count());
}

void KisPerChannelFilterConfiguration::initDefaultCurves(QList<KisCurve> &curves, int nCh)
{
    curves.clear();
    for (int i = 0; i < nCh; i++) {
        curves.append(KisCurve());
        curves[i].append(QPointF(0,0));
        curves[i].append(QPointF(1.,1.));
    }
}

void KisPerChannelFilterConfiguration::createTransfers(int nTransfers)
{
    deleteTransfers();
    m_transfers = new quint16* [nTransfers];
    memset(m_transfers, 0, sizeof(qint16*) * nTransfers);
    m_nTransfers=nTransfers;
}

void KisPerChannelFilterConfiguration::deleteTransfers()
{
    if (m_transfers) {
        for (int i = 0;i < m_nTransfers;i++)
            delete[] m_transfers[i];
        delete[] m_transfers;
        m_transfers=NULL;
    }
}

void KisPerChannelFilterConfiguration::updateTransfers()
{
    for (int ch = 0; ch < m_nTransfers; ++ch) {
        if (!m_transfers[ch])
            m_transfers[ch] = new quint16[256];

        qint32 val;
        for (int i = 0; i < 256; ++i) {
            /* Direct uncached version */
            val = int(0xFFFF * KCurve::getCurveValue(m_curves[ch], i / 255.0));
            val = bounds(val, 0, 0xFFFF);
            m_transfers[ch][i] = val;
        }

    }
}

void KisPerChannelFilterConfiguration::updateTransfersCached(KCurve &cacheKCurve)
{
    for (int ch = 0; ch < m_nTransfers; ++ch) {
        if (!m_transfers[ch])
            m_transfers[ch] = new quint16[256];

        qint32 val;
        for (int i = 0; i < 256; ++i) {
            /* Cached version of the cycle */
            val = int(0xFFFF * cacheKCurve.getCurveValue(i / 255.0));
                val = bounds(val, 0, 0xFFFF);
                m_transfers[ch][i] = val;
        }
    }
}

void KisPerChannelFilterConfiguration::fromLegacyXML(const QDomElement& root)
{
    fromXML(root);
}

void KisPerChannelFilterConfiguration::fromXML(const QDomElement& root)
{
    QList<KisCurve> curves;
    quint16 numTransfers=0;
    int version;
    version  = root.attribute("version").toInt();

    QDomElement e = root.firstChild().toElement();
    QString attributeName;

    while (!e.isNull()) {
        if ((attributeName = e.attribute("name")) == "nTransfers") {
            numTransfers = e.text().toUShort();
        } 
        else {
            QRegExp rx("curve(\\d+)");
            if (rx.indexIn(attributeName, 0) != -1) {
                KisCurve curve;
                quint16 index = rx.cap(1).toUShort();
                index = qMin(index, quint16(curves.count()));

                if (!e.text().isEmpty()) {
                    QStringList data = e.text().split(';');

                    foreach(const QString & pair, data) {
                        if (pair.indexOf(',') > -1) {
                            QPointF p;
                            p.rx() = pair.section(',', 0, 0).toDouble();
                            p.ry() = pair.section(',', 1, 1).toDouble();
                            curve.append(p);
                        }
                    }
                }
                curves.insert(index, curve);
            }
        }
        e=e.nextSiblingElement();
    }

    if(!numTransfers)
        return;

    setVersion(version);
    setCurves(curves);
    updateTransfers();
}

/**
 * Inherited from KisPropertiesConfiguration
 */
//void KisPerChannelFilterConfiguration::fromXML(const QString& s)

void KisPerChannelFilterConfiguration::toLegacyXML(QDomDocument& doc, QDomElement& root) const
{
    toXML(doc,root);
}

void KisPerChannelFilterConfiguration::toXML(QDomDocument& doc, QDomElement& root) const
{
    /**
     * <params version=1>
     *       <param name="nTransfers">3</param>
     *       <param name="curve0">0,0;0.5,0.5;1,1;</param>
     *       <param name="curve1">0,0;1,1;</param>
     *       <param name="curve2">0,0;1,1;</param>
     *       <!-- for the future 
     *       <param name="commonCurve">0,0;1,1;</param>
     *       -->
     * </params>
     */

    root.setAttribute("version", version());

    QDomElement c = doc.createElement("curves");
    c.setAttribute("number", m_nTransfers);
    c.setAttribute("name", "curves");

    QDomElement t = doc.createElement("param");
    QDomText text = doc.createTextNode(QString::number(m_nTransfers));
    t.setAttribute("name", "nTransfers");
    t.appendChild(text);
    root.appendChild(t);

    QString paramName;

    for (int i = 0; i < m_nTransfers; ++i) {
        paramName = QString::fromAscii("curve") + QString::number(i);
        t = doc.createElement("param");
        t.setAttribute("name", paramName);

        KisCurve curve = m_curves[i];

        QString sCurve;
        foreach(const QPointF & pair, curve) {
            sCurve += QString::number(pair.x());
            sCurve += ',';
            sCurve += QString::number(pair.y());
            sCurve += ';';
        }
        text = doc.createTextNode(sCurve);
        t.appendChild(text);
        root.appendChild(t);
    }
}

/**
 * Inherited from KisPropertiesConfiguration
 */
//void KisPerChannelFilterConfiguration::toXML()


KisPerChannelFilter::KisPerChannelFilter() : KisFilter(id(), categoryAdjust(), i18n("&Color Adjustment curves..."))
{
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
    setColorSpaceIndependence(TO_LAB16);
    setBookmarkManager(new KisBookmarkedConfigurationManager(configEntryGroup(),
                       new KisPerChannelFilterConfigurationFactory()));
}

KisConfigWidget * KisPerChannelFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, const KisImageSP image) const
{
    Q_UNUSED(image);
    return new KisPerChannelConfigWidget(parent, dev);
}

KisFilterConfiguration * KisPerChannelFilter::factoryConfiguration(const KisPaintDeviceSP) const
{
    return new KisPerChannelFilterConfiguration(0);
}

void KisPerChannelFilter::process(KisConstProcessingInformation srcInfo,
                                  KisProcessingInformation dstInfo,
                                  const QSize& size,
                                  const KisFilterConfiguration* config,
                                  KoUpdater* progressUpdater
                                 ) const
{
    const KisPaintDeviceSP src = srcInfo.paintDevice();
    KisPaintDeviceSP dst = dstInfo.paintDevice();
    QPoint dstTopLeft = dstInfo.topLeft();
    QPoint srcTopLeft = srcInfo.topLeft();
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    if (!config) {
        warnKrita << "No configuration object for per-channel filter";
        return;
    }

    KisPerChannelFilterConfiguration* configBC =
        const_cast<KisPerChannelFilterConfiguration*>(dynamic_cast<const KisPerChannelFilterConfiguration*>(config)); // Somehow, this shouldn't happen
    if (not configBC)
        return;
    if (configBC->m_nTransfers != src->colorSpace()->colorChannelCount()) {
        // We got an illegal number of colorchannels.KisFilter
        return;
    }

    KoColorTransformation *adj = src->colorSpace()->createPerChannelAdjustment(configBC->m_transfers);


    if (src != dst) {
        KisPainter gc(dst, dstInfo.selection());
        gc.bitBlt(dstTopLeft.x(), dstTopLeft.y(), COMPOSITE_COPY, src, srcTopLeft.x(), srcTopLeft.y(), size.width(), size.height());
        gc.end();
    }

    KisRectIteratorPixel iter = dst->createRectIterator(dstTopLeft.x(), dstTopLeft.y(), size.width(), size.height(), dstInfo.selection());
    KoMixColorsOp * mixOp = src->colorSpace()->mixColorsOp();
    qint32 totalCost = (size.width() * size.height()) / 100;
    if (totalCost == 0) totalCost = 1;
    qint32 pixelsProcessed = 0;

    while (!iter.isDone()  && (progressUpdater && !progressUpdater->interrupted())) {
        quint32 npix = 0, maxpix = iter.nConseqPixels();
        quint8 selectedness = iter.selectedness();
        // The idea here is to handle stretches of completely selected and completely unselected pixels.
        // Partially selected pixels are handled one pixel at a time.
        switch (selectedness) {
        case MIN_SELECTED:
            while (iter.selectedness() == MIN_SELECTED && maxpix) {
                --maxpix;
                ++iter;
                ++npix;
            }
            pixelsProcessed += npix;
            break;

        case MAX_SELECTED: {
            quint8 *firstPixel = iter.rawData();
            while (iter.selectedness() == MAX_SELECTED && maxpix) {
                --maxpix;
                if (maxpix != 0)
                    ++iter;
                ++npix;
            }
            // adjust
            adj->transform(firstPixel, firstPixel, npix);
            pixelsProcessed += npix;
            ++iter;
            break;
        }

        default:
            // adjust, but since it's partially selected we also only partially adjust
            adj->transform(iter.oldRawData(), iter.rawData(), 1);
            const quint8 *pixels[2] = {iter.oldRawData(), iter.rawData()};
            qint16 weights[2] = {MAX_SELECTED - selectedness, selectedness};
            mixOp->mixColors(pixels, weights, 2, iter.rawData());
            ++iter;
            pixelsProcessed++;
            break;
        }
        if (progressUpdater) progressUpdater->setProgress(pixelsProcessed / totalCost);
    }

}



#include "kis_perchannel_filter.moc"

