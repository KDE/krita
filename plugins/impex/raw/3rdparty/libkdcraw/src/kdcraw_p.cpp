/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2008-10-09
 * @brief  internal private container for KDcraw
 *
 * @author Copyright (C) 2008-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "kdcraw.h"
#include "kdcraw_p.h"

// Qt includes

#include <QString>
#include <QFile>

// Local includes

#include "libkdcraw_debug.h"

namespace KDcrawIface
{

int callbackForLibRaw(void* data, enum LibRaw_progress p, int iteration, int expected)
{
    if (data)
    {
        KDcraw::Private* const d = static_cast<KDcraw::Private*>(data);

        if (d)
        {
            return d->progressCallback(p, iteration, expected);
        }
    }

    return 0;
}

// --------------------------------------------------------------------------------------------------

KDcraw::Private::Private(KDcraw* const p)
{
    m_progress = 0.0;
    m_parent   = p;
}

KDcraw::Private::~Private()
{
}

void KDcraw::Private::createPPMHeader(QByteArray& imgData, libraw_processed_image_t* const img)
{
    QString header = QString("P%1\n%2 %3\n%4\n").arg(img->colors == 3 ? "6" : "5")
                                                .arg(img->width)
                                                .arg(img->height)
                                                .arg((1 << img->bits)-1);
    imgData.append(header.toLatin1());
    imgData.append(QByteArray((const char*)img->data, (int)img->data_size));
}

int KDcraw::Private::progressCallback(enum LibRaw_progress p, int iteration, int expected)
{
    qCDebug(LIBKDCRAW_LOG) << "LibRaw progress: " << libraw_strprogress(p) << " pass "
                           << iteration << " of " << expected;

    // post a little change in progress indicator to show raw processor activity.
    setProgress(progressValue()+0.01);

    // Clean processing termination by user...
    if (m_parent->checkToCancelWaitingData())
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw process terminaison invoked...";
        m_parent->m_cancel = true;
        m_progress         = 0.0;
        return 1;
    }

    // Return 0 to continue processing...
    return 0;
}

void KDcraw::Private::setProgress(double value)
{
    m_progress = value;
    m_parent->setWaitingDataProgress(m_progress);
}

double KDcraw::Private::progressValue() const
{
    return m_progress;
}

