/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2006-09-13
 * @brief  LibRaw settings widgets
 *
 * @author Copyright (C) 2006-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 * @author Copyright (C) 2006-2011 by Marcel Wiesweg
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

#include "dcrawsettingswidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QWhatsThis>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QIcon>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "kdcraw.h"
#include "rnuminput.h"
#include "rcombobox.h"
#include "rexpanderbox.h"
#include "libkdcraw_debug.h"

#include <kis_icon_utils.h>

namespace KDcrawIface
{

class Q_DECL_HIDDEN DcrawSettingsWidget::Private
{
public:

    Private()
    {
        autoBrightnessBox              = 0;
        sixteenBitsImage               = 0;
        fourColorCheckBox              = 0;
        brightnessLabel                = 0;
        brightnessSpinBox              = 0;
        blackPointCheckBox             = 0;
        blackPointSpinBox              = 0;
        whitePointCheckBox             = 0;
        whitePointSpinBox              = 0;
        whiteBalanceComboBox           = 0;
        whiteBalanceLabel              = 0;
        customWhiteBalanceSpinBox      = 0;
        customWhiteBalanceLabel        = 0;
        customWhiteBalanceGreenSpinBox = 0;
        customWhiteBalanceGreenLabel   = 0;
        unclipColorLabel               = 0;
        dontStretchPixelsCheckBox      = 0;
        RAWQualityComboBox             = 0;
        RAWQualityLabel                = 0;
        noiseReductionComboBox         = 0;
        NRSpinBox1                     = 0;
        NRSpinBox2                     = 0;
        NRLabel1                       = 0;
        NRLabel2                       = 0;
        enableCACorrectionBox          = 0;
        autoCACorrectionBox            = 0;
        caRedMultSpinBox               = 0;
        caBlueMultSpinBox              = 0;
        caRedMultLabel                 = 0;
        caBlueMultLabel                = 0;
        unclipColorComboBox            = 0;
        reconstructLabel               = 0;
        reconstructSpinBox             = 0;
        outputColorSpaceLabel          = 0;
        outputColorSpaceComboBox       = 0;
        demosaicingSettings            = 0;
        whiteBalanceSettings           = 0;
        correctionsSettings            = 0;
        colormanSettings               = 0;
        medianFilterPassesSpinBox      = 0;
        medianFilterPassesLabel        = 0;
        inIccUrlEdit                   = 0;
        outIccUrlEdit                  = 0;
        inputColorSpaceLabel           = 0;
        inputColorSpaceComboBox        = 0;
        fixColorsHighlightsBox         = 0;
        refineInterpolationBox         = 0;
        noiseReductionLabel            = 0;
        expoCorrectionBox              = 0;
        expoCorrectionShiftSpinBox     = 0;
        expoCorrectionHighlightSpinBox = 0;
        expoCorrectionShiftLabel       = 0;
        expoCorrectionHighlightLabel   = 0;
    }

    /** Convert Exposure correction shift E.V value from GUI to Linear value needs by libraw decoder.
     */
    double shiftExpoFromEvToLinear(double ev) const
    {
        // From GUI : -2.0EV => 0.25
        //            +3.0EV => 8.00
        return (1.55*ev + 3.35);
    }

    /** Convert Exposure correction shift Linear value from liraw decoder to E.V value needs by GUI.
     */
    double shiftExpoFromLinearToEv(double lin) const
    {
        // From GUI : 0.25 => -2.0EV
        //            8.00 => +3.0EV
        return ((lin-3.35) / 1.55);
    }

public:

    QWidget*         demosaicingSettings;
    QWidget*         whiteBalanceSettings;
    QWidget*         correctionsSettings;
    QWidget*         colormanSettings;

    QLabel*          whiteBalanceLabel;
    QLabel*          customWhiteBalanceLabel;
    QLabel*          customWhiteBalanceGreenLabel;
    QLabel*          brightnessLabel;
    QLabel*          RAWQualityLabel;
    QLabel*          NRLabel1;
    QLabel*          NRLabel2;
    QLabel*          caRedMultLabel;
    QLabel*          caBlueMultLabel;
    QLabel*          unclipColorLabel;
    QLabel*          reconstructLabel;
    QLabel*          inputColorSpaceLabel;
    QLabel*          outputColorSpaceLabel;
    QLabel*          medianFilterPassesLabel;
    QLabel*          noiseReductionLabel;
    QLabel*          expoCorrectionShiftLabel;
    QLabel*          expoCorrectionHighlightLabel;

    QCheckBox*       blackPointCheckBox;
    QCheckBox*       whitePointCheckBox;
    QCheckBox*       sixteenBitsImage;
    QCheckBox*       autoBrightnessBox;
    QCheckBox*       fourColorCheckBox;
    QCheckBox*       dontStretchPixelsCheckBox;
    QCheckBox*       enableCACorrectionBox;
    QCheckBox*       autoCACorrectionBox;
    QCheckBox*       fixColorsHighlightsBox;
    QCheckBox*       refineInterpolationBox;
    QCheckBox*       expoCorrectionBox;

    RFileSelector*   inIccUrlEdit;
    RFileSelector*   outIccUrlEdit;

    RComboBox*       noiseReductionComboBox;
    RComboBox*       whiteBalanceComboBox;
    RComboBox*       RAWQualityComboBox;
    RComboBox*       unclipColorComboBox;
    RComboBox*       inputColorSpaceComboBox;
    RComboBox*       outputColorSpaceComboBox;

    RIntNumInput*    customWhiteBalanceSpinBox;
    RIntNumInput*    reconstructSpinBox;
    RIntNumInput*    blackPointSpinBox;
    RIntNumInput*    whitePointSpinBox;
    RIntNumInput*    NRSpinBox1;
    RIntNumInput*    NRSpinBox2;
    RIntNumInput*    medianFilterPassesSpinBox;

