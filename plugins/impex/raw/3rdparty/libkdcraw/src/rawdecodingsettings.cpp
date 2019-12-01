/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2006-12-09
 * @brief  Raw decoding settings
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2006-2013 by Marcel Wiesweg
 *         <a href="mailto:marcel dot wiesweg at gmx dot de">marcel dot wiesweg at gmx dot de</a>
 * @author Copyright (C) 2007-2008 by Guillaume Castagnino
 *         <a href="mailto:casta at xwing dot info">casta at xwing dot info</a>
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

#define OPTIONFIXCOLORSHIGHLIGHTSENTRY                 "FixColorsHighlights"
#define OPTIONDECODESIXTEENBITENTRY                    "SixteenBitsImage"
#define OPTIONWHITEBALANCEENTRY                        "White Balance"
#define OPTIONCUSTOMWHITEBALANCEENTRY                  "Custom White Balance"
#define OPTIONCUSTOMWBGREENENTRY                       "Custom White Balance Green"
#define OPTIONFOURCOLORRGBENTRY                        "Four Color RGB"
#define OPTIONUNCLIPCOLORSENTRY                        "Unclip Color"
// Wrong spelling, but do not fix it since it is a configuration key
// krazy:cond=spelling
#define OPTIONDONTSTRETCHPIXELSENTRY                   "Dont Stretch Pixels"
// krazy:endcond=spelling
#define OPTIONMEDIANFILTERPASSESENTRY                  "Median Filter Passes"
#define OPTIONNOISEREDUCTIONTYPEENTRY                  "Noise Reduction Type"
#define OPTIONNOISEREDUCTIONTHRESHOLDENTRY             "Noise Reduction Threshold"
#define OPTIONUSECACORRECTIONENTRY                     "EnableCACorrection"
#define OPTIONCAREDMULTIPLIERENTRY                     "caRedMultiplier"
#define OPTIONCABLUEMULTIPLIERENTRY                    "caBlueMultiplier"
#define OPTIONAUTOBRIGHTNESSENTRY                      "AutoBrightness"
#define OPTIONDECODINGQUALITYENTRY                     "Decoding Quality"
#define OPTIONINPUTCOLORSPACEENTRY                     "Input Color Space"
#define OPTIONOUTPUTCOLORSPACEENTRY                    "Output Color Space"
#define OPTIONINPUTCOLORPROFILEENTRY                   "Input Color Profile"
#define OPTIONOUTPUTCOLORPROFILEENTRY                  "Output Color Profile"
#define OPTIONBRIGHTNESSMULTIPLIERENTRY                "Brightness Multiplier"
#define OPTIONUSEBLACKPOINTENTRY                       "Use Black Point"
#define OPTIONBLACKPOINTENTRY                          "Black Point"
#define OPTIONUSEWHITEPOINTENTRY                       "Use White Point"
#define OPTIONWHITEPOINTENTRY                          "White Point"

//-- Extended demosaicing settings ----------------------------------------------------------

#define OPTIONDCBITERATIONSENTRY                       "Dcb Iterations"
#define OPTIONDCBENHANCEFLENTRY                        "Dcb Enhance Filter"
#define OPTIONEECIREFINEENTRY                          "Eeci Refine"
#define OPTIONESMEDPASSESENTRY                         "Es Median Filter Passes"
#define OPTIONNRCHROMINANCETHRESHOLDENTRY              "Noise Reduction Chrominance Threshold"
#define OPTIONEXPOCORRECTIONENTRY                      "Expo Correction"
#define OPTIONEXPOCORRECTIONSHIFTENTRY                 "Expo Correction Shift"
#define OPTIONEXPOCORRECTIONHIGHLIGHTENTRY             "Expo Correction Highlight"

#include "rawdecodingsettings.h"

