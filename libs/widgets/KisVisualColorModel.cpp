/*
 * SPDX-FileCopyrightText: 2016 Wolthera van Hovell tot Westerflier <griffinvalley@gmail.com>
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisVisualColorModel.h"

#include <QRect>
#include <QVector>
#include <QVector3D>
#include <QVector4D>
#include <QList>
#include <QtMath>
#include <QPointer>

#include <KSharedConfig>
#include <KConfigGroup>

#include "KoColorConversions.h"
#include "KoColorDisplayRendererInterface.h"
#include "KoColorProfile.h"
#include "KoChannelInfo.h"
#include "KoColorModelStandardIds.h"
#include "KisColorSelectorConfiguration.h"
#include "kis_signal_compressor.h"
#include "kis_debug.h"

struct KisVisualColorModel::Private
{
    KoColor currentcolor;
    const KoColorSpace *currentCS {0};
    //QList<KisVisualColorSelectorShape*> widgetlist;
    bool acceptTabletEvents {false};
    bool circular {false};
    bool exposureSupported {false};
    bool isRGBA {false};
    bool isLinear {false};
    bool applyGamma {false};
    bool allowUpdates {true};
    int displayPosition[4]; // map channel index to storage index for display
    int colorChannelCount {0};
    qreal gamma {2.2};
    qreal lumaRGB[3] {0.2126, 0.7152, 0.0722};
    QVector4D channelValues;
    QVector4D channelMaxValues;
    ColorModel modelRGB {ColorModel::HSV};
    ColorModel model {ColorModel::None};
    QPointer<const KoColorDisplayRendererInterface> displayRenderer;
    KisColorSelectorConfiguration acs_config;
};

KisVisualColorModel::KisVisualColorModel(QObject *parent)
    : QObject(parent)
    , m_d(new Private)
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_d->acs_config = KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString()));

}

KisVisualColorModel::~KisVisualColorModel()
{
}

void KisVisualColorModel::slotSetColor(const KoColor &c)
{
    if (!m_d->allowUpdates) {
        return;
    }

    if (!m_d->currentCS) {
        m_d->currentcolor = c;
        slotSetColorSpace(c.colorSpace());
    }
    else {
        m_d->currentcolor = c.convertedTo(m_d->currentCS);
        m_d->channelValues = convertKoColorToChannelValues(m_d->currentcolor);
        emitChannelValues();
    }
    if (isHSXModel()) {
        emit sigHSXChanged(QVector3D(m_d->channelValues));
    }
}

void KisVisualColorModel::slotSetColorSpace(const KoColorSpace *cs)
{
    if (!m_d->currentCS || *m_d->currentCS != *cs) {
        const KoColorSpace *csNew = cs;

        // PQ color space is not very suitable for color picking, substitute with linear one
        if (cs->colorModelId() == RGBAColorModelID &&
            cs->profile()->uniqueId() == KoColorSpaceRegistry::instance()->p2020PQProfile()->uniqueId()) {

            csNew = KoColorSpaceRegistry::instance()->
                    colorSpace(RGBAColorModelID.id(), Float32BitsColorDepthID.id(),
                               KoColorSpaceRegistry::instance()->p2020G10Profile());
        }


        // TODO: split off non-config related initializations
        loadColorSpace(csNew);
        m_d->currentcolor = KoColor(csNew);
        slotDisplayConfigurationChanged();
        emit sigColorSpaceChanged();
    }
}

void KisVisualColorModel::slotSetChannelValues(const QVector4D &values)
{
    if (!m_d->allowUpdates) {
        return;
    }

    QVector4D newValues(0, 0, 0, 0);
    for (int i = 0; i < m_d->colorChannelCount; i++) {
        newValues[i] = values[i];
    }
    if (newValues != m_d->channelValues) {
        m_d->allowUpdates = false;
        m_d->channelValues = newValues;
        m_d->currentcolor = convertChannelValuesToKoColor(newValues);
        emit sigChannelValuesChanged(m_d->channelValues);
        emit sigNewColor(m_d->currentcolor);
        m_d->allowUpdates = true;
    }
}

// deprecated, and broken...
void KisVisualColorModel::slotSetHSX(const QVector3D &hsx)
{
    if (isHSXModel()) {
        m_d->channelValues = QVector4D(hsx, 0.f);

        KoColor newColor = convertChannelValuesToKoColor(QVector4D(hsx));
        if (newColor != m_d->currentcolor) {
            m_d->currentcolor = newColor;
            emit sigNewColor(m_d->currentcolor);
        }
    }
}

KoColor KisVisualColorModel::currentColor() const
{
    return m_d->currentcolor;
}

QVector4D KisVisualColorModel::channelValues() const
{
    return m_d->channelValues;
}

int KisVisualColorModel::colorChannelCount() const
{
    return m_d->colorChannelCount;
}

KisVisualColorModel::ColorModel KisVisualColorModel::colorModel() const
{
    return m_d->model;
}

void KisVisualColorModel::setColorModel(KisVisualColorModel::ColorModel model)
{
    if (model == m_d->model) {
        return;
    }
    if (m_d->isRGBA) {
        if (model >= ColorModel::HSV && model <= ColorModel::HSY) {
            m_d->model = model;
            m_d->allowUpdates = false;
            emit sigColorModelChanged();
            m_d->channelValues = convertKoColorToChannelValues(m_d->currentcolor);
            emit sigChannelValuesChanged(m_d->channelValues);
            m_d->allowUpdates = true;
        }
    }
}

const KoColorSpace *KisVisualColorModel::colorSpace() const
{
    return m_d->currentCS;
}

bool KisVisualColorModel::isHSXModel() const
{
    return (m_d->model >= ColorModel::HSV && m_d->model <= ColorModel::HSY);
}

KoColor KisVisualColorModel::convertChannelValuesToKoColor(const QVector4D &values) const
{
    KoColor c(m_d->currentCS);
    QVector4D baseValues(values);
    QVector <float> channelValues(c.colorSpace()->channelCount());
    channelValues.fill(1.0);

    if (m_d->model != ColorModel::Channel && m_d->isRGBA == true) {

        if (m_d->model == ColorModel::HSV) {
            HSVToRGB(values.x()*360, values.y(), values.z(), &baseValues[0], &baseValues[1], &baseValues[2]);
        }
        else if (m_d->model == ColorModel::HSL) {
            HSLToRGB(values.x()*360, values.y(), values.z(), &baseValues[0], &baseValues[1], &baseValues[2]);
        }
        else if (m_d->model == ColorModel::HSI) {
            // why suddenly qreal?
            qreal temp[3];
            HSIToRGB(values.x(), values.y(), values.z(), &temp[0], &temp[1], &temp[2]);
            baseValues.setX(temp[0]);
            baseValues.setY(temp[1]);
            baseValues.setZ(temp[2]);
        }
        else /*if (m_d->model == ColorModel::HSY)*/ {
            qreal temp[3];
            qreal Y = pow(values.z(), m_d->gamma);
            HSYToRGB(values.x(), values.y(), Y, &temp[0], &temp[1], &temp[2],
                    m_d->lumaRGB[0], m_d->lumaRGB[1], m_d->lumaRGB[2]);
            baseValues.setX(temp[0]);
            baseValues.setY(temp[1]);
            baseValues.setZ(temp[2]);
            if (!m_d->isLinear) {
                // Note: not all profiles define a TRC necessary for (de-)linearization,
                // substituting with a linear profiles would be better
                QVector<qreal> temp({baseValues[0], baseValues[1], baseValues[2]});
                if (m_d->exposureSupported) {
                    m_d->currentCS->profile()->delinearizeFloatValue(temp);
                }
                else {
                    m_d->currentCS->profile()->delinearizeFloatValueFast(temp);
                }
                baseValues = QVector4D(temp[0], temp[1], temp[2], 0);
            }
        }
        if (m_d->applyGamma) {
            for (int i=0; i<3; i++) {
                baseValues[i] = pow(baseValues[i], 2.2);
            }
        }
    }

    if (m_d->exposureSupported) {
        baseValues *= m_d->channelMaxValues;
    }

    for (int i=0; i<m_d->colorChannelCount; i++) {
        channelValues[m_d->displayPosition[i]] = baseValues[i];
    }

    c.colorSpace()->fromNormalisedChannelsValue(c.data(), channelValues);

    return c;

}