    RDoubleNumInput* customWhiteBalanceGreenSpinBox;
    RDoubleNumInput* caRedMultSpinBox;
    RDoubleNumInput* caBlueMultSpinBox;
    RDoubleNumInput* brightnessSpinBox;
    RDoubleNumInput* expoCorrectionShiftSpinBox;
    RDoubleNumInput* expoCorrectionHighlightSpinBox;
};

DcrawSettingsWidget::DcrawSettingsWidget(QWidget* const parent, int advSettings)
    : RExpanderBox(parent), d(new Private)
{
    setup(advSettings);
}

void DcrawSettingsWidget::setup(int advSettings)
{
    setObjectName( QLatin1String("DCRawSettings Expander" ));

    // ---------------------------------------------------------------
    // DEMOSAICING Settings panel

    d->demosaicingSettings               = new QWidget(this);
    QGridLayout* const demosaicingLayout = new QGridLayout(d->demosaicingSettings);

    int line = 0;

    d->sixteenBitsImage = new QCheckBox(i18nc("@option:check", "16 bits color depth"), d->demosaicingSettings);
    d->sixteenBitsImage->setToolTip(i18nc("@info:whatsthis", "<p>If enabled, all RAW files will "
                                "be decoded in 16-bit color depth using a linear gamma curve. To "
                                "prevent dark picture rendering in the editor, it is recommended to "
                                "use Color Management in this mode.</p>"
                                "<p>If disabled, all RAW files will be decoded in 8-bit color "
                                "depth with a BT.709 gamma curve and a 99th-percentile white point. "
                                "This mode is faster than 16-bit decoding.</p>"));
    demosaicingLayout->addWidget(d->sixteenBitsImage, 0, 0, 1, 2);

    if (advSettings & SIXTEENBITS)
    {
        d->sixteenBitsImage->show();
        line = 1;
    }
    else
    {
        d->sixteenBitsImage->hide();
    }

    d->fourColorCheckBox = new QCheckBox(i18nc("@option:check", "Interpolate RGB as four colors"), d->demosaicingSettings);
    d->fourColorCheckBox->setToolTip(i18nc("@info:whatsthis", "<title>Interpolate RGB as four "
                                "colors</title>"
                                "<p>The default is to assume that all green pixels are the same. "
                                "If even-row green pixels are more sensitive to ultraviolet light "
                                "than odd-row this difference causes a mesh pattern in the output; "
                                "using this option solves this problem with minimal loss of detail.</p>"
                                "<p>To resume, this option blurs the image a little, but it "
                                "eliminates false 2x2 mesh patterns with VNG quality method or "
                                "mazes with AHD quality method.</p>"));
    demosaicingLayout->addWidget(d->fourColorCheckBox, line, 0, 1, line == 0 ? 2 : 3);
    line++;

    QLabel* const dcrawVersion = new QLabel(d->demosaicingSettings);
    dcrawVersion->setAlignment(Qt::AlignRight);
    dcrawVersion->setToolTip(i18nc("@info:tooltip", "Visit LibRaw project website"));
    dcrawVersion->setOpenExternalLinks(true);
    dcrawVersion->setTextFormat(Qt::RichText);
    dcrawVersion->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    dcrawVersion->setText(QString::fromLatin1("<a href=\"%1\">%2</a>")
                          .arg(QLatin1String("https://www.libraw.org"))
                          .arg(QString::fromLatin1("libraw %1").arg(KDcraw::librawVersion())));

    demosaicingLayout->addWidget(dcrawVersion, 0, 2, 1, 1);

    d->dontStretchPixelsCheckBox  = new QCheckBox(i18nc("@option:check", "Do not stretch or rotate pixels"), d->demosaicingSettings);
    d->dontStretchPixelsCheckBox->setToolTip(i18nc("@info:whatsthis",
                                "<title>Do not stretch or rotate pixels</title>"
                                "<p>For Fuji Super CCD cameras, show the image tilted 45 degrees. "
                                "For cameras with non-square pixels, do not stretch the image to "
                                "its correct aspect ratio. In any case, this option guarantees that "
                                "each output pixel corresponds to one RAW pixel.</p>"));
    demosaicingLayout->addWidget(d->dontStretchPixelsCheckBox, line, 0, 1, 3);
    line++;

    d->RAWQualityLabel    = new QLabel(i18nc("@label:listbox", "Quality:"), d->demosaicingSettings);
    d->RAWQualityComboBox = new RComboBox(d->demosaicingSettings);

    // Original dcraw demosaicing methods
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::BILINEAR, i18nc("@item:inlistbox Quality", "Bilinear"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::VNG,      i18nc("@item:inlistbox Quality", "VNG"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::PPG,      i18nc("@item:inlistbox Quality", "PPG"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::AHD,      i18nc("@item:inlistbox Quality", "AHD"));

    // Extended demosaicing method from GPL2 pack
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::DCB,      i18nc("@item:inlistbox Quality", "DCB"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::PL_AHD,   i18nc("@item:inlistbox Quality", "AHD v2"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::AFD,      i18nc("@item:inlistbox Quality", "AFD"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::VCD,      i18nc("@item:inlistbox Quality", "VCD"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::VCD_AHD,  i18nc("@item:inlistbox Quality", "VCD & AHD"));
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::LMMSE,    i18nc("@item:inlistbox Quality", "LMMSE"));
    // Extended demosaicing method from GPL3 pack
    d->RAWQualityComboBox->insertItem(RawDecodingSettings::AMAZE,    i18nc("@item:inlistbox Quality", "AMaZE"));

    // If Libraw do not support GPL2 pack, disable entries relevant.
    if (!KDcraw::librawUseGPL2DemosaicPack())
    {
        for (int i=RawDecodingSettings::DCB ; i <=RawDecodingSettings::LMMSE ; ++i)
            d->RAWQualityComboBox->combo()->setItemData(i, false, Qt::UserRole-1);
    }

    // If Libraw do not support GPL3 pack, disable entries relevant.
    if (!KDcraw::librawUseGPL3DemosaicPack())
    {
        d->RAWQualityComboBox->combo()->setItemData(RawDecodingSettings::AMAZE, false, Qt::UserRole-1);
    }

    d->RAWQualityComboBox->setDefaultIndex(RawDecodingSettings::BILINEAR);
    d->RAWQualityComboBox->setCurrentIndex(RawDecodingSettings::BILINEAR);
    d->RAWQualityComboBox->setToolTip(i18nc("@info:whatsthis", "<title>Quality (interpolation)</title>"
                                "<p>Select here the demosaicing method to use when decoding RAW "
                                "images. A demosaicing algorithm is a digital image process used to "
                                "interpolate a complete image from the partial raw data received "
                                "from the color-filtered image sensor, internal to many digital "
                                "cameras, in form of a matrix of colored pixels. Also known as CFA "
                                "interpolation or color reconstruction, another common spelling is "
                                "demosaicing. The following methods are available for demosaicing "
                                "RAW images:</p>"

                                // Original dcraw demosaicing methods

                                "<p><ul><li><emphasis strong='true'>Bilinear</emphasis>: use "
                                "high-speed but low-quality bilinear interpolation (default - for "
                                "slow computers). In this method, the red value of a non-red pixel "
                                "is computed as the average of the adjacent red pixels, and similarly "
                                "for blue and green.</li>"

                                "<li><emphasis strong='true'>VNG</emphasis>: use Variable Number "
                                "of Gradients interpolation. This method computes gradients near "
                                "the pixel of interest and uses the lower gradients (representing "
                                "smoother and more similar parts of the image) to make an estimate.</li>"

                                "<li><emphasis strong='true'>PPG</emphasis>: use Patterned-Pixel-"
                                "Grouping interpolation. Pixel Grouping uses assumptions about "
                                "natural scenery in making estimates. It has fewer color artifacts "
                                "on natural images than the Variable Number of Gradients method.</li>"

                                "<li><emphasis strong='true'>AHD</emphasis>: use Adaptive "
                                "Homogeneity-Directed interpolation. This method selects the "
                                "direction of interpolation so as to maximize a homogeneity metric, "
                                "thus typically minimizing color artifacts.</li>"

                                // Extended demosaicing method

                                "<li><emphasis strong='true'>DCB</emphasis>: DCB interpolation from "
                                "linuxphoto.org project.</li>"

                                "<li><emphasis strong='true'>AHD v2</emphasis>: modified AHD "
                                "interpolation using Variance of Color Differences method.</li>"

                                "<li><emphasis strong='true'>AFD</emphasis>: Adaptive Filtered "
                                "Demosaicing interpolation through 5 pass median filter from PerfectRaw "
                                "project.</li>"

                                "<li><emphasis strong='true'>VCD</emphasis>: Variance of Color "
                                "Differences interpolation.</li>"

                                "<li><emphasis strong='true'>VCD & AHD</emphasis>: Mixed demosaicing "
                                "between VCD and AHD.</li>"

                                "<li><emphasis strong='true'>LMMSE</emphasis>: color demosaicing via "
                                "directional linear minimum mean-square error estimation interpolation "
                                "from PerfectRaw.</li>"

                                "<li><emphasis strong='true'>AMaZE</emphasis>: Aliasing Minimization "
                                "interpolation and Zipper Elimination to apply color aberration removal "
                                "from RawTherapee project.</li></ul></p>"

                                "<p>Note: some methods can be unavailable if RAW decoder have been built "
                                "without extension packs.</p>"));

    demosaicingLayout->addWidget(d->RAWQualityLabel,    line, 0, 1, 1);
    demosaicingLayout->addWidget(d->RAWQualityComboBox, line, 1, 1, 2);
    line++;

    d->medianFilterPassesSpinBox = new RIntNumInput(d->demosaicingSettings);
    d->medianFilterPassesSpinBox->setRange(0, 10, 1);
    d->medianFilterPassesSpinBox->setDefaultValue(0);
    d->medianFilterPassesLabel   = new QLabel(i18nc("@label:slider", "Pass:"), d->whiteBalanceSettings);
    d->medianFilterPassesSpinBox->setToolTip( i18nc("@info:whatsthis", "<title>Pass</title>"
                                "<p>Set here the passes used by the median filter applied after "
                                "interpolation to Red-Green and Blue-Green channels.</p>"
                                "<p>This setting is only available for specific Quality options: "
                                "<emphasis strong='true'>Bilinear</emphasis>, <emphasis strong='true'>"
                                "VNG</emphasis>, <emphasis strong='true'>PPG</emphasis>, "
                                "<emphasis strong='true'>AHD</emphasis>, <emphasis strong='true'>"
                                "DCB</emphasis>, and <emphasis strong='true'>VCD & AHD</emphasis>.</p>"));
    demosaicingLayout->addWidget(d->medianFilterPassesLabel,   line, 0, 1, 1);
    demosaicingLayout->addWidget(d->medianFilterPassesSpinBox, line, 1, 1, 2);
    line++;

    d->refineInterpolationBox = new QCheckBox(i18nc("@option:check", "Refine interpolation"), d->demosaicingSettings);
    d->refineInterpolationBox->setToolTip(i18nc("@info:whatsthis", "<title>Refine interpolation</title>"
                                "<p>This setting is available only for few Quality options:</p>"
                                "<p><ul><li><emphasis strong='true'>DCB</emphasis>: turn on "
                                "the enhance interpolated colors filter.</li>"
                                "<li><emphasis strong='true'>VCD & AHD</emphasis>: turn on the "
                                "enhanced effective color interpolation (EECI) refine to improve "
                                "sharpness.</li></ul></p>"));
    demosaicingLayout->addWidget(d->refineInterpolationBox, line, 0, 1, 2);

    // If Libraw do not support GPL2 pack, disable options relevant.
    if (!KDcraw::librawUseGPL2DemosaicPack())
    {
        d->medianFilterPassesLabel->setEnabled(false);
        d->medianFilterPassesSpinBox->setEnabled(false);
        d->refineInterpolationBox->setEnabled(false);
    }

    addItem(d->demosaicingSettings, KisIconUtils::loadIcon("kdcraw").pixmap(16, 16), i18nc("@label", "Demosaicing"), QString("demosaicing"), true);

    // ---------------------------------------------------------------
    // WHITE BALANCE Settings Panel

    d->whiteBalanceSettings               = new QWidget(this);
    QGridLayout* const whiteBalanceLayout = new QGridLayout(d->whiteBalanceSettings);

    d->whiteBalanceLabel    = new QLabel(i18nc("@label:listbox", "Method:"), d->whiteBalanceSettings);
    d->whiteBalanceComboBox = new RComboBox(d->whiteBalanceSettings);
    d->whiteBalanceComboBox->insertItem(RawDecodingSettings::NONE,   i18nc("@item:inlistbox", "Default D65"));
    d->whiteBalanceComboBox->insertItem(RawDecodingSettings::CAMERA, i18nc("@item:inlistbox", "Camera"));
    d->whiteBalanceComboBox->insertItem(RawDecodingSettings::AUTO,   i18nc("@item:inlistbox set while balance automatically", "Automatic"));
    d->whiteBalanceComboBox->insertItem(RawDecodingSettings::CUSTOM, i18nc("@item:inlistbox set white balance manually", "Manual"));
    d->whiteBalanceComboBox->setDefaultIndex(RawDecodingSettings::CAMERA);
    d->whiteBalanceComboBox->setToolTip(i18nc("@info:whatsthis", "<title>White Balance</title>"
                                "<p>Configure the raw white balance:</p>"
                                "<p><ul><li><emphasis strong='true'>Default D65</emphasis>: "
                                "Use a standard daylight D65 white balance.</li>"
                                "<li><emphasis strong='true'>Camera</emphasis>: Use the white "
                                "balance specified by the camera. If not available, reverts to "
                                "default neutral white balance.</li>"
                                "<li><emphasis strong='true'>Automatic</emphasis>: Calculates an "
                                "automatic white balance averaging the entire image.</li>"
                                "<li><emphasis strong='true'>Manual</emphasis>: Set a custom "
                                "temperature and green level values.</li></ul></p>"));

    d->customWhiteBalanceSpinBox = new RIntNumInput(d->whiteBalanceSettings);
    d->customWhiteBalanceSpinBox->setRange(2000, 12000, 10);
    d->customWhiteBalanceSpinBox->setDefaultValue(6500);
    d->customWhiteBalanceLabel   = new QLabel(i18nc("@label:slider", "T(K):"), d->whiteBalanceSettings);
    d->customWhiteBalanceSpinBox->setToolTip( i18nc("@info:whatsthis", "<title>Temperature</title>"
                                "<p>Set here the color temperature in Kelvin.</p>"));

    d->customWhiteBalanceGreenSpinBox = new RDoubleNumInput(d->whiteBalanceSettings);
    d->customWhiteBalanceGreenSpinBox->setDecimals(2);
    d->customWhiteBalanceGreenSpinBox->setRange(0.2, 2.5, 0.01);
    d->customWhiteBalanceGreenSpinBox->setDefaultValue(1.0);
    d->customWhiteBalanceGreenLabel   = new QLabel(i18nc("@label:slider Green component", "Green:"), d->whiteBalanceSettings);
    d->customWhiteBalanceGreenSpinBox->setToolTip(i18nc("@info:whatsthis", "<p>Set here the "
                                "green component to set magenta color cast removal level.</p>"));

    d->unclipColorLabel    = new QLabel(i18nc("@label:listbox", "Highlights:"), d->whiteBalanceSettings);
    d->unclipColorComboBox = new RComboBox(d->whiteBalanceSettings);
    d->unclipColorComboBox->insertItem(0, i18nc("@item:inlistbox", "Solid white"));
    d->unclipColorComboBox->insertItem(1, i18nc("@item:inlistbox", "Unclip"));
    d->unclipColorComboBox->insertItem(2, i18nc("@item:inlistbox", "Blend"));
    d->unclipColorComboBox->insertItem(3, i18nc("@item:inlistbox", "Rebuild"));
    d->unclipColorComboBox->setDefaultIndex(0);
    d->unclipColorComboBox->setToolTip(i18nc("@info:whatsthis", "<title>Highlights</title>"
                                "<p>Select here the highlight clipping method:</p>"
                                "<p><ul><li><emphasis strong='true'>Solid white</emphasis>: "
                                "clip all highlights to solid white</li>"
                                "<li><emphasis strong='true'>Unclip</emphasis>: leave highlights "
                                "unclipped in various shades of pink</li>"
                                "<li><emphasis strong='true'>Blend</emphasis>:Blend clipped and "
                                "unclipped values together for a gradual fade to white</li>"
                                "<li><emphasis strong='true'>Rebuild</emphasis>: reconstruct "
                                "highlights using a level value</li></ul></p>"));

    d->reconstructLabel   = new QLabel(i18nc("@label:slider Highlight reconstruct level", "Level:"), d->whiteBalanceSettings);
    d->reconstructSpinBox = new RIntNumInput(d->whiteBalanceSettings);
    d->reconstructSpinBox->setRange(0, 6, 1);
    d->reconstructSpinBox->setDefaultValue(0);
    d->reconstructSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Level</title>"
                                "<p>Specify the reconstruct highlight level. Low values favor "
                                "whites and high values favor colors.</p>"));

    d->expoCorrectionBox = new QCheckBox(i18nc("@option:check", "Exposure Correction (E.V)"), d->whiteBalanceSettings);
    d->expoCorrectionBox->setToolTip(i18nc("@info:whatsthis", "<p>Turn on the exposure "
                                "correction before interpolation.</p>"));

    d->expoCorrectionShiftLabel   = new QLabel(i18nc("@label:slider", "Linear Shift:"), d->whiteBalanceSettings);
    d->expoCorrectionShiftSpinBox = new RDoubleNumInput(d->whiteBalanceSettings);
    d->expoCorrectionShiftSpinBox->setDecimals(2);
    d->expoCorrectionShiftSpinBox->setRange(-2.0, 3.0, 0.01);
    d->expoCorrectionShiftSpinBox->setDefaultValue(0.0);
    d->expoCorrectionShiftSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Shift</title>"
                                "<p>Linear Shift of exposure correction before interpolation in E.V</p>"));

    d->expoCorrectionHighlightLabel   = new QLabel(i18nc("@label:slider", "Highlight:"), d->whiteBalanceSettings);
    d->expoCorrectionHighlightSpinBox = new RDoubleNumInput(d->whiteBalanceSettings);
    d->expoCorrectionHighlightSpinBox->setDecimals(2);
    d->expoCorrectionHighlightSpinBox->setRange(0.0, 1.0, 0.01);
    d->expoCorrectionHighlightSpinBox->setDefaultValue(0.0);
    d->expoCorrectionHighlightSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Highlight</title>"
                                "<p>Amount of highlight preservation for exposure correction "
                                "before interpolation in E.V. Only take effect if Shift Correction is > 1.0 E.V</p>"));

    d->fixColorsHighlightsBox = new QCheckBox(i18nc("@option:check", "Correct false colors in highlights"), d->whiteBalanceSettings);
    d->fixColorsHighlightsBox->setToolTip(i18nc("@info:whatsthis", "<p>If enabled, images with "
                                "overblown channels are processed much more accurately, without "
                                "'pink clouds' (and blue highlights under tungsten lamps).</p>"));

    d->autoBrightnessBox = new QCheckBox(i18nc("@option:check", "Auto Brightness"), d->whiteBalanceSettings);
    d->autoBrightnessBox->setToolTip(i18nc("@info:whatsthis", "<p>If disable, use a fixed white level "
                                "and ignore the image histogram to adjust brightness.</p>"));

    d->brightnessLabel   = new QLabel(i18nc("@label:slider", "Brightness:"), d->whiteBalanceSettings);
    d->brightnessSpinBox = new RDoubleNumInput(d->whiteBalanceSettings);
    d->brightnessSpinBox->setDecimals(2);
    d->brightnessSpinBox->setRange(0.0, 10.0, 0.01);
    d->brightnessSpinBox->setDefaultValue(1.0);
    d->brightnessSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Brightness</title>"
                                "<p>Specify the brightness level of output image. The default "
                                "value is 1.0 (works in 8-bit mode only).</p>"));

    if (! (advSettings & POSTPROCESSING))
    {
        d->brightnessLabel->hide();
        d->brightnessSpinBox->hide();
    }

    d->blackPointCheckBox = new QCheckBox(i18nc("@option:check Black point", "Black:"), d->whiteBalanceSettings);
    d->blackPointCheckBox->setToolTip(i18nc("@info:whatsthis", "<title>Black point</title>"
                                "<p>Use a specific black point value to decode RAW pictures. If "
                                "you set this option to off, the Black Point value will be "
                                "automatically computed.</p>"));
    d->blackPointSpinBox = new RIntNumInput(d->whiteBalanceSettings);
    d->blackPointSpinBox->setRange(0, 1000, 1);
    d->blackPointSpinBox->setDefaultValue(0);
    d->blackPointSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Black point value</title>"
                                "<p>Specify specific black point value of the output image.</p>"));

    d->whitePointCheckBox = new QCheckBox(i18nc("@option:check White point", "White:"), d->whiteBalanceSettings);
    d->whitePointCheckBox->setToolTip(i18nc("@info:whatsthis", "<title>White point</title>"
                                "<p>Use a specific white point value to decode RAW pictures. If "
                                "you set this option to off, the White Point value will be "
                                "automatically computed.</p>"));
    d->whitePointSpinBox = new RIntNumInput(d->whiteBalanceSettings);
    d->whitePointSpinBox->setRange(0, 20000, 1);
    d->whitePointSpinBox->setDefaultValue(0);
    d->whitePointSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>White point value</title>"
                                "<p>Specify specific white point value of the output image.</p>"));

    if (! (advSettings & BLACKWHITEPOINTS))
    {
        d->blackPointCheckBox->hide();
        d->blackPointSpinBox->hide();
        d->whitePointCheckBox->hide();
        d->whitePointSpinBox->hide();
    }

    whiteBalanceLayout->addWidget(d->whiteBalanceLabel,              0,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->whiteBalanceComboBox,           0,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceLabel,        1,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceSpinBox,      1,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceGreenLabel,   2,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->customWhiteBalanceGreenSpinBox, 2,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->unclipColorLabel,               3,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->unclipColorComboBox,            3,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->reconstructLabel,               4,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->reconstructSpinBox,             4,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->expoCorrectionBox,              5,  0, 1, 2);
    whiteBalanceLayout->addWidget(d->expoCorrectionShiftLabel,       6,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->expoCorrectionShiftSpinBox,     6,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->expoCorrectionHighlightLabel,   7,  0, 1, 1);
    whiteBalanceLayout->addWidget(d->expoCorrectionHighlightSpinBox, 7,  1, 1, 2);
    whiteBalanceLayout->addWidget(d->fixColorsHighlightsBox,         8,  0, 1, 3);
    whiteBalanceLayout->addWidget(d->autoBrightnessBox,              9,  0, 1, 3);
    whiteBalanceLayout->addWidget(d->brightnessLabel,                10, 0, 1, 1);
    whiteBalanceLayout->addWidget(d->brightnessSpinBox,              10, 1, 1, 2);
    whiteBalanceLayout->addWidget(d->blackPointCheckBox,             11, 0, 1, 1);
    whiteBalanceLayout->addWidget(d->blackPointSpinBox,              11, 1, 1, 2);
    whiteBalanceLayout->addWidget(d->whitePointCheckBox,             12, 0, 1, 1);
    whiteBalanceLayout->addWidget(d->whitePointSpinBox,              12, 1, 1, 2);
    whiteBalanceLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    whiteBalanceLayout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    addItem(d->whiteBalanceSettings, KisIconUtils::loadIcon("kdcraw").pixmap(16, 16), i18nc("@label", "White Balance"), QString("whitebalance"), true);

    // ---------------------------------------------------------------
    // CORRECTIONS Settings panel

    d->correctionsSettings               = new QWidget(this);
    QGridLayout* const correctionsLayout = new QGridLayout(d->correctionsSettings);

    d->noiseReductionLabel    = new QLabel(i18nc("@label:listbox", "Noise reduction:"), d->correctionsSettings);
    d->noiseReductionComboBox = new RComboBox(d->colormanSettings);
    d->noiseReductionComboBox->insertItem(RawDecodingSettings::NONR,       i18nc("@item:inlistbox Noise Reduction", "None"));
    d->noiseReductionComboBox->insertItem(RawDecodingSettings::WAVELETSNR, i18nc("@item:inlistbox Noise Reduction", "Wavelets"));
    d->noiseReductionComboBox->insertItem(RawDecodingSettings::FBDDNR,     i18nc("@item:inlistbox Noise Reduction", "FBDD"));
    d->noiseReductionComboBox->insertItem(RawDecodingSettings::LINENR,     i18nc("@item:inlistbox Noise Reduction", "CFA Line Denoise"));
    d->noiseReductionComboBox->insertItem(RawDecodingSettings::IMPULSENR,  i18nc("@item:inlistbox Noise Reduction", "Impulse Denoise"));
    d->noiseReductionComboBox->setDefaultIndex(RawDecodingSettings::NONR);
    d->noiseReductionComboBox->setToolTip(i18nc("@info:whatsthis", "<title>Noise Reduction</title>"
                                "<p>Select here the noise reduction method to apply during RAW "
                                "decoding.</p>"
                                "<p><ul><li><emphasis strong='true'>None</emphasis>: no "
                                "noise reduction.</li>"
                                "<li><emphasis strong='true'>Wavelets</emphasis>: wavelets "
                                "correction to erase noise while preserving real detail. It's "
                                "applied after interpolation.</li>"
                                "<li><emphasis strong='true'>FBDD</emphasis>: Fake Before "
                                "Demosaicing Denoising noise reduction. It's applied before "
                                "interpolation.</li>"
                                "<li><emphasis strong='true'>CFA Line Denoise</emphasis>: Banding "
                                "noise suppression. It's applied after interpolation.</li>"
                                "<li><emphasis strong='true'>Impulse Denoise</emphasis>: Impulse "
                                "noise suppression. It's applied after interpolation.</li></ul></p>"));

    d->NRSpinBox1 = new RIntNumInput(d->correctionsSettings);
    d->NRSpinBox1->setRange(100, 1000, 1);
    d->NRSpinBox1->setDefaultValue(100);
    d->NRLabel1   = new QLabel(d->correctionsSettings);

    d->NRSpinBox2 = new RIntNumInput(d->correctionsSettings);
    d->NRSpinBox2->setRange(100, 1000, 1);
    d->NRSpinBox2->setDefaultValue(100);
    d->NRLabel2   = new QLabel(d->correctionsSettings);

    d->enableCACorrectionBox = new QCheckBox(i18nc("@option:check", "Enable Chromatic Aberration correction"), d->correctionsSettings);
    d->enableCACorrectionBox->setToolTip(i18nc("@info:whatsthis", "<title>Enable Chromatic "
                                "Aberration correction</title>"
                                "<p>Enlarge the raw red-green and blue-yellow axis by the given "
                                "factors (automatic by default).</p>"));

    d->autoCACorrectionBox = new QCheckBox(i18nc("@option:check", "Automatic color axis adjustments"), d->correctionsSettings);
    d->autoCACorrectionBox->setToolTip(i18nc("@info:whatsthis", "<title>Automatic Chromatic "
                                "Aberration correction</title>"
                                "<p>If this option is turned on, it will try to shift image "
                                "channels slightly and evaluate Chromatic Aberration change. Note "
                                "that if you shot blue-red pattern, the method may fail. In this "
                                "case, disable this option and tune manually color factors.</p>"));

    d->caRedMultLabel   = new QLabel(i18nc("@label:slider", "Red-Green:"), d->correctionsSettings);
    d->caRedMultSpinBox = new RDoubleNumInput(d->correctionsSettings);
    d->caRedMultSpinBox->setDecimals(1);
    d->caRedMultSpinBox->setRange(-4.0, 4.0, 0.1);
    d->caRedMultSpinBox->setDefaultValue(0.0);
    d->caRedMultSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Red-Green multiplier</title>"
                                "<p>Set here the amount of correction on red-green axis</p>"));

    d->caBlueMultLabel   = new QLabel(i18nc("@label:slider", "Blue-Yellow:"), d->correctionsSettings);
    d->caBlueMultSpinBox = new RDoubleNumInput(d->correctionsSettings);
    d->caBlueMultSpinBox->setDecimals(1);
    d->caBlueMultSpinBox->setRange(-4.0, 4.0, 0.1);
    d->caBlueMultSpinBox->setDefaultValue(0.0);
    d->caBlueMultSpinBox->setToolTip(i18nc("@info:whatsthis", "<title>Blue-Yellow multiplier</title>"
                                "<p>Set here the amount of correction on blue-yellow axis</p>"));

    correctionsLayout->addWidget(d->noiseReductionLabel,    0, 0, 1, 1);
    correctionsLayout->addWidget(d->noiseReductionComboBox, 0, 1, 1, 2);
    correctionsLayout->addWidget(d->NRLabel1,               1, 0, 1, 1);
    correctionsLayout->addWidget(d->NRSpinBox1,             1, 1, 1, 2);
    correctionsLayout->addWidget(d->NRLabel2,               2, 0, 1, 1);
    correctionsLayout->addWidget(d->NRSpinBox2,             2, 1, 1, 2);
    correctionsLayout->addWidget(d->enableCACorrectionBox,  3, 0, 1, 3);
    correctionsLayout->addWidget(d->autoCACorrectionBox,    4, 0, 1, 3);
    correctionsLayout->addWidget(d->caRedMultLabel,         5, 0, 1, 1);
    correctionsLayout->addWidget(d->caRedMultSpinBox,       5, 1, 1, 2);
    correctionsLayout->addWidget(d->caBlueMultLabel,        6, 0, 1, 1);
    correctionsLayout->addWidget(d->caBlueMultSpinBox,      6, 1, 1, 2);
    correctionsLayout->setRowStretch(7, 10);
    correctionsLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    correctionsLayout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    addItem(d->correctionsSettings, KisIconUtils::loadIcon("kdcraw").pixmap(16, 16), i18nc("@label", "Corrections"), QString("corrections"), false);

    // ---------------------------------------------------------------
    // COLOR MANAGEMENT Settings panel

    d->colormanSettings               = new QWidget(this);
    QGridLayout* const colormanLayout = new QGridLayout(d->colormanSettings);

    d->inputColorSpaceLabel     = new QLabel(i18nc("@label:listbox", "Camera Profile:"), d->colormanSettings);
    d->inputColorSpaceComboBox  = new RComboBox(d->colormanSettings);
    d->inputColorSpaceComboBox->insertItem(RawDecodingSettings::NOINPUTCS,     i18nc("@item:inlistbox Camera Profile", "None"));
    d->inputColorSpaceComboBox->insertItem(RawDecodingSettings::EMBEDDED,      i18nc("@item:inlistbox Camera Profile", "Embedded"));
    d->inputColorSpaceComboBox->insertItem(RawDecodingSettings::CUSTOMINPUTCS, i18nc("@item:inlistbox Camera Profile", "Custom"));
    d->inputColorSpaceComboBox->setDefaultIndex(RawDecodingSettings::NOINPUTCS);
    d->inputColorSpaceComboBox->setToolTip(i18nc("@info:whatsthis", "<title>Camera Profile</title>"
                                "<p>Select here the input color space used to decode RAW data.</p>"
                                "<p><ul><li><emphasis strong='true'>None</emphasis>: no "
                                "input color profile is used during RAW decoding.</li>"
                                "<li><emphasis strong='true'>Embedded</emphasis>: use embedded "
                                "color profile from RAW file, if it exists.</li>"
                                "<li><emphasis strong='true'>Custom</emphasis>: use a custom "
                                "input color space profile.</li></ul></p>"));

    d->inIccUrlEdit = new RFileSelector(d->colormanSettings);
    d->inIccUrlEdit->setFileDlgMode(QFileDialog::ExistingFile);
    d->inIccUrlEdit->setFileDlgFilter(i18n("ICC Files (*.icc; *.icm)"));

    d->outputColorSpaceLabel    = new QLabel(i18nc("@label:listbox", "Workspace:"), d->colormanSettings);
    d->outputColorSpaceComboBox = new RComboBox( d->colormanSettings );
    d->outputColorSpaceComboBox->insertItem(RawDecodingSettings::RAWCOLOR,       i18nc("@item:inlistbox Workspace", "Raw (no profile)"));
    d->outputColorSpaceComboBox->insertItem(RawDecodingSettings::SRGB,           i18nc("@item:inlistbox Workspace", "sRGB"));
    d->outputColorSpaceComboBox->insertItem(RawDecodingSettings::ADOBERGB,       i18nc("@item:inlistbox Workspace", "Adobe RGB"));
    d->outputColorSpaceComboBox->insertItem(RawDecodingSettings::WIDEGAMMUT,     i18nc("@item:inlistbox Workspace", "Wide Gamut"));
    d->outputColorSpaceComboBox->insertItem(RawDecodingSettings::PROPHOTO,       i18nc("@item:inlistbox Workspace", "Pro-Photo"));
    d->outputColorSpaceComboBox->insertItem(RawDecodingSettings::CUSTOMOUTPUTCS, i18nc("@item:inlistbox Workspace", "Custom"));
    d->outputColorSpaceComboBox->setDefaultIndex(RawDecodingSettings::SRGB);
    d->outputColorSpaceComboBox->setToolTip(i18nc("@info:whatsthis", "<title>Workspace</title>"
                                "<p>Select here the output color space used to decode RAW data.</p>"
                                "<p><ul><li><emphasis strong='true'>Raw (no profile)</emphasis>: "
                                "in this mode, no output color space is used during RAW decoding.</li>"
                                "<li><emphasis strong='true'>sRGB</emphasis>: this is an RGB "
                                "color space, created cooperatively by Hewlett-Packard and "
                                "Microsoft. It is the best choice for images destined for the Web "
                                "and portrait photography.</li>"
                                "<li><emphasis strong='true'>Adobe RGB</emphasis>: this color "
                                "space is an extended RGB color space, developed by Adobe. It is "
                                "used for photography applications such as advertising and fine "
                                "art.</li>"
                                "<li><emphasis strong='true'>Wide Gamut</emphasis>: this color "
                                "space is an expanded version of the Adobe RGB color space.</li>"
                                "<li><emphasis strong='true'>Pro-Photo</emphasis>: this color "
                                "space is an RGB color space, developed by Kodak, that offers an "
                                "especially large gamut designed for use with photographic outputs "
                                "in mind.</li>"
                                "<li><emphasis strong='true'>Custom</emphasis>: use a custom "
                                "output color space profile.</li></ul></p>"));

    d->outIccUrlEdit = new RFileSelector(d->colormanSettings);
    d->outIccUrlEdit->setFileDlgMode(QFileDialog::ExistingFile);
    d->outIccUrlEdit->setFileDlgFilter(i18n("ICC Files (*.icc; *.icm)"));

    colormanLayout->addWidget(d->inputColorSpaceLabel,     0, 0, 1, 1);
    colormanLayout->addWidget(d->inputColorSpaceComboBox,  0, 1, 1, 2);
    colormanLayout->addWidget(d->inIccUrlEdit,             1, 0, 1, 3);
    colormanLayout->addWidget(d->outputColorSpaceLabel,    2, 0, 1, 1);
    colormanLayout->addWidget(d->outputColorSpaceComboBox, 2, 1, 1, 2);
    colormanLayout->addWidget(d->outIccUrlEdit,            3, 0, 1, 3);
    colormanLayout->setRowStretch(4, 10);
    colormanLayout->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    colormanLayout->setMargin(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    addItem(d->colormanSettings, KisIconUtils::loadIcon("kdcraw").pixmap(16, 16), i18nc("@label", "Color Management"), QString("colormanagement"), false);

    if (! (advSettings & COLORSPACE))
        removeItem(COLORMANAGEMENT);

    addStretch();

    // ---------------------------------------------------------------

    connect(d->unclipColorComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::slotUnclipColorActivated);

    connect(d->whiteBalanceComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::slotWhiteBalanceToggled);

    connect(d->noiseReductionComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::slotNoiseReductionChanged);

    connect(d->enableCACorrectionBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::slotCACorrectionToggled);

    connect(d->autoCACorrectionBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::slotAutoCAToggled);

    connect(d->blackPointCheckBox, SIGNAL(toggled(bool)),
            d->blackPointSpinBox, SLOT(setEnabled(bool)));

    connect(d->whitePointCheckBox, SIGNAL(toggled(bool)),
            d->whitePointSpinBox, SLOT(setEnabled(bool)));

    connect(d->sixteenBitsImage, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::slotsixteenBitsImageToggled);

    connect(d->inputColorSpaceComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::slotInputColorSpaceChanged);

    connect(d->outputColorSpaceComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::slotOutputColorSpaceChanged);

    connect(d->expoCorrectionBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::slotExposureCorrectionToggled);

    connect(d->expoCorrectionShiftSpinBox, &RDoubleNumInput::valueChanged,
            this, &DcrawSettingsWidget::slotExpoCorrectionShiftChanged);

    // Wrapper to emit signal when something is changed in settings.

    connect(d->inIccUrlEdit->lineEdit(), &QLineEdit::textChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->outIccUrlEdit->lineEdit(), &QLineEdit::textChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->whiteBalanceComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->RAWQualityComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::slotRAWQualityChanged);

    connect(d->unclipColorComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->inputColorSpaceComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->outputColorSpaceComboBox, static_cast<void (RComboBox::*)(int)>(&RComboBox::activated),
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->blackPointCheckBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->whitePointCheckBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->sixteenBitsImage, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->fixColorsHighlightsBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->autoBrightnessBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->fourColorCheckBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->dontStretchPixelsCheckBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->refineInterpolationBox, &QCheckBox::toggled,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->customWhiteBalanceSpinBox, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->reconstructSpinBox, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->blackPointSpinBox, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->whitePointSpinBox, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->NRSpinBox1, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->NRSpinBox2, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->medianFilterPassesSpinBox, &RIntNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->customWhiteBalanceGreenSpinBox, &RDoubleNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->caRedMultSpinBox, &RDoubleNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->caBlueMultSpinBox, &RDoubleNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->brightnessSpinBox, &RDoubleNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);

    connect(d->expoCorrectionHighlightSpinBox, &RDoubleNumInput::valueChanged,
            this, &DcrawSettingsWidget::signalSettingsChanged);
}

