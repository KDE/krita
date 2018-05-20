/*
 * This file is part of Krita
 *
 * Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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
#include <QDomDocument>
#include <QHBoxLayout>

#include "KoChannelInfo.h"
#include "KoBasicHistogramProducers.h"
#include "KoColorModelStandardIds.h"
#include "KoColorSpace.h"
#include "KoColorTransformation.h"
#include "KoCompositeColorTransformation.h"
#include "KoCompositeOp.h"
#include "KoID.h"

#include "kis_signals_blocker.h"

#include "kis_bookmarked_configuration_manager.h"
#include "kis_config_widget.h"
#include <filter/kis_filter_configuration.h>
#include <kis_selection.h>
#include <kis_paint_device.h>
#include <kis_processing_information.h>

#include "kis_histogram.h"
#include "kis_painter.h"
#include "widgets/kis_curve_widget.h"

QVector<VirtualChannelInfo> KisMultiChannelFilterConfigurationBase::getVirtualChannels(const KoColorSpace *cs)
{
    const bool supportsLightness =
        cs->colorModelId() != LABAColorModelID &&
        cs->colorModelId() != GrayAColorModelID &&
        cs->colorModelId() != GrayColorModelID &&
        cs->colorModelId() != AlphaColorModelID;

    const bool supportsHue = supportsLightness;
    const bool supportSaturation = supportsLightness;

    QVector<VirtualChannelInfo> vchannels;

    QList<KoChannelInfo *> sortedChannels =
        KoChannelInfo::displayOrderSorted(cs->channels());

    if (supportsLightness) {
        vchannels << VirtualChannelInfo(VirtualChannelInfo::ALL_COLORS, -1, 0, cs);
    }

    Q_FOREACH (KoChannelInfo *channel, sortedChannels) {
        int pixelIndex = KoChannelInfo::displayPositionToChannelIndex(channel->displayPosition(), sortedChannels);
        vchannels << VirtualChannelInfo(VirtualChannelInfo::REAL, pixelIndex, channel, cs);
    }

    if (supportsHue) {
        vchannels << VirtualChannelInfo(VirtualChannelInfo::HUE, -1, 0, cs);
    }

    if (supportSaturation) {
        vchannels << VirtualChannelInfo(VirtualChannelInfo::SATURATION, -1, 0, cs);
    }

    if (supportsLightness) {
        vchannels << VirtualChannelInfo(VirtualChannelInfo::LIGHTNESS, -1, 0, cs);
    }

    return vchannels;
}


KisMultiChannelFilterConfigurationBase::KisMultiChannelFilterConfigurationBase(int channelCount, const QString & name, qint32 version)
        : KisColorTransformationConfiguration(name, version)
{
    initDefaultCurves(m_curves, channelCount);
    updateTransfers();
}

KisMultiChannelFilterConfigurationBase::~KisMultiChannelFilterConfigurationBase()
{}

bool KisMultiChannelFilterConfigurationBase::isCompatible(const KisPaintDeviceSP dev) const
{
    return (int)dev->compositionSourceColorSpace()->channelCount() == m_curves.size();
}

void KisMultiChannelFilterConfigurationBase::setCurves(QList<KisCubicCurve> &curves)
{
    m_curves.clear();
    m_curves = curves;

    updateTransfers();
}

void KisMultiChannelFilterConfigurationBase::initDefaultCurves(QList<KisCubicCurve> &curves, int nCh)
{
    curves.clear();
    for (int i = 0; i < nCh; i++) {
        curves.append(KisCubicCurve());
    }
}

void KisMultiChannelFilterConfigurationBase::updateTransfers()
{
    m_transfers.resize(m_curves.size());
    for (int i = 0; i < m_curves.size(); i++) {
        m_transfers[i] = m_curves[i].uint16Transfer();
    }
}

const QVector<QVector<quint16> >&
KisMultiChannelFilterConfigurationBase::transfers() const
{
    return m_transfers;
}

const QList<KisCubicCurve>&
KisMultiChannelFilterConfigurationBase::curves() const
{
    return m_curves;
}

void KisMultiChannelFilterConfigurationBase::fromLegacyXML(const QDomElement& root)
{
    fromXML(root);
}

void KisMultiChannelFilterConfigurationBase::fromXML(const QDomElement& root)
{
    QList<KisCubicCurve> curves;
    quint16 numTransfers = 0;
    int version;
    version  = root.attribute("version").toInt();

    QDomElement e = root.firstChild().toElement();
    QString attributeName;
    KisCubicCurve curve;
    quint16 index;
    while (!e.isNull()) {
        if ((attributeName = e.attribute("name")) == "nTransfers") {
            numTransfers = e.text().toUShort();
        } else {
            QRegExp rx("curve(\\d+)");
            if (rx.indexIn(attributeName, 0) != -1) {

                index = rx.cap(1).toUShort();
                index = qMin(index, quint16(curves.count()));

                if (!e.text().isEmpty()) {
                    curve.fromString(e.text());
                }
                curves.insert(index, curve);
            }
        }
        e = e.nextSiblingElement();
    }

    //prepend empty curves for the brightness contrast filter.
    if(getString("legacy") == "brightnesscontrast") {
        if (getString("colorModel") == LABAColorModelID.id()) {
            curves.append(KisCubicCurve());
            curves.append(KisCubicCurve());
            curves.append(KisCubicCurve());
        } else {
            int extraChannels = 5;
            if (getString("colorModel") == CMYKAColorModelID.id()) {
                extraChannels = 6;
            } else if (getString("colorModel") == GrayAColorModelID.id()) {
                extraChannels = 0;
            }
            for(int c = 0; c < extraChannels; c ++) {
                curves.insert(0, KisCubicCurve());
            }
        }
    }
    if (!numTransfers)
        return;

    setVersion(version);
    setCurves(curves);
}

/**
 * Inherited from KisPropertiesConfiguration
 */