QVector4D KisVisualColorModel::convertKoColorToChannelValues(KoColor c) const
{
    if (c.colorSpace() != m_d->currentCS) {
        c.convertTo(m_d->currentCS);
    }
    QVector <float> channelValues (c.colorSpace()->channelCount());
    channelValues.fill(1.0);
    m_d->currentCS->normalisedChannelsValue(c.data(), channelValues);
    QVector4D channelValuesDisplay(0, 0, 0, 0), coordinates(0, 0, 0, 0);

    for (int i =0; i<m_d->colorChannelCount; i++) {
        channelValuesDisplay[i] = channelValues[m_d->displayPosition[i]];
    }

    if (m_d->exposureSupported) {
        channelValuesDisplay /= m_d->channelMaxValues;
    }
    if (m_d->model != ColorModel::Channel && m_d->isRGBA == true) {
        if (m_d->isRGBA == true) {
            if (m_d->applyGamma) {
                for (int i=0; i<3; i++) {
                    channelValuesDisplay[i] = pow(channelValuesDisplay[i], 1/2.2);
                }
            }
            if (m_d->model == ColorModel::HSV) {
                QVector3D hsv;
                RGBToHSV(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsv[0], &hsv[1], &hsv[2]);
                hsv[0] /= 360;
                coordinates = QVector4D(hsv, 0.f);
            } else if (m_d->model == ColorModel::HSL) {
                QVector3D hsl;
                RGBToHSL(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsl[0], &hsl[1], &hsl[2]);
                hsl[0] /= 360;
                coordinates = QVector4D(hsl, 0.f);
            } else if (m_d->model == ColorModel::HSI) {
                qreal hsi[3];
                RGBToHSI(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsi[0], &hsi[1], &hsi[2]);
                coordinates = QVector4D(hsi[0], hsi[1], hsi[2], 0.f);
            } else if (m_d->model == ColorModel::HSY) {
                if (!m_d->isLinear) {
                    // Note: not all profiles define a TRC necessary for (de-)linearization,
                    // substituting with a linear profiles would be better
                    QVector<qreal> temp({channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2]});
                    m_d->currentCS->profile()->linearizeFloatValue(temp);
                    channelValuesDisplay = QVector4D(temp[0], temp[1], temp[2], 0);
                }
                qreal hsy[3];
                RGBToHSY(channelValuesDisplay[0], channelValuesDisplay[1], channelValuesDisplay[2], &hsy[0], &hsy[1], &hsy[2],
                         m_d->lumaRGB[0], m_d->lumaRGB[1], m_d->lumaRGB[2]);
                hsy[2] = pow(hsy[2], 1/m_d->gamma);
                coordinates = QVector4D(hsy[0], hsy[1], hsy[2], 0.f);
            }
            // if we couldn't determine a hue, keep last value
            if (coordinates[0] < 0) {
                coordinates[0] = m_d->channelValues[0];
            }
            for (int i=0; i<3; i++) {
                coordinates[i] = qBound(0.f, coordinates[i], 1.f);
            }
        }
    } else {
        for (int i=0; i<4; i++) {
            coordinates[i] = qBound(0.f, channelValuesDisplay[i], 1.f);
        }
    }
    return coordinates;
}