DcrawSettingsWidget::~DcrawSettingsWidget()
{
    delete d;
}

void DcrawSettingsWidget::updateMinimumWidth()
{
    int width = 0;

    for (int i = 0; i < count(); i++)
    {
        if (widget(i)->width() > width)
            width = widget(i)->width();
    }

    setMinimumWidth(width);
}

RFileSelector* DcrawSettingsWidget::inputProfileUrlEdit() const
{
    return d->inIccUrlEdit;
}

RFileSelector* DcrawSettingsWidget::outputProfileUrlEdit() const
{
    return d->outIccUrlEdit;
}

void DcrawSettingsWidget::resetToDefault()
{
    setSettings(RawDecodingSettings());
}

void DcrawSettingsWidget::slotsixteenBitsImageToggled(bool b)
{
    setEnabledBrightnessSettings(!b);
    emit signalSixteenBitsImageToggled(d->sixteenBitsImage->isChecked());
}

void DcrawSettingsWidget::slotWhiteBalanceToggled(int v)
{
    if (v == 3)
    {
        d->customWhiteBalanceSpinBox->setEnabled(true);
        d->customWhiteBalanceGreenSpinBox->setEnabled(true);
        d->customWhiteBalanceLabel->setEnabled(true);
        d->customWhiteBalanceGreenLabel->setEnabled(true);
    }
    else
    {
        d->customWhiteBalanceSpinBox->setEnabled(false);
        d->customWhiteBalanceGreenSpinBox->setEnabled(false);
        d->customWhiteBalanceLabel->setEnabled(false);
        d->customWhiteBalanceGreenLabel->setEnabled(false);
    }
}