namespace KDcrawIface
{

RawDecodingSettings::RawDecodingSettings()
{
    fixColorsHighlights        = false;
    autoBrightness             = true;
    sixteenBitsImage           = false;
    brightness                 = 1.0;
    RAWQuality                 = BILINEAR;
    inputColorSpace            = NOINPUTCS;
    outputColorSpace           = SRGB;
    RGBInterpolate4Colors      = false;
    DontStretchPixels          = false;
    unclipColors               = 0;
    whiteBalance               = CAMERA;
    customWhiteBalance         = 6500;
    customWhiteBalanceGreen    = 1.0;
    medianFilterPasses         = 0;

    halfSizeColorImage         = false;

    enableBlackPoint           = false;
    blackPoint                 = 0;

    enableWhitePoint           = false;
    whitePoint                 = 0;

    NRType                     = NONR;
    NRThreshold                = 0;

    enableCACorrection         = false;
    caMultiplier[0]            = 0.0;
    caMultiplier[1]            = 0.0;

    inputProfile               = QString();
    outputProfile              = QString();

    deadPixelMap               = QString();

    whiteBalanceArea           = QRect();

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations              = -1;
    dcbEnhanceFl               = false;
    eeciRefine                 = false;
    esMedPasses                = 0;
    NRChroThreshold            = 0;
    expoCorrection             = false;
    expoCorrectionShift        = 1.0;
    expoCorrectionHighlight    = 0.0;
}

RawDecodingSettings::~RawDecodingSettings()
{
}

RawDecodingSettings& RawDecodingSettings::operator=(const RawDecodingSettings& o)
{
    fixColorsHighlights     = o.fixColorsHighlights;
    autoBrightness          = o.autoBrightness;
    sixteenBitsImage        = o.sixteenBitsImage;
    brightness              = o.brightness;
    RAWQuality              = o.RAWQuality;
    inputColorSpace         = o.inputColorSpace;
    outputColorSpace        = o.outputColorSpace;
    RGBInterpolate4Colors   = o.RGBInterpolate4Colors;
    DontStretchPixels       = o.DontStretchPixels;
    unclipColors            = o.unclipColors;
    whiteBalance            = o.whiteBalance;
    customWhiteBalance      = o.customWhiteBalance;
    customWhiteBalanceGreen = o.customWhiteBalanceGreen;
    halfSizeColorImage      = o.halfSizeColorImage;
    enableBlackPoint        = o.enableBlackPoint;
    blackPoint              = o.blackPoint;
    enableWhitePoint        = o.enableWhitePoint;
    whitePoint              = o.whitePoint;
    NRType                  = o.NRType;
    NRThreshold             = o.NRThreshold;
    enableCACorrection      = o.enableCACorrection;
    caMultiplier[0]         = o.caMultiplier[0];
    caMultiplier[1]         = o.caMultiplier[1];
    medianFilterPasses      = o.medianFilterPasses;
    inputProfile            = o.inputProfile;
    outputProfile           = o.outputProfile;
    deadPixelMap            = o.deadPixelMap;
    whiteBalanceArea        = o.whiteBalanceArea;

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = o.dcbIterations;
    dcbEnhanceFl            = o.dcbEnhanceFl;
    eeciRefine              = o.eeciRefine;
    esMedPasses             = o.esMedPasses;
    NRChroThreshold         = o.NRChroThreshold;
    expoCorrection          = o.expoCorrection;
    expoCorrectionShift     = o.expoCorrectionShift;
    expoCorrectionHighlight = o.expoCorrectionHighlight;

    return *this;
}

bool RawDecodingSettings::operator==(const RawDecodingSettings& o) const
{
    return fixColorsHighlights     == o.fixColorsHighlights
        && autoBrightness          == o.autoBrightness
        && sixteenBitsImage        == o.sixteenBitsImage
        && brightness              == o.brightness
        && RAWQuality              == o.RAWQuality
        && inputColorSpace         == o.inputColorSpace
        && outputColorSpace        == o.outputColorSpace
        && RGBInterpolate4Colors   == o.RGBInterpolate4Colors
        && DontStretchPixels       == o.DontStretchPixels
        && unclipColors            == o.unclipColors
        && whiteBalance            == o.whiteBalance
        && customWhiteBalance      == o.customWhiteBalance
        && customWhiteBalanceGreen == o.customWhiteBalanceGreen
        && halfSizeColorImage      == o.halfSizeColorImage
        && enableBlackPoint        == o.enableBlackPoint
        && blackPoint              == o.blackPoint
        && enableWhitePoint        == o.enableWhitePoint
        && whitePoint              == o.whitePoint
        && NRType                  == o.NRType
        && NRThreshold             == o.NRThreshold
        && enableCACorrection      == o.enableCACorrection
        && caMultiplier[0]         == o.caMultiplier[0]
        && caMultiplier[1]         == o.caMultiplier[1]
        && medianFilterPasses      == o.medianFilterPasses
        && inputProfile            == o.inputProfile
        && outputProfile           == o.outputProfile
        && deadPixelMap            == o.deadPixelMap
        && whiteBalanceArea        == o.whiteBalanceArea

        //-- Extended demosaicing settings ----------------------------------------------------------

        && dcbIterations           == o.dcbIterations
        && dcbEnhanceFl            == o.dcbEnhanceFl
        && eeciRefine              == o.eeciRefine
        && esMedPasses             == o.esMedPasses
        && NRChroThreshold         == o.NRChroThreshold
        && expoCorrection          == o.expoCorrection
        && expoCorrectionShift     == o.expoCorrectionShift
        && expoCorrectionHighlight == o.expoCorrectionHighlight
        ;
}

void RawDecodingSettings::optimizeTimeLoading()
{
    fixColorsHighlights     = false;
    autoBrightness          = true;
    sixteenBitsImage        = true;
    brightness              = 1.0;
    RAWQuality              = BILINEAR;
    inputColorSpace         = NOINPUTCS;
    outputColorSpace        = SRGB;
    RGBInterpolate4Colors   = false;
    DontStretchPixels       = false;
    unclipColors            = 0;
    whiteBalance            = CAMERA;
    customWhiteBalance      = 6500;
    customWhiteBalanceGreen = 1.0;
    halfSizeColorImage      = true;
    medianFilterPasses      = 0;

    enableBlackPoint        = false;
    blackPoint              = 0;

    enableWhitePoint        = false;
    whitePoint              = 0;

    NRType                  = NONR;
    NRThreshold             = 0;

    enableCACorrection      = false;
    caMultiplier[0]         = 0.0;
    caMultiplier[1]         = 0.0;

    inputProfile            = QString();
    outputProfile           = QString();

    deadPixelMap            = QString();

    whiteBalanceArea        = QRect();

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = -1;
    dcbEnhanceFl            = false;
    eeciRefine              = false;
    esMedPasses             = 0;
    NRChroThreshold         = 0;
    expoCorrection          = false;
    expoCorrectionShift     = 1.0;
    expoCorrectionHighlight = 0.0;
}

void RawDecodingSettings::readSettings(KConfigGroup& group)
{
    RawDecodingSettings defaultPrm;

    fixColorsHighlights     = group.readEntry(OPTIONFIXCOLORSHIGHLIGHTSENTRY,     defaultPrm.fixColorsHighlights);
    sixteenBitsImage        = group.readEntry(OPTIONDECODESIXTEENBITENTRY,        defaultPrm.sixteenBitsImage);
    whiteBalance            = (WhiteBalance)
                              group.readEntry(OPTIONWHITEBALANCEENTRY,            (int)defaultPrm.whiteBalance);
    customWhiteBalance      = group.readEntry(OPTIONCUSTOMWHITEBALANCEENTRY,      defaultPrm.customWhiteBalance);
    customWhiteBalanceGreen = group.readEntry(OPTIONCUSTOMWBGREENENTRY,           defaultPrm.customWhiteBalanceGreen);
    RGBInterpolate4Colors   = group.readEntry(OPTIONFOURCOLORRGBENTRY,            defaultPrm.RGBInterpolate4Colors);
    unclipColors            = group.readEntry(OPTIONUNCLIPCOLORSENTRY,            defaultPrm.unclipColors);
    DontStretchPixels       = group.readEntry(OPTIONDONTSTRETCHPIXELSENTRY,       defaultPrm.DontStretchPixels);
    NRType                  = (NoiseReduction)
                              group.readEntry(OPTIONNOISEREDUCTIONTYPEENTRY,      (int)defaultPrm.NRType);
    brightness              = group.readEntry(OPTIONBRIGHTNESSMULTIPLIERENTRY,    defaultPrm.brightness);
    enableBlackPoint        = group.readEntry(OPTIONUSEBLACKPOINTENTRY,           defaultPrm.enableBlackPoint);
    blackPoint              = group.readEntry(OPTIONBLACKPOINTENTRY,              defaultPrm.blackPoint);
    enableWhitePoint        = group.readEntry(OPTIONUSEWHITEPOINTENTRY,           defaultPrm.enableWhitePoint);
    whitePoint              = group.readEntry(OPTIONWHITEPOINTENTRY,              defaultPrm.whitePoint);
    medianFilterPasses      = group.readEntry(OPTIONMEDIANFILTERPASSESENTRY,      defaultPrm.medianFilterPasses);
    NRThreshold             = group.readEntry(OPTIONNOISEREDUCTIONTHRESHOLDENTRY, defaultPrm.NRThreshold);
    enableCACorrection      = group.readEntry(OPTIONUSECACORRECTIONENTRY,         defaultPrm.enableCACorrection);
    caMultiplier[0]         = group.readEntry(OPTIONCAREDMULTIPLIERENTRY,         defaultPrm.caMultiplier[0]);
    caMultiplier[1]         = group.readEntry(OPTIONCABLUEMULTIPLIERENTRY,        defaultPrm.caMultiplier[1]);
    RAWQuality              = (DecodingQuality)
                              group.readEntry(OPTIONDECODINGQUALITYENTRY,         (int)defaultPrm.RAWQuality);
    outputColorSpace        = (OutputColorSpace)
                              group.readEntry(OPTIONOUTPUTCOLORSPACEENTRY,        (int)defaultPrm.outputColorSpace);
    autoBrightness          = group.readEntry(OPTIONAUTOBRIGHTNESSENTRY,          defaultPrm.autoBrightness);

    //-- Extended demosaicing settings ----------------------------------------------------------

    dcbIterations           = group.readEntry(OPTIONDCBITERATIONSENTRY,           defaultPrm.dcbIterations);
    dcbEnhanceFl            = group.readEntry(OPTIONDCBENHANCEFLENTRY,            defaultPrm.dcbEnhanceFl);
    eeciRefine              = group.readEntry(OPTIONEECIREFINEENTRY,              defaultPrm.eeciRefine);
    esMedPasses             = group.readEntry(OPTIONESMEDPASSESENTRY,             defaultPrm.esMedPasses);
    NRChroThreshold         = group.readEntry(OPTIONNRCHROMINANCETHRESHOLDENTRY,  defaultPrm.NRChroThreshold);
    expoCorrection          = group.readEntry(OPTIONEXPOCORRECTIONENTRY,          defaultPrm.expoCorrection);
    expoCorrectionShift     = group.readEntry(OPTIONEXPOCORRECTIONSHIFTENTRY,     defaultPrm.expoCorrectionShift);
    expoCorrectionHighlight = group.readEntry(OPTIONEXPOCORRECTIONHIGHLIGHTENTRY, defaultPrm.expoCorrectionHighlight);
}

void RawDecodingSettings::writeSettings(KConfigGroup& group)
{
    group.writeEntry(OPTIONFIXCOLORSHIGHLIGHTSENTRY,     fixColorsHighlights);
    group.writeEntry(OPTIONDECODESIXTEENBITENTRY,        sixteenBitsImage);
    group.writeEntry(OPTIONWHITEBALANCEENTRY,            (int)whiteBalance);
    group.writeEntry(OPTIONCUSTOMWHITEBALANCEENTRY,      customWhiteBalance);
    group.writeEntry(OPTIONCUSTOMWBGREENENTRY,           customWhiteBalanceGreen);
    group.writeEntry(OPTIONFOURCOLORRGBENTRY,            RGBInterpolate4Colors);
    group.writeEntry(OPTIONUNCLIPCOLORSENTRY,            unclipColors);
    group.writeEntry(OPTIONDONTSTRETCHPIXELSENTRY,       DontStretchPixels);
    group.writeEntry(OPTIONNOISEREDUCTIONTYPEENTRY,      (int)NRType);
    group.writeEntry(OPTIONBRIGHTNESSMULTIPLIERENTRY,    brightness);
    group.writeEntry(OPTIONUSEBLACKPOINTENTRY,           enableBlackPoint);
    group.writeEntry(OPTIONBLACKPOINTENTRY,              blackPoint);
    group.writeEntry(OPTIONUSEWHITEPOINTENTRY,           enableWhitePoint);
    group.writeEntry(OPTIONWHITEPOINTENTRY,              whitePoint);
    group.writeEntry(OPTIONMEDIANFILTERPASSESENTRY,      medianFilterPasses);
    group.writeEntry(OPTIONNOISEREDUCTIONTHRESHOLDENTRY, NRThreshold);
    group.writeEntry(OPTIONUSECACORRECTIONENTRY,         enableCACorrection);
    group.writeEntry(OPTIONCAREDMULTIPLIERENTRY,         caMultiplier[0]);
    group.writeEntry(OPTIONCABLUEMULTIPLIERENTRY,        caMultiplier[1]);
    group.writeEntry(OPTIONDECODINGQUALITYENTRY,         (int)RAWQuality);
    group.writeEntry(OPTIONOUTPUTCOLORSPACEENTRY,        (int)outputColorSpace);
    group.writeEntry(OPTIONAUTOBRIGHTNESSENTRY,          autoBrightness);

    //-- Extended demosaicing settings ----------------------------------------------------------

    group.writeEntry(OPTIONDCBITERATIONSENTRY,           dcbIterations);
    group.writeEntry(OPTIONDCBENHANCEFLENTRY,            dcbEnhanceFl);
    group.writeEntry(OPTIONEECIREFINEENTRY,              eeciRefine);
    group.writeEntry(OPTIONESMEDPASSESENTRY,             esMedPasses);
    group.writeEntry(OPTIONNRCHROMINANCETHRESHOLDENTRY,  NRChroThreshold);
    group.writeEntry(OPTIONEXPOCORRECTIONENTRY,          expoCorrection);
    group.writeEntry(OPTIONEXPOCORRECTIONSHIFTENTRY,     expoCorrectionShift);
    group.writeEntry(OPTIONEXPOCORRECTIONHIGHLIGHTENTRY, expoCorrectionHighlight);
}

QDebug operator<<(QDebug dbg, const RawDecodingSettings& s)
{
    dbg.nospace() << endl;
    dbg.nospace() << "-- RAW DECODING SETTINGS --------------------------------" << endl;
    dbg.nospace() << "-- autoBrightness:          " << s.autoBrightness          << endl;
    dbg.nospace() << "-- sixteenBitsImage:        " << s.sixteenBitsImage        << endl;
    dbg.nospace() << "-- brightness:              " << s.brightness              << endl;
    dbg.nospace() << "-- RAWQuality:              " << s.RAWQuality              << endl;
    dbg.nospace() << "-- inputColorSpace:         " << s.inputColorSpace         << endl;
    dbg.nospace() << "-- outputColorSpace:        " << s.outputColorSpace        << endl;
    dbg.nospace() << "-- RGBInterpolate4Colors:   " << s.RGBInterpolate4Colors   << endl;
    dbg.nospace() << "-- DontStretchPixels:       " << s.DontStretchPixels       << endl;
    dbg.nospace() << "-- unclipColors:            " << s.unclipColors            << endl;
    dbg.nospace() << "-- whiteBalance:            " << s.whiteBalance            << endl;
    dbg.nospace() << "-- customWhiteBalance:      " << s.customWhiteBalance      << endl;
    dbg.nospace() << "-- customWhiteBalanceGreen: " << s.customWhiteBalanceGreen << endl;
    dbg.nospace() << "-- halfSizeColorImage:      " << s.halfSizeColorImage      << endl;
    dbg.nospace() << "-- enableBlackPoint:        " << s.enableBlackPoint        << endl;
    dbg.nospace() << "-- blackPoint:              " << s.blackPoint              << endl;
    dbg.nospace() << "-- enableWhitePoint:        " << s.enableWhitePoint        << endl;
    dbg.nospace() << "-- whitePoint:              " << s.whitePoint              << endl;
    dbg.nospace() << "-- NoiseReductionType:      " << s.NRType                  << endl;
    dbg.nospace() << "-- NoiseReductionThreshold: " << s.NRThreshold             << endl;
    dbg.nospace() << "-- enableCACorrection:      " << s.enableCACorrection      << endl;
    dbg.nospace() << "-- caMultiplier:            " << s.caMultiplier[0]
                  << ", "                           << s.caMultiplier[1]         << endl;
    dbg.nospace() << "-- medianFilterPasses:      " << s.medianFilterPasses      << endl;
    dbg.nospace() << "-- inputProfile:            " << s.inputProfile            << endl;
    dbg.nospace() << "-- outputProfile:           " << s.outputProfile           << endl;
    dbg.nospace() << "-- deadPixelMap:            " << s.deadPixelMap            << endl;
    dbg.nospace() << "-- whiteBalanceArea:        " << s.whiteBalanceArea        << endl;

    //-- Extended demosaicing settings ----------------------------------------------------------

    dbg.nospace() << "-- dcbIterations:           " << s.dcbIterations           << endl;
    dbg.nospace() << "-- dcbEnhanceFl:            " << s.dcbEnhanceFl            << endl;
    dbg.nospace() << "-- eeciRefine:              " << s.eeciRefine              << endl;
    dbg.nospace() << "-- esMedPasses:             " << s.esMedPasses             << endl;
    dbg.nospace() << "-- NRChrominanceThreshold:  " << s.NRChroThreshold         << endl;
    dbg.nospace() << "-- expoCorrection:          " << s.expoCorrection          << endl;
    dbg.nospace() << "-- expoCorrectionShift:     " << s.expoCorrectionShift     << endl;
    dbg.nospace() << "-- expoCorrectionHighlight: " << s.expoCorrectionHighlight << endl;
    dbg.nospace() << "---------------------------------------------------------" << endl;

    return dbg.space();
}

}  // namespace KDcrawIface