void KDcraw::Private::fillIndentifyInfo(LibRaw* const raw, DcrawInfoContainer& identify)
{
#if QT_VERSION >= 0x050900
    identify.dateTime.setSecsSinceEpoch(raw->imgdata.other.timestamp);
#else
    identify.dateTime.setTime_t(raw->imgdata.other.timestamp);
#endif
    identify.make             = QString(raw->imgdata.idata.make);
    identify.model            = QString(raw->imgdata.idata.model);
    identify.owner            = QString(raw->imgdata.other.artist);
    identify.DNGVersion       = QString::number(raw->imgdata.idata.dng_version);
    identify.sensitivity      = raw->imgdata.other.iso_speed;
    identify.exposureTime     = raw->imgdata.other.shutter;
    identify.aperture         = raw->imgdata.other.aperture;
    identify.focalLength      = raw->imgdata.other.focal_len;
    identify.imageSize        = QSize(raw->imgdata.sizes.width, raw->imgdata.sizes.height);
    identify.fullSize         = QSize(raw->imgdata.sizes.raw_width, raw->imgdata.sizes.raw_height);
    identify.outputSize       = QSize(raw->imgdata.sizes.iwidth, raw->imgdata.sizes.iheight);
    identify.thumbSize        = QSize(raw->imgdata.thumbnail.twidth, raw->imgdata.thumbnail.theight);
    identify.topMargin        = raw->imgdata.sizes.top_margin;
    identify.leftMargin       = raw->imgdata.sizes.left_margin;
    identify.hasIccProfile    = raw->imgdata.color.profile ? true : false;
    identify.isDecodable      = true;
    identify.pixelAspectRatio = raw->imgdata.sizes.pixel_aspect;
    identify.rawColors        = raw->imgdata.idata.colors;
    identify.rawImages        = raw->imgdata.idata.raw_count;
    identify.blackPoint       = raw->imgdata.color.black;

    for (int ch = 0; ch < 4; ch++)
    {
        identify.blackPointCh[ch] = raw->imgdata.color.cblack[ch];
    }

    identify.whitePoint       = raw->imgdata.color.maximum;
    identify.orientation      = (DcrawInfoContainer::ImageOrientation)raw->imgdata.sizes.flip;

    memcpy(&identify.cameraColorMatrix1, &raw->imgdata.color.cmatrix, sizeof(raw->imgdata.color.cmatrix));
    memcpy(&identify.cameraColorMatrix2, &raw->imgdata.color.rgb_cam, sizeof(raw->imgdata.color.rgb_cam));
    memcpy(&identify.cameraXYZMatrix,    &raw->imgdata.color.cam_xyz, sizeof(raw->imgdata.color.cam_xyz));

    if (raw->imgdata.idata.filters)
    {
        if (!raw->imgdata.idata.cdesc[3])
        {
            raw->imgdata.idata.cdesc[3] = 'G';
        }

        for (int i=0; i < 16; i++)
        {
            identify.filterPattern.append(raw->imgdata.idata.cdesc[raw->COLOR(i >> 1,i & 1)]);
        }

        identify.colorKeys = raw->imgdata.idata.cdesc;
    }

    for(int c = 0 ; c < raw->imgdata.idata.colors ; c++)
    {
        identify.daylightMult[c] = raw->imgdata.color.pre_mul[c];
    }

    if (raw->imgdata.color.cam_mul[0] > 0)
    {
        for(int c = 0 ; c < 4 ; c++)
        {
            identify.cameraMult[c] = raw->imgdata.color.cam_mul[c];
        }
    }
}