void DcrawSettingsWidget::slotUnclipColorActivated(int v)
{
    if (v == 3)     // Reconstruct Highlight method
    {
        d->reconstructLabel->setEnabled(true);
        d->reconstructSpinBox->setEnabled(true);
    }
    else
    {
        d->reconstructLabel->setEnabled(false);
        d->reconstructSpinBox->setEnabled(false);
    }
}

void DcrawSettingsWidget::slotNoiseReductionChanged(int item)
{
    d->NRSpinBox1->setEnabled(true);
    d->NRLabel1->setEnabled(true);
    d->NRSpinBox2->setEnabled(true);
    d->NRLabel2->setEnabled(true);
    d->NRLabel1->setText(i18nc("@label", "Threshold:"));
    d->NRSpinBox1->setToolTip(i18nc("@info:whatsthis", "<title>Threshold</title>"
                                "<p>Set here the noise reduction threshold value to use.</p>"));

    switch(item)
    {
        case RawDecodingSettings::WAVELETSNR:
        case RawDecodingSettings::FBDDNR:
        case RawDecodingSettings::LINENR:
            d->NRSpinBox2->setVisible(false);
            d->NRLabel2->setVisible(false);
            break;

        case RawDecodingSettings::IMPULSENR:
            d->NRLabel1->setText(i18nc("@label", "Luminance:"));
            d->NRSpinBox1->setToolTip(i18nc("@info:whatsthis", "<title>Luminance</title>"
                                "<p>Amount of Luminance impulse noise reduction.</p>"));
            d->NRLabel2->setText(i18nc("@label", "Chrominance:"));
            d->NRSpinBox2->setToolTip(i18nc("@info:whatsthis", "<title>Chrominance</title>"
                                "<p>Amount of Chrominance impulse noise reduction.</p>"));
            d->NRSpinBox2->setVisible(true);
            d->NRLabel2->setVisible(true);
            break;

        default:
            d->NRSpinBox1->setEnabled(false);
            d->NRLabel1->setEnabled(false);
            d->NRSpinBox2->setEnabled(false);
            d->NRLabel2->setEnabled(false);
            d->NRSpinBox2->setVisible(false);
            d->NRLabel2->setVisible(false);
            break;
    }

    emit signalSettingsChanged();
}