//void KisMultiChannelFilterConfigurationBase::fromXML(const QString& s)

void addParamNode(QDomDocument& doc,
                  QDomElement& root,
                  const QString &name,
                  const QString &value)
{
    QDomText text = doc.createTextNode(value);
    QDomElement t = doc.createElement("param");
    t.setAttribute("name", name);
    t.appendChild(text);
    root.appendChild(t);
}

void KisMultiChannelFilterConfigurationBase::toXML(QDomDocument& doc, QDomElement& root) const
{
    /**
     * <params version=1>
     *       <param name="nTransfers">3</param>
     *       <param name="curve0">0,0;0.5,0.5;1,1;</param>
     *       <param name="curve1">0,0;1,1;</param>
     *       <param name="curve2">0,0;1,1;</param>
     * </params>
     */

    root.setAttribute("version", version());

    QDomText text;
    QDomElement t;

    addParamNode(doc, root, "nTransfers", QString::number(m_curves.size()));

    KisCubicCurve curve;
    QString paramName;

    for (int i = 0; i < m_curves.size(); ++i) {
        QString name = QLatin1String("curve") + QString::number(i);
        QString value = m_curves[i].toString();

        addParamNode(doc, root, name, value);
    }
}


KisMultiChannelConfigWidgetBase::KisMultiChannelConfigWidgetBase(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f)
        : KisConfigWidget(parent, f)
        , m_dev(dev)
        , m_page(new WdgPerChannel(this))
{
    Q_ASSERT(m_dev);

    const KoColorSpace *targetColorSpace = dev->compositionSourceColorSpace();
    m_virtualChannels = KisMultiChannelFilterConfigurationBase::getVirtualChannels(targetColorSpace);

    /* in subclass:

    // fill in the channel chooser, in the display order, but store the pixel index as well.
    const int virtualChannelCount = m_virtualChannels.size();

    KisMultiChannelFilterConfigurationBase::initDefaultCurves(m_curves,
                                                        virtualChannelCount);
    init(m_page);

    */
}


/**
 * Initialize the dialog.
 * Note: m_curves must be populated before calling this
 */