bool KDcraw::Private::loadFromLibraw(const QString& filePath, QByteArray& imageData,
                                     int& width, int& height, int& rgbmax)
{
    m_parent->m_cancel = false;

    LibRaw raw;
    // Set progress call back function.
    raw.set_progress_handler(callbackForLibRaw, this);

    QByteArray deadpixelPath = QFile::encodeName(m_parent->m_rawDecodingSettings.deadPixelMap);
    QByteArray cameraProfile = QFile::encodeName(m_parent->m_rawDecodingSettings.inputProfile);
    QByteArray outputProfile = QFile::encodeName(m_parent->m_rawDecodingSettings.outputProfile);

    if (!m_parent->m_rawDecodingSettings.autoBrightness)
    {
        // Use a fixed white level, ignoring the image histogram.
        raw.imgdata.params.no_auto_bright = 1;
    }

    if (m_parent->m_rawDecodingSettings.sixteenBitsImage)
    {
        // (-4) 16bit ppm output
        raw.imgdata.params.output_bps = 16;
    }

    if (m_parent->m_rawDecodingSettings.halfSizeColorImage)
    {
        // (-h) Half-size color image (3x faster than -q).
        raw.imgdata.params.half_size = 1;
    }

    if (m_parent->m_rawDecodingSettings.RGBInterpolate4Colors)
    {
        // (-f) Interpolate RGB as four colors.
        raw.imgdata.params.four_color_rgb = 1;
    }

    if (m_parent->m_rawDecodingSettings.DontStretchPixels)
    {
        // (-j) Do not stretch the image to its correct aspect ratio.
        raw.imgdata.params.use_fuji_rotate = 1;
    }

    // (-H) Unclip highlight color.
    raw.imgdata.params.highlight = m_parent->m_rawDecodingSettings.unclipColors;

    if (m_parent->m_rawDecodingSettings.brightness != 1.0)
    {
        // (-b) Set Brightness value.
        raw.imgdata.params.bright = m_parent->m_rawDecodingSettings.brightness;
    }

    if (m_parent->m_rawDecodingSettings.enableBlackPoint)
    {
        // (-k) Set Black Point value.
        raw.imgdata.params.user_black = m_parent->m_rawDecodingSettings.blackPoint;
    }

    if (m_parent->m_rawDecodingSettings.enableWhitePoint)
    {
        // (-S) Set White Point value (saturation).
        raw.imgdata.params.user_sat = m_parent->m_rawDecodingSettings.whitePoint;
    }

    if (m_parent->m_rawDecodingSettings.medianFilterPasses > 0)
    {
        // (-m) After interpolation, clean up color artifacts by repeatedly applying a 3x3 median filter to the R-G and B-G channels.
        raw.imgdata.params.med_passes = m_parent->m_rawDecodingSettings.medianFilterPasses;
    }

    if (!m_parent->m_rawDecodingSettings.deadPixelMap.isEmpty())
    {
        // (-P) Read the dead pixel list from this file.
        raw.imgdata.params.bad_pixels = deadpixelPath.data();
    }

    switch (m_parent->m_rawDecodingSettings.whiteBalance)
    {
        case RawDecodingSettings::NONE:
        {
            break;
        }
        case RawDecodingSettings::CAMERA:
        {
            // (-w) Use camera white balance, if possible.
            raw.imgdata.params.use_camera_wb = 1;
            break;
        }
        case RawDecodingSettings::AUTO:
        {
            // (-a) Use automatic white balance.
            raw.imgdata.params.use_auto_wb = 1;
            break;
        }
        case RawDecodingSettings::CUSTOM:
        {
            /* Convert between Temperature and RGB.
             */
            double T;
            double RGB[3];
            double xD, yD, X, Y, Z;
            DcrawInfoContainer identify;
            T = m_parent->m_rawDecodingSettings.customWhiteBalance;

            /* Here starts the code picked and adapted from ufraw (0.12.1)
               to convert Temperature + green multiplier to RGB multipliers
            */
            /* Convert between Temperature and RGB.
             * Base on information from http://www.brucelindbloom.com/
             * The fit for D-illuminant between 4000K and 12000K are from CIE
             * The generalization to 2000K < T < 4000K and the blackbody fits
             * are my own and should be taken with a grain of salt.
             */
            const double XYZ_to_RGB[3][3] = {
                                                { 3.24071,  -0.969258,  0.0556352 },
                                                {-1.53726,  1.87599,    -0.203996 },
                                                {-0.498571, 0.0415557,  1.05707   }
                                            };

            // Fit for CIE Daylight illuminant
            if (T <= 4000)
            {
                xD = 0.27475e9/(T*T*T) - 0.98598e6/(T*T) + 1.17444e3/T + 0.145986;
            }
            else if (T <= 7000)
            {
                xD = -4.6070e9/(T*T*T) + 2.9678e6/(T*T) + 0.09911e3/T + 0.244063;
            }
            else
            {
                xD = -2.0064e9/(T*T*T) + 1.9018e6/(T*T) + 0.24748e3/T + 0.237040;
            }

            yD     = -3*xD*xD + 2.87*xD - 0.275;
            X      = xD/yD;
            Y      = 1;
            Z      = (1-xD-yD)/yD;
            RGB[0] = X*XYZ_to_RGB[0][0] + Y*XYZ_to_RGB[1][0] + Z*XYZ_to_RGB[2][0];
            RGB[1] = X*XYZ_to_RGB[0][1] + Y*XYZ_to_RGB[1][1] + Z*XYZ_to_RGB[2][1];
            RGB[2] = X*XYZ_to_RGB[0][2] + Y*XYZ_to_RGB[1][2] + Z*XYZ_to_RGB[2][2];
            /* End of the code picked to ufraw
            */

            RGB[1] = RGB[1] / m_parent->m_rawDecodingSettings.customWhiteBalanceGreen;

            /* By default, decraw override his default D65 WB
               We need to keep it as a basis : if not, colors with some
               DSLR will have a high dominant of color that will lead to
               a completely wrong WB
            */
            if (rawFileIdentify(identify, filePath))
            {
                RGB[0] = identify.daylightMult[0] / RGB[0];
                RGB[1] = identify.daylightMult[1] / RGB[1];
                RGB[2] = identify.daylightMult[2] / RGB[2];
            }
            else
            {
                RGB[0] = 1.0 / RGB[0];
                RGB[1] = 1.0 / RGB[1];
                RGB[2] = 1.0 / RGB[2];
                qCDebug(LIBKDCRAW_LOG) << "Warning: cannot get daylight multipliers";
            }

            // (-r) set Raw Color Balance Multipliers.
            raw.imgdata.params.user_mul[0] = RGB[0];
            raw.imgdata.params.user_mul[1] = RGB[1];
            raw.imgdata.params.user_mul[2] = RGB[2];
            raw.imgdata.params.user_mul[3] = RGB[1];
            break;
        }
        case RawDecodingSettings::AERA:
        {
            // (-A) Calculate the white balance by averaging a rectangular area from image.
            raw.imgdata.params.greybox[0] = m_parent->m_rawDecodingSettings.whiteBalanceArea.left();
            raw.imgdata.params.greybox[1] = m_parent->m_rawDecodingSettings.whiteBalanceArea.top();
            raw.imgdata.params.greybox[2] = m_parent->m_rawDecodingSettings.whiteBalanceArea.width();
            raw.imgdata.params.greybox[3] = m_parent->m_rawDecodingSettings.whiteBalanceArea.height();
            break;
        }
    }

    // (-q) Use an interpolation method.
    raw.imgdata.params.user_qual = m_parent->m_rawDecodingSettings.RAWQuality;

    switch (m_parent->m_rawDecodingSettings.NRType)
    {
        case RawDecodingSettings::WAVELETSNR:
        {
            // (-n) Use wavelets to erase noise while preserving real detail.
            raw.imgdata.params.threshold    = m_parent->m_rawDecodingSettings.NRThreshold;
            break;
        }
        case RawDecodingSettings::FBDDNR:
        {
            // (100 - 1000) => (1 - 10) conversion
            raw.imgdata.params.fbdd_noiserd = lround(m_parent->m_rawDecodingSettings.NRThreshold / 100.0);
            break;
        }
#if !LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0, 19)
        case RawDecodingSettings::LINENR:
        {
            // (100 - 1000) => (0.001 - 0.02) conversion.
            raw.imgdata.params.linenoise    = m_parent->m_rawDecodingSettings.NRThreshold * 2.11E-5 + 0.00111111;
            raw.imgdata.params.cfaline      = true;
            break;
        }

        case RawDecodingSettings::IMPULSENR:
        {
            // (100 - 1000) => (0.005 - 0.05) conversion.
            raw.imgdata.params.lclean       = m_parent->m_rawDecodingSettings.NRThreshold     * 5E-5;
            raw.imgdata.params.cclean       = m_parent->m_rawDecodingSettings.NRChroThreshold * 5E-5;
            raw.imgdata.params.cfa_clean    = true;
            break;
        }
#endif
        default:   // No Noise Reduction
        {
            raw.imgdata.params.threshold    = 0;
            raw.imgdata.params.fbdd_noiserd = 0;
#if !LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0, 19)
            raw.imgdata.params.linenoise    = 0;
            raw.imgdata.params.cfaline      = false;
            raw.imgdata.params.lclean       = 0;
            raw.imgdata.params.cclean       = 0;
            raw.imgdata.params.cfa_clean    = false;
#endif
            break;
        }
    }

#if !LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0, 19)
    // Chromatic aberration correction.
    raw.imgdata.params.ca_correc  = m_parent->m_rawDecodingSettings.enableCACorrection;
    raw.imgdata.params.cared      = m_parent->m_rawDecodingSettings.caMultiplier[0];
    raw.imgdata.params.cablue     = m_parent->m_rawDecodingSettings.caMultiplier[1];
#endif

    // Exposure Correction before interpolation.
    raw.imgdata.params.exp_correc = m_parent->m_rawDecodingSettings.expoCorrection;
    raw.imgdata.params.exp_shift  = m_parent->m_rawDecodingSettings.expoCorrectionShift;
    raw.imgdata.params.exp_preser = m_parent->m_rawDecodingSettings.expoCorrectionHighlight;

    switch (m_parent->m_rawDecodingSettings.inputColorSpace)
    {
        case RawDecodingSettings::EMBEDDED:
        {
            // (-p embed) Use input profile from RAW file to define the camera's raw colorspace.
            raw.imgdata.params.camera_profile = (char*)"embed";
            break;
        }
        case RawDecodingSettings::CUSTOMINPUTCS:
        {
            if (!m_parent->m_rawDecodingSettings.inputProfile.isEmpty())
            {
                // (-p) Use input profile file to define the camera's raw colorspace.
                raw.imgdata.params.camera_profile = cameraProfile.data();
            }
            break;
        }
        default:
        {
            // No input profile
            break;
        }
    }

    switch (m_parent->m_rawDecodingSettings.outputColorSpace)
    {
        case RawDecodingSettings::CUSTOMOUTPUTCS:
        {
            if (!m_parent->m_rawDecodingSettings.outputProfile.isEmpty())
            {
                // (-o) Use ICC profile file to define the output colorspace.
                raw.imgdata.params.output_profile = outputProfile.data();
            }
            break;
        }
        default:
        {
            // (-o) Define the output colorspace.
            raw.imgdata.params.output_color = m_parent->m_rawDecodingSettings.outputColorSpace;
            break;
        }
    }

    //-- Extended demosaicing settings ----------------------------------------------------------

    raw.imgdata.params.dcb_iterations = m_parent->m_rawDecodingSettings.dcbIterations;
    raw.imgdata.params.dcb_enhance_fl = m_parent->m_rawDecodingSettings.dcbEnhanceFl;
#if !LIBRAW_COMPILE_CHECK_VERSION_NOTLESS(0, 19)
    raw.imgdata.params.eeci_refine    = m_parent->m_rawDecodingSettings.eeciRefine;
    raw.imgdata.params.es_med_passes  = m_parent->m_rawDecodingSettings.esMedPasses;
#endif

    //-------------------------------------------------------------------------------------------

    setProgress(0.1);

    qCDebug(LIBKDCRAW_LOG) << filePath;
    qCDebug(LIBKDCRAW_LOG) << m_parent->m_rawDecodingSettings;

    int ret = raw.open_file((const char*)(QFile::encodeName(filePath)).constData());

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run open_file: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    if (m_parent->m_cancel)
    {
        raw.recycle();
        return false;
    }

    setProgress(0.2);

    ret = raw.unpack();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run unpack: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    if (m_parent->m_cancel)
    {
        raw.recycle();
        return false;
    }

    setProgress(0.25);

    if (m_parent->m_rawDecodingSettings.fixColorsHighlights)
    {
        qCDebug(LIBKDCRAW_LOG) << "Applying LibRaw highlights adjustments";
        // 1.0 is fallback to default value
        raw.imgdata.params.adjust_maximum_thr = 1.0;
    }
    else
    {
        qCDebug(LIBKDCRAW_LOG) << "Disabling LibRaw highlights adjustments";
        // 0.0 disables this feature
        raw.imgdata.params.adjust_maximum_thr = 0.0;
    }

    ret = raw.dcraw_process();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run dcraw_process: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    if (m_parent->m_cancel)
    {
        raw.recycle();
        return false;
    }

    setProgress(0.3);

    libraw_processed_image_t* img = raw.dcraw_make_mem_image(&ret);

    if(!img)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run dcraw_make_mem_image: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    if (m_parent->m_cancel)
    {
        // Clear memory allocation. Introduced with LibRaw 0.11.0
        raw.dcraw_clear_mem(img);
        raw.recycle();
        return false;
    }

    setProgress(0.35);

    width  = img->width;
    height = img->height;
    rgbmax = (1 << img->bits)-1;

    if (img->colors == 3)
    {
        imageData = QByteArray((const char*)img->data, (int)img->data_size);
    }
    else
    {
        // img->colors == 1 (Grayscale) : convert to RGB
        imageData = QByteArray();

        for (int i = 0 ; i < (int)img->data_size ; ++i)
        {
            for (int j = 0 ; j < 3 ; ++j)
            {
                imageData.append(img->data[i]);
            }
        }
    }

    // Clear memory allocation. Introduced with LibRaw 0.11.0
    raw.dcraw_clear_mem(img);
    raw.recycle();

    if (m_parent->m_cancel)
    {
        return false;
    }

    setProgress(0.4);

    qCDebug(LIBKDCRAW_LOG) << "LibRaw: data info: width=" << width
             << " height=" << height
             << " rgbmax=" << rgbmax;

    return true;
}