void DcrawSettingsWidget::slotCACorrectionToggled(bool b)
{
    d->autoCACorrectionBox->setEnabled(b);
    slotAutoCAToggled(d->autoCACorrectionBox->isChecked());
}

void DcrawSettingsWidget::slotAutoCAToggled(bool b)
{
    if (b)
    {
        d->caRedMultSpinBox->setValue(0.0);
        d->caBlueMultSpinBox->setValue(0.0);
    }

    bool mult = (!b) && (d->autoCACorrectionBox->isEnabled());
    d->caRedMultSpinBox->setEnabled(mult);
    d->caBlueMultSpinBox->setEnabled(mult);
    d->caRedMultLabel->setEnabled(mult);
    d->caBlueMultLabel->setEnabled(mult);
    emit signalSettingsChanged();
}

void DcrawSettingsWidget::slotExposureCorrectionToggled(bool b)
{
    d->expoCorrectionShiftLabel->setEnabled(b);
    d->expoCorrectionShiftSpinBox->setEnabled(b);
    d->expoCorrectionHighlightLabel->setEnabled(b);
    d->expoCorrectionHighlightSpinBox->setEnabled(b);

    slotExpoCorrectionShiftChanged(d->expoCorrectionShiftSpinBox->value());
}

void DcrawSettingsWidget::slotExpoCorrectionShiftChanged(double ev)
{
    // Only enable Highligh exposure correction if Shift correction is >= 1.0, else this settings do not take effect.
    bool b = (ev >= 1.0);

    d->expoCorrectionHighlightLabel->setEnabled(b);
    d->expoCorrectionHighlightSpinBox->setEnabled(b);

    emit signalSettingsChanged();
}