void KisVisualColorModel::slotLoadACSConfig()
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    m_d->acs_config = KisColorSelectorConfiguration::fromString(cfg.readEntry("colorSelectorConfiguration", KisColorSelectorConfiguration().toString()));

    m_d->gamma = cfg.readEntry("gamma", 2.2);

    KisVisualColorModel::ColorModel RGB_model = KisVisualColorModel::HSV;

    switch(m_d->acs_config.mainTypeParameter) {

    case KisColorSelectorConfiguration::SV:
    case KisColorSelectorConfiguration::SV2:
    case KisColorSelectorConfiguration::hsvSH:
    case KisColorSelectorConfiguration::VH:
        RGB_model = KisVisualColorModel::HSV;
        break;

    case KisColorSelectorConfiguration::SL:
    case KisColorSelectorConfiguration::hslSH:
    case KisColorSelectorConfiguration::LH:
        RGB_model = KisVisualColorModel::HSL;
        break;

    case KisColorSelectorConfiguration::SI:
    case KisColorSelectorConfiguration::hsiSH:
    case KisColorSelectorConfiguration::IH:
        RGB_model = KisVisualColorModel::HSI;
        break;

    case KisColorSelectorConfiguration::SY:
    case KisColorSelectorConfiguration::hsySH:
    case KisColorSelectorConfiguration::YH:
        RGB_model = KisVisualColorModel::HSY;
        break;

    default:
        KIS_SAFE_ASSERT_RECOVER_NOOP(false);
    }
    if (m_d->acs_config.mainType == KisColorSelectorConfiguration::Triangle) {
        //Triangle only really works in HSV mode.
        RGB_model = KisVisualColorModel::HSV;
    }

    m_d->modelRGB = RGB_model;

    if (m_d->isRGBA && m_d->model != m_d->modelRGB) {
        m_d->model = m_d->modelRGB;
        emit sigColorModelChanged();
        // recalculate channel values and update widgets
        slotDisplayConfigurationChanged();
    }
}