bool KDcraw::Private::loadEmbeddedPreview(QByteArray& imgData, LibRaw& raw)
{
    int ret = raw.unpack_thumb();

    if (ret != LIBRAW_SUCCESS)
    {
        raw.recycle();
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run unpack_thumb: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    libraw_processed_image_t* const thumb = raw.dcraw_make_mem_thumb(&ret);

    if(!thumb)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run dcraw_make_mem_thumb: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    if(thumb->type == LIBRAW_IMAGE_BITMAP)
    {
        createPPMHeader(imgData, thumb);
    }
    else
    {
        imgData = QByteArray((const char*)thumb->data, (int)thumb->data_size);
    }

    // Clear memory allocation. Introduced with LibRaw 0.11.0
    raw.dcraw_clear_mem(thumb);
    raw.recycle();

    if ( imgData.isEmpty() )
    {
        qCDebug(LIBKDCRAW_LOG) << "Failed to load JPEG thumb from LibRaw!";
        return false;
    }

    return true;
}

bool KDcraw::Private::loadHalfPreview(QImage& image, LibRaw& raw)
{
    raw.imgdata.params.use_auto_wb   = 1;         // Use automatic white balance.
    raw.imgdata.params.use_camera_wb = 1;         // Use camera white balance, if possible.
    raw.imgdata.params.half_size     = 1;         // Half-size color image (3x faster than -q).
    QByteArray imgData;

    int ret = raw.unpack();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run unpack: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    ret = raw.dcraw_process();

    if (ret != LIBRAW_SUCCESS)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run dcraw_process: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    libraw_processed_image_t* halfImg = raw.dcraw_make_mem_image(&ret);

    if(!halfImg)
    {
        qCDebug(LIBKDCRAW_LOG) << "LibRaw: failed to run dcraw_make_mem_image: " << libraw_strerror(ret);
        raw.recycle();
        return false;
    }

    Private::createPPMHeader(imgData, halfImg);
    // Clear memory allocation. Introduced with LibRaw 0.11.0
    raw.dcraw_clear_mem(halfImg);
    raw.recycle();

    if ( imgData.isEmpty() )
    {
        qCDebug(LIBKDCRAW_LOG) << "Failed to load half preview from LibRaw!";
        return false;
    }

    if (!image.loadFromData(imgData))
    {
        qCDebug(LIBKDCRAW_LOG) << "Failed to load PPM data from LibRaw!";
        return false;
    }

    return true;
}

}  // namespace KDcrawIface