void DcrawSettingsWidget::slotInputColorSpaceChanged(int item)
{
    d->inIccUrlEdit->setEnabled(item == RawDecodingSettings::CUSTOMINPUTCS);
}

void DcrawSettingsWidget::slotOutputColorSpaceChanged(int item)
{
    d->outIccUrlEdit->setEnabled(item == RawDecodingSettings::CUSTOMOUTPUTCS);
}

void DcrawSettingsWidget::slotRAWQualityChanged(int quality)
{
    switch(quality)
    {
        case RawDecodingSettings::DCB:
        case RawDecodingSettings::VCD_AHD:
            // These options can be only available if Libraw use GPL2 pack.
            d->medianFilterPassesLabel->setEnabled(KDcraw::librawUseGPL2DemosaicPack());
            d->medianFilterPassesSpinBox->setEnabled(KDcraw::librawUseGPL2DemosaicPack());
            d->refineInterpolationBox->setEnabled(KDcraw::librawUseGPL2DemosaicPack());
            break;

        case RawDecodingSettings::PL_AHD:
        case RawDecodingSettings::AFD:
        case RawDecodingSettings::VCD:
        case RawDecodingSettings::LMMSE:
        case RawDecodingSettings::AMAZE:
            d->medianFilterPassesLabel->setEnabled(false);
            d->medianFilterPassesSpinBox->setEnabled(false);
            d->refineInterpolationBox->setEnabled(false);
            break;

        default: // BILINEAR, VNG, PPG, AHD
            d->medianFilterPassesLabel->setEnabled(true);
            d->medianFilterPassesSpinBox->setEnabled(true);
            d->refineInterpolationBox->setEnabled(false);
            break;
    }

    emit signalSettingsChanged();
}

