/*
 *
 *  SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_normalize.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QTime>
#include <QVector3D>

#include <kpluginfactory.h>
#include <klocalizedstring.h>

#include <kis_debug.h>

#include <kis_processing_information.h>
#include <kis_types.h>
#include <kis_selection.h>
#include <kis_layer.h>
#include <filter/kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>

#include <KoColorSpaceMaths.h>
#include <filter/kis_color_transformation_configuration.h>

K_PLUGIN_FACTORY_WITH_JSON(KritaNormalizeFilterFactory, "kritanormalize.json", registerPlugin<KritaNormalizeFilter>();)

KritaNormalizeFilter::KritaNormalizeFilter(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisFilterNormalize()));
}

KritaNormalizeFilter::~KritaNormalizeFilter()
{
}


KisFilterNormalize::KisFilterNormalize()
    : KisColorTransformationFilter(KoID("normalize", i18n("Normalize")),
                                   FiltersCategoryMapId, i18n("&Normalize"))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setShowConfigurationWidget(false);
}

KoColorTransformation* KisFilterNormalize::createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const
{
    Q_UNUSED(config);
    return new KisNormalizeTransformation(cs);
}

KisNormalizeTransformation::KisNormalizeTransformation(const KoColorSpace* cs) : m_colorSpace(cs), m_psize(cs->pixelSize())
{

}

void KisNormalizeTransformation::transform(const quint8* src, quint8* dst, qint32 nPixels) const
{
    // if the color space is not RGBA o something like that, just
    // pass the values through
    if (m_colorSpace->channelCount() != 4) {
        memcpy(dst, src, nPixels * m_colorSpace->pixelSize());
        return;
    }

    QVector3D normal_vector;
    QVector<float> channelValues(4);
    //if (m_colorSpace->colorDepthId().id()!="F16" && m_colorSpace->colorDepthId().id()!="F32" && m_colorSpace->colorDepthId().id()!="F64") {
    /* I don't know why, but the results of this are unexpected with a floating point space.
     * And manipulating the pixels gives strange results.
     */
    while (nPixels--) {
        m_colorSpace->normalisedChannelsValue(src, channelValues);
        normal_vector.setX(channelValues[2]*2-1.0);
        normal_vector.setY(channelValues[1]*2-1.0);
        normal_vector.setZ(channelValues[0]*2-1.0);
        normal_vector.normalize();

        channelValues[0]=normal_vector.z()*0.5+0.5;
        channelValues[1]=normal_vector.y()*0.5+0.5;
        channelValues[2]=normal_vector.x()*0.5+0.5;
        //channelValues[3]=1.0;

        m_colorSpace->fromNormalisedChannelsValue(dst, channelValues);

        dst[3]=src[3];
        src += m_psize;
        dst += m_psize;
    }
    /*    } else {
    while (nPixels--) {
        m_colorSpace->normalisedChannelsValue(src, channelValues);
        qreal max = qMax(channelValues[2], qMax(channelValues[1], channelValues[0]));
        qreal min = qMin(channelValues[2], qMin(channelValues[1], channelValues[0]));
        qreal range = max-min;
        normal_vector.setX( ((channelValues[2]-min)/range) *2.0-1.0);
        normal_vector.setY( ((channelValues[1]-min)/range) *2.0-1.0);
        normal_vector.setZ( ((channelValues[0]-min)/range) *2.0-1.0);
        normal_vector.normalize();

        channelValues[2]=normal_vector.x()*0.5+0.5;
        channelValues[1]=normal_vector.y()*0.5+0.5;
        channelValues[0]=normal_vector.z()*0.5+0.5;
        //channelValues[3]=1.0;

        m_colorSpace->fromNormalisedChannelsValue(dst, channelValues);
        dst[3]=src[3];
        //hack to trunucate values.
        m_colorSpace->toRgbA16(dst, reinterpret_cast<quint8 *>(m_rgba), 1);
        m_colorSpace->fromRgbA16(reinterpret_cast<quint8 *>(m_rgba), dst, 1);

        src += m_psize;
        dst += m_psize;
    }
    }*/
}

#include "kis_normalize.moc"