void KisVisualColorModel::slotDisplayConfigurationChanged()
{
    const KoColorDisplayRendererInterface *dri = displayRenderer();

    if (m_d->currentCS)
    {
        m_d->channelMaxValues = QVector4D(1, 1, 1, 1);
        QList<KoChannelInfo *> channels = m_d->currentCS->channels();
        for (int i=0; i<m_d->colorChannelCount; ++i)
        {
            m_d->channelMaxValues[i] = dri->maxVisibleFloatValue(channels[m_d->displayPosition[i]]);
        }
        // need to re-scale our normalized channel values on exposure changes:
        m_d->channelValues = convertKoColorToChannelValues(m_d->currentcolor);

        emitChannelValues();
        if (isHSXModel()) {
            emit sigHSXChanged(QVector3D(m_d->channelValues));
        }
    }
}

void KisVisualColorModel::loadColorSpace(const KoColorSpace *cs)
{
    QList<KoChannelInfo *> channelList = cs->channels();
    int cCount = 0;
    for (const KoChannelInfo *channel: channelList) {
        if (channel->channelType() != KoChannelInfo::ALPHA) {
            m_d->displayPosition[cCount] = channel->displayPosition();
            ++cCount;
        }
    }
    KIS_ASSERT_X(cCount < 5, "", "unsupported channel count!");

    m_d->colorChannelCount = cCount;

    // TODO: The following is done because the IDs are actually strings. Ideally, in the future, we
    // refactor everything so that the IDs are actually proper enums or something faster.
    if (m_d->displayRenderer
            && (cs->colorDepthId() == Float16BitsColorDepthID
                || cs->colorDepthId() == Float32BitsColorDepthID
                || cs->colorDepthId() == Float64BitsColorDepthID)
            && cs->colorModelId() != LABAColorModelID
            && cs->colorModelId() != CMYKAColorModelID) {
        m_d->exposureSupported = true;
    } else {
        m_d->exposureSupported = false;
    }
    m_d->isRGBA = (cs->colorModelId() == RGBAColorModelID);

    const KoColorProfile *profile = cs->profile();
    m_d->isLinear = (profile && profile->isLinear());

    if (m_d->isRGBA) {
        m_d->applyGamma = (m_d->isLinear && m_d->modelRGB != ColorModel::HSY);
        // Note: only profiles that define colorants will give precise luma coefficients.
        // Maybe using the explicitly set values of the Advanced Color Selector is better?
        QVector <qreal> luma = cs->lumaCoefficients();
        memcpy(m_d->lumaRGB, luma.constData(), 3*sizeof(qreal));
        m_d->model = m_d->modelRGB;
    } else {
        m_d->model = KisVisualColorModel::Channel;
    }

    const KoColorSpace *oldCS = m_d->currentCS;
    m_d->currentCS = cs;

    if (!oldCS || (oldCS->colorModelId() != cs->colorModelId())) {
        emit sigColorModelChanged();
    }
}

void KisVisualColorModel::emitChannelValues()
{
    bool updatesAllowed = m_d->allowUpdates;
    m_d->allowUpdates = false;
    emit sigChannelValuesChanged(m_d->channelValues);
    m_d->allowUpdates = updatesAllowed;
}

const KoColorDisplayRendererInterface* KisVisualColorModel::displayRenderer() const
{
    return m_d->displayRenderer ? m_d->displayRenderer : KoDumbColorDisplayRenderer::instance();
}

void KisVisualColorModel::setDisplayRenderer (const KoColorDisplayRendererInterface *displayRenderer)
{
    if (m_d->displayRenderer) {
        m_d->displayRenderer->disconnect(this);
    }
    if (displayRenderer) {
        connect(displayRenderer, SIGNAL(displayConfigurationChanged()),
                SLOT(slotDisplayConfigurationChanged()), Qt::UniqueConnection);
    }
    m_d->displayRenderer = displayRenderer;
    slotDisplayConfigurationChanged();
}
