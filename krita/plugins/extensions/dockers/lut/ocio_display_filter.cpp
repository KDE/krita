/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
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
#include "ocio_display_filter.h"
#include <math.h>

OcioDisplayFilter::OcioDisplayFilter(QObject *parent)
    : KisDisplayFilter(parent)
    , srcColorSpaceIndex(-1)
    , displayDevice(0)
    , displayColorSpaceName(0)
    , view(0)
    , swizzle(RGBA)
    , exposure(0.0)
    , gamma(0.0)
{
}

void OcioDisplayFilter::filter(quint8 *src, quint8 */*dst*/, quint32 numPixels)
{
    // processes that data _in_ place
    OCIO::PackedImageDesc img(reinterpret_cast<float*>(src), numPixels, 1, 4);
    m_processor->apply(img);

}

void OcioDisplayFilter::updateProcessor()
{
    if (!displayDevice) {
        displayDevice = config->getDefaultDisplay();
    }
    if (!view) {
        view = config->getDefaultView(displayDevice);
    }
    if (!displayColorSpaceName) {
        displayColorSpaceName = config->getDisplayColorSpaceName(displayDevice, view);
    }

    OCIO::DisplayTransformRcPtr transform = OCIO::DisplayTransform::Create();
    transform->setInputColorSpaceName(OCIO::ROLE_SCENE_LINEAR);

    // XXX: the docs say "transform->setDisplayColorSpaceName( displayColorSpace );", but that
    // doesn't exist anymore... So we set display and view
    transform->setDisplay(displayDevice);
    transform->setView(view);

    // fstop exposure control -- not sure how that translates to our
    float gain = powf(2.0f, exposure);
    const float slope3f[] = { gain, gain, gain };
    OCIO::CDLTransformRcPtr cc = OCIO::CDLTransform::Create();
    cc->setSlope(slope3f);
    transform->setLinearCC(cc);

    // channel swizzle
    int channelHot[4];
    switch (swizzle) {
    case LUMINANCE:
        channelHot[0] = 1;
        channelHot[1] = 1;
        channelHot[2] = 1;
        channelHot[3] = 0;
        break;
    case RGBA:
        channelHot[0] = 1;
        channelHot[1] = 1;
        channelHot[2] = 1;
        channelHot[3] = 1;
        break;
    case R:
        channelHot[0] = 1;
        channelHot[1] = 0;
        channelHot[2] = 0;
        channelHot[3] = 0;
        break;
    case G:
        channelHot[0] = 0;
        channelHot[1] = 1;
        channelHot[2] = 0;
        channelHot[3] = 0;
        break;
    case B:
        channelHot[0] = 0;
        channelHot[1] = 0;
        channelHot[2] = 1;
        channelHot[3] = 0;
        break;
    case A:
        channelHot[0] = 0;
        channelHot[1] = 0;
        channelHot[2] = 0;
        channelHot[3] = 1;
    default:
        ;
    }

    // XXX: make configurable?
    float lumacoef[3];
    config->getDefaultLumaCoefs(lumacoef);

    float m44[16];
    float offset[4];

    OCIO::MatrixTransform::View(m44, offset, channelHot, lumacoef);
    OCIO::MatrixTransformRcPtr swizzle = OCIO::MatrixTransform::Create();
    swizzle->setValue(m44, offset);
    transform->setChannelView(swizzle);

    // XXX: where does our lut file come in? --> probably not at all. let's hide it for now.

    // And then process the image normally.
    m_processor = config->getProcessor(transform);


}