void DcrawSettingsWidget::setEnabledBrightnessSettings(bool b)
{
    d->brightnessLabel->setEnabled(b);
    d->brightnessSpinBox->setEnabled(b);
}

bool DcrawSettingsWidget::brightnessSettingsIsEnabled() const
{
    return d->brightnessSpinBox->isEnabled();
}

void DcrawSettingsWidget::setSettings(const RawDecodingSettings& settings)
{
    d->sixteenBitsImage->setChecked(settings.sixteenBitsImage);

    switch(settings.whiteBalance)
    {
        case RawDecodingSettings::CAMERA:
            d->whiteBalanceComboBox->setCurrentIndex(1);
            break;
        case RawDecodingSettings::AUTO:
            d->whiteBalanceComboBox->setCurrentIndex(2);
            break;
        case RawDecodingSettings::CUSTOM:
            d->whiteBalanceComboBox->setCurrentIndex(3);
            break;
        default:
            d->whiteBalanceComboBox->setCurrentIndex(0);
            break;
    }
    slotWhiteBalanceToggled(d->whiteBalanceComboBox->currentIndex());

    d->customWhiteBalanceSpinBox->setValue(settings.customWhiteBalance);
    d->customWhiteBalanceGreenSpinBox->setValue(settings.customWhiteBalanceGreen);
    d->fourColorCheckBox->setChecked(settings.RGBInterpolate4Colors);
    d->autoBrightnessBox->setChecked(settings.autoBrightness);
    d->fixColorsHighlightsBox->setChecked(settings.fixColorsHighlights);

    switch(settings.unclipColors)
    {
        case 0:
            d->unclipColorComboBox->setCurrentIndex(0);
            break;
        case 1:
            d->unclipColorComboBox->setCurrentIndex(1);
            break;
        case 2:
            d->unclipColorComboBox->setCurrentIndex(2);
            break;
        default:         // Reconstruct Highlight method
            d->unclipColorComboBox->setCurrentIndex(3);
            d->reconstructSpinBox->setValue(settings.unclipColors-3);
            break;
    }
    slotUnclipColorActivated(d->unclipColorComboBox->currentIndex());

    d->dontStretchPixelsCheckBox->setChecked(settings.DontStretchPixels);
    d->brightnessSpinBox->setValue(settings.brightness);
    d->blackPointCheckBox->setChecked(settings.enableBlackPoint);
    d->blackPointSpinBox->setEnabled(settings.enableBlackPoint);
    d->blackPointSpinBox->setValue(settings.blackPoint);
    d->whitePointCheckBox->setChecked(settings.enableWhitePoint);
    d->whitePointSpinBox->setEnabled(settings.enableWhitePoint);
    d->whitePointSpinBox->setValue(settings.whitePoint);

    int q = settings.RAWQuality;

    // If Libraw do not support GPL2 pack, reset to BILINEAR.
    if (!KDcraw::librawUseGPL2DemosaicPack())
    {
        for (int i=RawDecodingSettings::DCB ; i <=RawDecodingSettings::LMMSE ; ++i)
        {
            if (q == i)
            {
                q = RawDecodingSettings::BILINEAR;
                qCDebug(LIBKDCRAW_LOG) << "Libraw GPL2 pack not available. Raw quality set to Bilinear";
                break;
            }
        }
    }

    // If Libraw do not support GPL3 pack, reset to BILINEAR.
    if (!KDcraw::librawUseGPL3DemosaicPack() && (q == RawDecodingSettings::AMAZE))
    {
        q = RawDecodingSettings::BILINEAR;
        qCDebug(LIBKDCRAW_LOG) << "Libraw GPL3 pack not available. Raw quality set to Bilinear";
    }

    d->RAWQualityComboBox->setCurrentIndex(q);

    switch(q)
    {
        case RawDecodingSettings::DCB:
            d->medianFilterPassesSpinBox->setValue(settings.dcbIterations);
            d->refineInterpolationBox->setChecked(settings.dcbEnhanceFl);
            break;
        case RawDecodingSettings::VCD_AHD:
            d->medianFilterPassesSpinBox->setValue(settings.eeciRefine);
            d->refineInterpolationBox->setChecked(settings.eeciRefine);
            break;
        default:
            d->medianFilterPassesSpinBox->setValue(settings.medianFilterPasses);
            d->refineInterpolationBox->setChecked(false); // option not used.
            break;
    }

    slotRAWQualityChanged(q);

    d->inputColorSpaceComboBox->setCurrentIndex((int)settings.inputColorSpace);
    slotInputColorSpaceChanged((int)settings.inputColorSpace);
    d->outputColorSpaceComboBox->setCurrentIndex((int)settings.outputColorSpace);
    slotOutputColorSpaceChanged((int)settings.outputColorSpace);

    d->noiseReductionComboBox->setCurrentIndex(settings.NRType);
    slotNoiseReductionChanged(settings.NRType);
    d->NRSpinBox1->setValue(settings.NRThreshold);
    d->NRSpinBox2->setValue(settings.NRChroThreshold);

    d->enableCACorrectionBox->setChecked(settings.enableCACorrection);
    d->caRedMultSpinBox->setValue(settings.caMultiplier[0]);
    d->caBlueMultSpinBox->setValue(settings.caMultiplier[1]);
    d->autoCACorrectionBox->setChecked((settings.caMultiplier[0] == 0.0) && (settings.caMultiplier[1] == 0.0));
    slotCACorrectionToggled(settings.enableCACorrection);

    d->expoCorrectionBox->setChecked(settings.expoCorrection);
    slotExposureCorrectionToggled(settings.expoCorrection);
    d->expoCorrectionShiftSpinBox->setValue(d->shiftExpoFromLinearToEv(settings.expoCorrectionShift));
    d->expoCorrectionHighlightSpinBox->setValue(settings.expoCorrectionHighlight);

    d->inIccUrlEdit->lineEdit()->setText(settings.inputProfile);
    d->outIccUrlEdit->lineEdit()->setText(settings.outputProfile);
}