void KisMultiChannelConfigWidgetBase::init() {
    QHBoxLayout * layout = new QHBoxLayout(this);
    Q_CHECK_PTR(layout);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_page);

    const int virtualChannelCount = m_virtualChannels.size();
    for (int i = 0; i < virtualChannelCount; i++) {
        const VirtualChannelInfo &info = m_virtualChannels[i];

        m_page->cmbChannel->addItem(info.name(), info.pixelIndex());
        m_curves[i].setName(info.name());
    }

    connect(m_page->cmbChannel, SIGNAL(activated(int)), this, SLOT(setActiveChannel(int)));
    connect((QObject*)(m_page->chkLogarithmic), SIGNAL(toggled(bool)), this, SLOT(logHistView()));
    connect((QObject*)(m_page->resetButton), SIGNAL(clicked()), this, SLOT(resetCurve()));

    // create the horizontal and vertical gradient labels
    m_page->hgradient->setPixmap(createGradient(Qt::Horizontal));
    m_page->vgradient->setPixmap(createGradient(Qt::Vertical));

    // init histogram calculator
    const KoColorSpace *targetColorSpace = m_dev->compositionSourceColorSpace();
    QList<QString> keys =
        KoHistogramProducerFactoryRegistry::instance()->keysCompatibleWith(targetColorSpace);

    if(keys.size() > 0) {
        KoHistogramProducerFactory *hpf;
        hpf = KoHistogramProducerFactoryRegistry::instance()->get(keys.at(0));
        m_histogram = new KisHistogram(m_dev, m_dev->exactBounds(), hpf->generate(), LINEAR);
    }

    connect(m_page->curveWidget, SIGNAL(modified()), this, SIGNAL(sigConfigurationItemChanged()));

    // TODO: needed?
    //m_page->curveWidget->setupInOutControls(m_page->intIn, m_page->intOut, 0, 100);

    {
        KisSignalsBlocker b(m_page->curveWidget);
        m_page->curveWidget->setCurve(m_curves[0]);
        setActiveChannel(0);
    }
}

KisMultiChannelConfigWidgetBase::~KisMultiChannelConfigWidgetBase()
{
    delete m_histogram;
}

inline QPixmap KisMultiChannelConfigWidgetBase::createGradient(Qt::Orientation orient /*, int invert (not used yet) */)
{
    int width;
    int height;
    int *i, inc, col;
    int x = 0, y = 0;

    if (orient == Qt::Horizontal) {
        i = &x; inc = 1; col = 0;
        width = 256; height = 1;
    } else {
        i = &y; inc = -1; col = 255;
        width = 1; height = 256;
    }

    QPixmap gradientpix(width, height);
    QPainter p(&gradientpix);
    p.setPen(QPen(QColor(0, 0, 0), 1, Qt::SolidLine));
    for (; *i < 256; (*i)++, col += inc) {
        p.setPen(QColor(col, col, col));
        p.drawPoint(x, y);
    }
    return gradientpix;
}

inline QPixmap KisMultiChannelConfigWidgetBase::getHistogram()
{
    int i;
    int height = 256;
    QPixmap pix(256, height);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_histogram, pix);


    bool logarithmic = m_page->chkLogarithmic->isChecked();

    if (logarithmic)
        m_histogram->setHistogramType(LOGARITHMIC);
    else
        m_histogram->setHistogramType(LINEAR);


    QPalette appPalette = QApplication::palette();

    pix.fill(QColor(appPalette.color(QPalette::Base)));

    QPainter p(&pix);
    p.setPen(QColor(appPalette.color(QPalette::Text)));
    p.save();
    p.setOpacity(0.2);

    const VirtualChannelInfo &info = m_virtualChannels[m_activeVChannel];


    if (info.type() == VirtualChannelInfo::REAL) {
        m_histogram->setChannel(info.pixelIndex());

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
    }

    p.restore();

    return pix;
}

void KisMultiChannelConfigWidgetBase::setActiveChannel(int ch)
{
    m_curves[m_activeVChannel] = m_page->curveWidget->curve();

    m_activeVChannel = ch;
    m_page->curveWidget->setCurve(m_curves[m_activeVChannel]);
    m_page->curveWidget->setPixmap(getHistogram());
    m_page->cmbChannel->setCurrentIndex(m_activeVChannel);

    updateChannelRange();
}

void KisMultiChannelConfigWidgetBase::logHistView()
{
    m_page->curveWidget->setPixmap(getHistogram());
}

void KisMultiChannelConfigWidgetBase::resetCurve()
{
        m_page->curveWidget->reset();
}