RawDecodingSettings DcrawSettingsWidget::settings() const
{
    RawDecodingSettings prm;
    prm.sixteenBitsImage = d->sixteenBitsImage->isChecked();

    switch(d->whiteBalanceComboBox->currentIndex())
    {
        case 1:
            prm.whiteBalance = RawDecodingSettings::CAMERA;
            break;
        case 2:
            prm.whiteBalance = RawDecodingSettings::AUTO;
            break;
        case 3:
            prm.whiteBalance = RawDecodingSettings::CUSTOM;
            break;
        default:
            prm.whiteBalance = RawDecodingSettings::NONE;
            break;
    }

    prm.customWhiteBalance      = d->customWhiteBalanceSpinBox->value();
    prm.customWhiteBalanceGreen = d->customWhiteBalanceGreenSpinBox->value();
    prm.RGBInterpolate4Colors   = d->fourColorCheckBox->isChecked();
    prm.autoBrightness          = d->autoBrightnessBox->isChecked();
    prm.fixColorsHighlights     = d->fixColorsHighlightsBox->isChecked();

    switch(d->unclipColorComboBox->currentIndex())
    {
        case 0:
            prm.unclipColors = 0;
            break;
        case 1:
            prm.unclipColors = 1;
            break;
        case 2:
            prm.unclipColors = 2;
            break;
        default:         // Reconstruct Highlight method
            prm.unclipColors =  d->reconstructSpinBox->value()+3;
            break;
    }

    prm.DontStretchPixels    = d->dontStretchPixelsCheckBox->isChecked();
    prm.brightness           = d->brightnessSpinBox->value();
    prm.enableBlackPoint     = d->blackPointCheckBox->isChecked();
    prm.blackPoint           = d->blackPointSpinBox->value();
    prm.enableWhitePoint     = d->whitePointCheckBox->isChecked();
    prm.whitePoint           = d->whitePointSpinBox->value();

    prm.RAWQuality           = (RawDecodingSettings::DecodingQuality)d->RAWQualityComboBox->currentIndex();
    switch(prm.RAWQuality)
    {
        case RawDecodingSettings::DCB:
            prm.dcbIterations      = d->medianFilterPassesSpinBox->value();
            prm.dcbEnhanceFl       = d->refineInterpolationBox->isChecked();
            break;
        case RawDecodingSettings::VCD_AHD:
            prm.esMedPasses        = d->medianFilterPassesSpinBox->value();
            prm.eeciRefine         = d->refineInterpolationBox->isChecked();
            break;
        default:
            prm.medianFilterPasses = d->medianFilterPassesSpinBox->value();
            break;
    }

    prm.NRType = (RawDecodingSettings::NoiseReduction)d->noiseReductionComboBox->currentIndex();
    switch (prm.NRType)
    {
        case RawDecodingSettings::NONR:
        {
            prm.NRThreshold     = 0;
            prm.NRChroThreshold = 0;
            break;
        }
        case RawDecodingSettings::WAVELETSNR:
        case RawDecodingSettings::FBDDNR:
        case RawDecodingSettings::LINENR:
        {
            prm.NRThreshold     = d->NRSpinBox1->value();
            prm.NRChroThreshold = 0;
            break;
        }
        default:    // IMPULSENR
        {
            prm.NRThreshold     = d->NRSpinBox1->value();
            prm.NRChroThreshold = d->NRSpinBox2->value();
            break;
        }
    }

    prm.enableCACorrection      = d->enableCACorrectionBox->isChecked();
    prm.caMultiplier[0]         = d->caRedMultSpinBox->value();
    prm.caMultiplier[1]         = d->caBlueMultSpinBox->value();

    prm.expoCorrection          = d->expoCorrectionBox->isChecked();
    prm.expoCorrectionShift     = d->shiftExpoFromEvToLinear(d->expoCorrectionShiftSpinBox->value());
    prm.expoCorrectionHighlight = d->expoCorrectionHighlightSpinBox->value();

    prm.inputColorSpace         = (RawDecodingSettings::InputColorSpace)(d->inputColorSpaceComboBox->currentIndex());
    prm.outputColorSpace        = (RawDecodingSettings::OutputColorSpace)(d->outputColorSpaceComboBox->currentIndex());
    prm.inputProfile            = d->inIccUrlEdit->lineEdit()->text();
    prm.outputProfile           = d->outIccUrlEdit->lineEdit()->text();

    return prm;
}

void DcrawSettingsWidget::writeSettings(KConfigGroup& group)
{
    RawDecodingSettings prm = settings();
    prm.writeSettings(group);
    RExpanderBox::writeSettings(group);
}

void DcrawSettingsWidget::readSettings(KConfigGroup& group)
{
    RawDecodingSettings prm;
    prm.readSettings(group);
    setSettings(prm);
    RExpanderBox::readSettings(group);
}

} // NameSpace KDcrawIface
