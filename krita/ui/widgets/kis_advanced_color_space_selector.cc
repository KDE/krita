/*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
 *  Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include "kis_advanced_color_space_selector.h"

#include <KoFileDialog.h>
#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoColorModelStandardIds.h>
#include <KoID.h>

#include <KoConfig.h>
#include <KoIcon.h>

#ifdef GHNS
#include <knewstuff3/downloaddialog.h>
#include <knewstuff3/uploaddialog.h>
#endif

#include <QDesktopServices>
#include <QTextBrowser>
#include <QScrollBar>

#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kurl.h>

#include "kis_factory2.h"

#include "ui_wdgcolorspaceselectoradvanced.h"

#include <kis_debug.h>

struct KisAdvancedColorSpaceSelector::Private {
    Ui_WdgColorSpaceSelectorAdvanced* colorSpaceSelector;
    QString knsrcFile;
};

KisAdvancedColorSpaceSelector::KisAdvancedColorSpaceSelector(QWidget* parent, const QString &caption)
    : QDialog(parent)
    , d(new Private)
{
    setWindowTitle(caption);

    d->colorSpaceSelector = new Ui_WdgColorSpaceSelectorAdvanced;
    d->colorSpaceSelector->setupUi(this);

    d->colorSpaceSelector->cmbColorModels->setIDList(KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::OnlyUserVisible));
    fillCmbDepths(d->colorSpaceSelector->cmbColorModels->currentItem());

    d->colorSpaceSelector->bnDownloadProfile->setIcon(themedIcon("download"));
    d->colorSpaceSelector->bnDownloadProfile->setToolTip( i18n("Download Color Profile") );
    d->colorSpaceSelector->bnDownloadProfile->setEnabled( true );
    d->colorSpaceSelector->bnDownloadProfile->hide();

    d->colorSpaceSelector->bnUploadProfile->setIcon(themedIcon("arrow-up"));
    d->colorSpaceSelector->bnUploadProfile->setToolTip( i18n("Share Color Profile") );
    d->colorSpaceSelector->bnUploadProfile->setEnabled( false );
    d->colorSpaceSelector->bnUploadProfile->hide();

#ifdef GHNS
    d->colorSpaceSelector->bnUploadProfile->show();
    d->colorSpaceSelector->bnDownloadProfile->show();
#endif

    d->colorSpaceSelector->bnInstallProfile->setIcon(themedIcon("document-open"));
    d->colorSpaceSelector->bnInstallProfile->setToolTip( i18n("Open Color Profile") );
    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbDepths(const KoID &)));
    connect(d->colorSpaceSelector->cmbColorDepth, SIGNAL(activated(const KoID &)),
            this, SLOT(fillLstProfiles()));
    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(const KoID &)),
            this, SLOT(fillLstProfiles()));
    connect(d->colorSpaceSelector->lstProfile, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(colorSpaceChanged()));
    connect(d->colorSpaceSelector->lstProfile, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
            this, SLOT(buttonUpdate()));
    connect(this, SIGNAL(selectionChanged(bool)),
            this, SLOT(fillDescription()));
    connect(this, SIGNAL(selectionChanged(bool)), d->colorSpaceSelector->TongueWidget, SLOT(repaint()));

    connect(d->colorSpaceSelector->bnInstallProfile, SIGNAL(clicked()), this, SLOT(installProfile()));
    connect(d->colorSpaceSelector->bnDownloadProfile, SIGNAL(clicked()), this, SLOT(downloadProfile()));
    connect(d->colorSpaceSelector->bnUploadProfile, SIGNAL(clicked()), this, SLOT(uploadProfile()));

    connect(d->colorSpaceSelector->bnOK, SIGNAL(accepted()), this, SLOT(accept()));
    connect(d->colorSpaceSelector->bnOK, SIGNAL(rejected()), this, SLOT(reject()));

    d->knsrcFile = "kritaiccprofiles.knsrc";
    fillLstProfiles();
}

KisAdvancedColorSpaceSelector::~KisAdvancedColorSpaceSelector()
{
    delete d->colorSpaceSelector;
    delete d;
}

void KisAdvancedColorSpaceSelector::fillLstProfiles()
{
    d->colorSpaceSelector->lstProfile->blockSignals(true);
    QString s = KoColorSpaceRegistry::instance()->colorSpaceId(d->colorSpaceSelector->cmbColorModels->currentItem(), d->colorSpaceSelector->cmbColorDepth->currentItem());
    d->colorSpaceSelector->lstProfile->clear();

    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s);
    if (csf == 0) return;//TODO: make this give better feedback.

    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);
    QStringList profileNames;
    foreach(const KoColorProfile *profile, profileList) {
        profileNames.append(profile->name());
    }
    qSort(profileNames);
    QListWidgetItem *defaultProfile = new QListWidgetItem;
    defaultProfile->setText(csf->defaultProfile() + " " + i18nc("This is appended to the color profile which is the default for the given colorspace and bit-depth","(Default)"));
    foreach(QString stringName, profileNames) {
        if (stringName==csf->defaultProfile()) {
            d->colorSpaceSelector->lstProfile->addItem(defaultProfile);
        } else {
            d->colorSpaceSelector->lstProfile->addItem(stringName);
        }
    }
    d->colorSpaceSelector->lstProfile->setCurrentItem(defaultProfile);
    d->colorSpaceSelector->lstProfile->blockSignals(false);
    colorSpaceChanged();
}

void KisAdvancedColorSpaceSelector::fillCmbDepths(const KoID& id)
{
    KoID activeDepth = d->colorSpaceSelector->cmbColorDepth->currentItem();
    d->colorSpaceSelector->cmbColorDepth->clear();
    QList<KoID> depths = KoColorSpaceRegistry::instance()->colorDepthList(id, KoColorSpaceRegistry::OnlyUserVisible);
    QList<KoID> sortedDepths;
    if (depths.contains(Integer8BitsColorDepthID)) {
        sortedDepths << Integer8BitsColorDepthID;
    }
    if (depths.contains(Integer16BitsColorDepthID)) {
        sortedDepths << Integer16BitsColorDepthID;
    }
    if (depths.contains(Float16BitsColorDepthID)) {
        sortedDepths << Float16BitsColorDepthID;
    }
    if (depths.contains(Float32BitsColorDepthID)) {
        sortedDepths << Float32BitsColorDepthID;
    }
    if (depths.contains(Float64BitsColorDepthID)) {
        sortedDepths << Float64BitsColorDepthID;
    }

    d->colorSpaceSelector->cmbColorDepth->setIDList(sortedDepths);
    if (sortedDepths.contains(activeDepth)) {
        d->colorSpaceSelector->cmbColorDepth->setCurrent(activeDepth);
    }
}

void KisAdvancedColorSpaceSelector::fillDescription()
{
    QString notApplicable = i18nc("Not Applicable, used where there's no colorants or gamma curve found","N/A");
    QString notApplicableTooltip = i18nc("@info:tooltip","This profile has no colorants.");
    QString profileName = i18nc("Shows up instead of the name when there's no profile","No Profile Found");
    QString whatIsColorant = i18n("Colorant in d50-adapted xyY.");
    //set colorants
    QString s = KoColorSpaceRegistry::instance()->colorSpaceId(d->colorSpaceSelector->cmbColorModels->currentItem(), d->colorSpaceSelector->cmbColorDepth->currentItem());
    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s);
    if (csf == 0) return;
    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);
    if (profileList.isEmpty()==false) {
        profileName = currentColorSpace()->profile()->name();
        if (currentColorSpace()->profile()->hasColorants()){
            QVector <double> colorants = currentColorSpace()->profile()->getColorantsxyY();
            QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
            //QString text = currentColorSpace()->profile()->info() + " =" +
            d->colorSpaceSelector->lblXYZ_W->setText(nameWhitePoint(whitepoint));
            d->colorSpaceSelector->lblXYZ_W->setToolTip(QString::number(whitepoint[0], 'f', 4) + ", " + QString::number(whitepoint[1], 'f', 4) + ", " + QString::number(whitepoint[2], 'f', 4));
            d->colorSpaceSelector->lblXYZ_R->setText(QString::number(colorants[0], 'f', 4) + ", " + QString::number(colorants[1], 'f', 4) + ", " + QString::number(colorants[2], 'f', 4));
            d->colorSpaceSelector->lblXYZ_G->setText(QString::number(colorants[3], 'f', 4) + ", " + QString::number(colorants[4], 'f', 4) + ", " + QString::number(colorants[5], 'f', 4));
            d->colorSpaceSelector->lblXYZ_B->setText(QString::number(colorants[6], 'f', 4) + ", " + QString::number(colorants[7], 'f', 4) + ", " + QString::number(colorants[8], 'f', 4));
            d->colorSpaceSelector->lblXYZ_R->setToolTip(whatIsColorant);
            d->colorSpaceSelector->lblXYZ_G->setToolTip(whatIsColorant);
            d->colorSpaceSelector->lblXYZ_B->setToolTip(whatIsColorant);
        } else {
            QVector <double> whitepoint2 = currentColorSpace()->profile()->getWhitePointxyY();
            d->colorSpaceSelector->lblXYZ_W->setText(nameWhitePoint(whitepoint2));
            d->colorSpaceSelector->lblXYZ_W->setToolTip(QString::number(whitepoint2[0], 'f', 4) + ", " + QString::number(whitepoint2[1], 'f', 4) + ", " + QString::number(whitepoint2[2], 'f', 4));
            d->colorSpaceSelector->lblXYZ_R->setText(notApplicable);
            d->colorSpaceSelector->lblXYZ_R->setToolTip(notApplicableTooltip);
            d->colorSpaceSelector->lblXYZ_G->setText(notApplicable);
            d->colorSpaceSelector->lblXYZ_G->setToolTip(notApplicableTooltip);
            d->colorSpaceSelector->lblXYZ_B->setText(notApplicable);
            d->colorSpaceSelector->lblXYZ_B->setToolTip(notApplicableTooltip);
        }
    } else {
        d->colorSpaceSelector->lblXYZ_W->setText(notApplicable);
        d->colorSpaceSelector->lblXYZ_W->setToolTip(notApplicableTooltip);
        d->colorSpaceSelector->lblXYZ_R->setText(notApplicable);
        d->colorSpaceSelector->lblXYZ_R->setToolTip(notApplicableTooltip);
        d->colorSpaceSelector->lblXYZ_G->setText(notApplicable);
        d->colorSpaceSelector->lblXYZ_G->setToolTip(notApplicableTooltip);
        d->colorSpaceSelector->lblXYZ_B->setText(notApplicable);
        d->colorSpaceSelector->lblXYZ_B->setToolTip(notApplicableTooltip);
    }

    //set TRC
    QVector <double> estimatedTRC(3);
    QString estimatedGamma = i18nc("Estimated Gamma indicates how the TRC (Tone Response Curve or Tone Reproduction Curve) is bent. A Gamma of 1.0 means linear.", "<b>Estimated Gamma</b>: ");
    QString estimatedsRGB = i18nc("This is for special Gamma types that LCMS cannot differentiate between", "<b>Estimated Gamma</b>: sRGB, L* or rec709 TRC");
    QString whatissRGB = i18nc("@info:tooltip","The Tone Response Curve of this color space is either sRGB, L* or rec709 TRC.");
    QString currentModelStr = d->colorSpaceSelector->cmbColorModels->currentItem().id();

    if (profileList.isEmpty()) {
        d->colorSpaceSelector->TongueWidget->setProfileDataAvailable(false);
    }
    else if (currentModelStr == "RGBA") {
        QVector <double> colorants = currentColorSpace()->profile()->getColorantsxyY();
        QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
        d->colorSpaceSelector->TongueWidget->setRGBData(whitepoint, colorants);
        estimatedTRC = currentColorSpace()->profile()->getEstimatedTRC();
        if (estimatedTRC[0] == -1) {
            d->colorSpaceSelector->lbltrc->setToolTip(whatissRGB);
            d->colorSpaceSelector->lbltrc->setText(estimatedsRGB);
        } else {
            d->colorSpaceSelector->lbltrc->setToolTip(estimatedGamma + QString::number(estimatedTRC[0]) + "," + QString::number(estimatedTRC[1]) + "," + QString::number(estimatedTRC[2]));
            d->colorSpaceSelector->lbltrc->setText(estimatedGamma + QString::number((estimatedTRC[0] + estimatedTRC[1] + estimatedTRC[2])/3));
        }
    }
    else if (currentModelStr == "GRAYA") {
        QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
        d->colorSpaceSelector->TongueWidget->setGrayData(whitepoint);
        estimatedTRC = currentColorSpace()->profile()->getEstimatedTRC();
        if (estimatedTRC[0] == -1) {
            d->colorSpaceSelector->lbltrc->setToolTip(whatissRGB);
            d->colorSpaceSelector->lbltrc->setText(estimatedsRGB);
        } else {
            d->colorSpaceSelector->lbltrc->setToolTip(estimatedGamma + QString::number(estimatedTRC[0]));
            d->colorSpaceSelector->lbltrc->setText(estimatedGamma + QString::number(estimatedTRC[0]));
        }
    }
    else if (currentModelStr == "CMYKA") {
        QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
        d->colorSpaceSelector->TongueWidget->setCMYKData(whitepoint);
        d->colorSpaceSelector->lbltrc->setToolTip(i18nc("@info:tooltip","Estimated Gamma cannot be retrieved for CMYK."));
        d->colorSpaceSelector->lbltrc->setText(estimatedGamma + notApplicable);
    }
    else if (currentModelStr == "XYZA") {
        QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
        d->colorSpaceSelector->TongueWidget->setXYZData(whitepoint);
        d->colorSpaceSelector->lbltrc->setToolTip(i18nc("@info:tooltip","XYZ is assumed to be linear Gamma."));
        d->colorSpaceSelector->lbltrc->setText(estimatedGamma + "1.0");
    }
    else if (currentModelStr == "LABA") {
        QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
        d->colorSpaceSelector->TongueWidget->setLABData(whitepoint);
        d->colorSpaceSelector->lbltrc->setToolTip(i18nc("@info:tooltip","This is assumed to be the L * TRC."));
        d->colorSpaceSelector->lbltrc->setText(estimatedGamma + "L*");
    }
    else if (currentModelStr == "YCbCrA") {
        QVector <double> whitepoint = currentColorSpace()->profile()->getWhitePointxyY();
        d->colorSpaceSelector->TongueWidget->setYCbCrData(whitepoint);
        d->colorSpaceSelector->lbltrc->setToolTip(i18nc("@info:tooltip","Estimated Gamma cannot be retrieved for YCrCb."));
        d->colorSpaceSelector->lbltrc->setText(estimatedGamma + notApplicable);
    }

    d->colorSpaceSelector->textProfileDescription->clear();
    d->colorSpaceSelector->textProfileDescription->append("<h3>About "  +  currentColorSpace()->name()  +  "/"  +  profileName  +  "</h3>");

    if (currentModelStr == "RGBA") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("If the selected model is RGB",
                                                                    "<b><a href=\"https://en.wikipedia.org/wiki/RGB_color_space\">RGB (Red, Green, Blue)</a></b>, is the color model used by screens and other light-based media.<br/>"
                                                                    "RGB is an additive color model: adding colors together makes them brighter. This color "
                                                                    "model is the most extensive of all color models, and is recommended as a model for painting,"
                                                                    "that you can later convert to other spaces. RGB is also the recommended colorspace for HDR editing."));
    } else if (currentModelStr == "CMYKA") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("If the selected model is CMYK",
                                                                    "<b><a href=\"https://en.wikipedia.org/wiki/CMYK_color_model\">CMYK (Cyan, Magenta, Yellow, Key)</a></b>, "
                                                                    "is the model used by printers and other ink-based media.<p>"
                                                                    "CMYK is a subtractive model, meaning that adding colors together will turn them darker. Because of CMYK "
                                                                    "profiles being very specific per printer, it is recommended to work in RGB space, and then later convert "
                                                                    "to a CMYK profile, preferably one delivered by your printer. <br/>"
                                                                    "CMYK is <b>not</b> recommended for painting."
                                                                    "Unfortunately, Krita cannot retrieve colorants or the TRC for this space."));
    } else if (currentModelStr == "XYZA") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("If the selected model is XYZ",
                                                                    "<b><a href=\"https://en.wikipedia.org/wiki/CIE_1931_color_space\">CIE XYZ</a></b>"
                                                                    "is the space determined by the CIE as the space that encompasses all other colors, and used to "
                                                                    "convert colors between profiles. XYZ is an additive color model, meaning that adding colors together "
                                                                    "makes them brighter. XYZ is <b>not</b> recommended for painting, but can be useful to encode in. The Tone Response "
                                                                    "Curve is assumed to be linear."));
    } else if (currentModelStr == "GRAYA") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("If the selected model is Grayscale",
                                                                    "<b><a href=\"https://en.wikipedia.org/wiki/Grayscale\">Grayscale</a></b> only allows for "
                                                                    "gray values and transparent values. Grayscale images use half "
                                                                    "the memory and disk space compared to an RGB image of the same bit-depth.<br/>"
                                                                    "Grayscale is useful for inking and greyscale images. In "
                                                                    "Krita, you can mix Grayscale and RGB layers in the same image."));
    } else if (currentModelStr == "LABA") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("If the selected model is LAB",
                                                                    "<b><a href=\"https://en.wikipedia.org/wiki/Lab_color_space\">L*a*b</a></b>. <b>L<b> stands for Lightness, "
                                                                    "the <b>a</b> and <b>b</b> components represent color channels.<br/>"
                                                                    "L*a*b is a special model for color correction. It is based on human perception, meaning that it "
                                                                    "tries to encode the difference in lightness, red-green balance and yellow-blue balance. "
                                                                    "This makes it useful for color correction, but the vast majority of color maths in the blending "
                                                                    "modes do <b>not</b> work as expected here.<br/>"
                                                                    "Similarly, Krita does not support HDR in LAB, meaning that HDR images converted to LAB lose color "
                                                                    "information. This colorspace is <b>not</b> recommended for painting, nor for export, "
                                                                    "but best as a space to do post-processing in. The Tone Response Curve is assumed to be the L* TRC."));
    } else if (currentModelStr == "YCbCrA") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("If the selected model is YCbCr",
                                                                    "<b><a href=\"https://en.wikipedia.org/wiki/YCbCr\">YCbCr (Luma, Blue Chroma, Red Chroma)</a></b>, is a "
                                                                    "model designed for video encoding. It is based on human perception, meaning that it tries to "
                                                                    "encode the difference in lightness, red-green balance and yellow-blue balance. Chroma in "
                                                                    "this case is then a word indicating a special type of saturation, in these cases the saturation "
                                                                    "of Red and Blue, of which the desaturated equivalents are Green and Yellow respectively. It "
                                                                    "is available to open up certain images correctly, but Krita does not currently ship a profile for "
                                                                    "this due to lack of open source ICC profiles for YCrCb."));
    }

    QString currentDepthStr = d->colorSpaceSelector->cmbColorDepth->currentItem().id();

    if (currentDepthStr == "U8") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("When the selected Bitdepth is 8",
                                                                    "<b>8 bit integer</b>: The default amount of colors per channel. Each channel will have 256 values available, "
                                                                    "leading to a total amount of 256*amount of channels. Recommended to use for images intended for the web, "
                                                                    "or otherwise simple images."));
    }
    else if (currentDepthStr == "U16") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("When the selected Bitdepth is 16",
                                                                    "<b>16 bit integer</b>: Also known as 'deep color'. 16 bit is ideal for editing images with a linear TRC, large "
                                                                    "color space, or just when you need more precise color blending. This does take twice as much space on "
                                                                    "the RAM and hard-drive than any given 8 bit image of the same properties, and for some devices it "
                                                                    "takes much more processing power. We recommend watching the RAM usage of the file carefully, or "
                                                                    "otherwise use 8 bit if your computer slows down. Take care to disable conversion optimization "
                                                                    "when converting from 16 bit/channel to 8 bit/channel."));
    }
    else if (currentDepthStr == "F16") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("When the selected Bitdepth is 16 bit float",
                                                                    "<b>16 bit floating point</b>: Also known as 'Half Floating Point', and the standard in VFX industry images. "
                                                                    "16 bit float is ideal for editing images with a linear Tone Response Curve, large color space, or just when you need "
                                                                    "more precise color blending. It being floating point is an absolute requirement for Scene Referred "
                                                                    "(HDR) images. This does take twice as much space on the RAM and hard-drive than any given 8 bit image "
                                                                    "of the same properties, and for some devices it takes much more processing power. We recommend watching "
                                                                    "the RAM usage of the file carefully, or otherwise use 8 bit if your computer slows down."));
    }
    else if (currentDepthStr == "F32") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("When the selected Bitdepth is 32bit float",
                                                                    "<b>32 bit float point</b>: Also known as 'Full Floating Point'. 32 bit float is ideal for editing images "
                                                                    "with a linear TRC, large color space, or just when you need more precise color blending. It being "
                                                                    "floating point is an absolute requirement for Scene Referred (HDR) images. This does take four times "
                                                                    "as much space on the RAM and hard-drive than any given 8 bit image of the same properties, and for "
                                                                    "some devices it takes much more processing power. We recommend watching the RAM usage of the file "
                                                                    "carefully, or otherwise use 8 bit if your computer slows down."));
    }
    else if (currentDepthStr == "F64") {
        d->colorSpaceSelector->textProfileDescription->append(i18nc("When the selected Bitdepth is 64bit float, but this isn't actually available in Krita at the moment.",\
                                                                    "<b>64 bit float point</b>: 64 bit float is as precise as it gets in current technology, and this depth is used "
                                                                    "most of the time for images that are generated or used as an input for software. It being floating point "
                                                                    "is an absolute requirement for Scene Referred (HDR) images. This does take eight times as much space on "
                                                                    "the RAM and hard-drive than any given 8 bit image of the same properties, and for some devices it takes "
                                                                    "much more processing power. We recommend watching the RAM usage of the file carefully, or otherwise use "
                                                                    "8 bit if your computer slows down."));
    }

    if (profileName.contains("-elle-")) {

        d->colorSpaceSelector->textProfileDescription->append(i18nc("These are Elle Stone's notes on her profiles that we ship.",
                                                                    "<p><b>Extra notes on profiles by Elle Stone:</b>"
                                                                    "<p><i>Krita comes with a number of high quality profiles created by "
                                                                    "<a href=\"http://ninedegreesbelow.com\">Elle Stone</a>. This is a summary. Please check "
                                                                    "<a href=\"http://ninedegreesbelow.com/photography/lcms-make-icc-profiles.html\">the full documentation</a> as well.</i>"));

                if (profileName.contains("ACES-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>Quoting Wikipedia, 'Academy Color Encoding System (ACES) is a color image "
                                                                        "encoding system proposed by the Academy of Motion Picture Arts and Sciences that will allow for "
                                                                        "a fully encompassing color accurate workflow, with 'seamless interchange of high quality motion "
                                                                        "picture images regardless of source'."));
        }
        if (profileName.contains("ACEScg-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>The ACEScg color space is smaller than the ACES color space, but large enough to contain the 'Rec-2020 gamut "
                                                                        "and the DCI-P3 gamut', unlike the ACES color space it has no negative values and contains only few colors "
                                                                        "that fall just barely outside the area of real colors humans can see"));
        }
        if (profileName.contains("ClayRGB-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "To avoid possible copyright infringement issues, I used 'ClayRGB' (following ArgyllCMS) as the base name "
                                                                        "for these profiles. As used below, 'Compatible with Adobe RGB 1998' is terminology suggested in the preamble "
                                                                        "to the AdobeRGB 1998 color space specifications.<p>"
                                                                        "The Adobe RGB 1998 color gamut covers a higher "
                                                                        "percentage of real-world cyans, greens, and yellow-greens than sRGB, but still doesn't include all printable "
                                                                        "cyans, greens, yellow-greens, especially when printing using today's high-end, wider gamut, ink jet printers. "
                                                                        "BetaRGB (not included in the profile pack) and Rec.2020 are better matches for the color gamuts of today's "
                                                                        "wide gamut printers.<p>"
                                                                        "The Adobe RGB 1998 color gamut is a reasonable approximation to some of today's "
                                                                        "high-end wide gamut monitors."));
        }
        if (profileName.contains("AllColorsRGB-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "This profile's color gamut is roughly the same size and shape as the ACES color space gamut, "
                                                                        "and like the ACES color space, AllColorsRGB holds all possible real colors. But AllColorsRGB "
                                                                        "actually has a slightly larger color gamut (to capture some fringe colors that barely qualify "
                                                                        "as real when viewed by the standard observer) and uses the D50 white point.<p>"
                                                                        "Just like the ACES color space, AllColorsRGB holds a high percentage of imaginary colors. See the Completely "
                                                                        "<a href=\"http://ninedegreesbelow.com/photography/xyz-rgb.html\">"
                                                                        "Painless Programmer's Guide to XYZ, RGB, ICC, xyY, and TRCs</a> for more information about imaginary "
                                                                        "colors.<p>"
                                                                        "There is no particular reason why anyone would want to use this profile "
                                                                        "for editing, unless one needs to make sure your color space really does hold all "
                                                                        "possible real colors."));
        }
        if (profileName.contains("CIERGB-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "This profile is included mostly for its historical significance. "
                                                                        "It's the color space that was used in the original color matching experiments "
                                                                        "that led to the creation of the XYZ reference color space.<p>"
                                                                        "The ASTM E white point "
                                                                        "is probably the right E white point to use when making the CIERGB color space profile. "
                                                                        "It's not clear to me what the correct CIERGB primaries really are. "
                                                                        "Lindbloom gives one set. The LCMS version 1 tutorial gives a different set. "
                                                                        "Experts in the field contend that the real primaries "
                                                                        "should be calculated from the spectral wavelengths, so I did."));
        }
        if (profileName.contains("IdentityRGB-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "The IdentityRGB working space is included in the profile pack because it's a mathematically "
                                                                        "obvious way to include all possible visible colors, though it has a higher percentage of "
                                                                        "imaginary colors than the ACES and AllColorsRGB color spaces. I cannot think of any reason "
                                                                        "why you'd ever want to actually edit images in the IdentityRGB working space."));
        }
        if (profileName.contains("LargeRGB-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "To avoid possible copyright infringement issues, I used 'LargeRGB' (following RawTherapee) "
                                                                        "as the base name for these profiles.<p>"
                                                                        "Kodak designed the RIMM/ROMM (ProPhotoRGB) color "
                                                                        "gamut to include all printable and most real world colors. It includes some imaginary colors "
                                                                        "and excludes some of the real world blues and violet blues that can be captured by digital "
                                                                        "cameras. It also excludes some very saturated 'camera-captured' yellows as interpreted by "
                                                                        "some (and probably many) camera matrix input profiles.<p>"
                                                                        "The ProPhotoRGB primaries are "
                                                                        "hard-coded into Adobe products such as Lightroom and the Dng-DCP camera 'profiles'. However, "
                                                                        "other than being large enough to hold a lot of colors, ProPhotoRGB has no particular merit "
                                                                        "as an RGB working space. Personally and for most editing purposes, I recommend BetaRGB, Rec2020, "
                                                                        "or the ACEScg profiles ProPhotoRGB."));
        }
        if (profileName.contains("Rec2020-")) {

            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "Rec.2020 is the up-and-coming replacement for the thoroughly outdated sRGB color space. As of "
                                                                        "June 2015, very few (if any) display devices (and certainly no affordable display devices) can "
                                                                        "display all of Rec.2020. However, display technology is closing in on Rec.2020, movies are "
                                                                        "already being made for Rec.2020, and various cameras offer support for Rec.2020. And in the "
                                                                        "digital darkroom Rec.2020 is much more suitable as a general RGB working space than the "
                                                                        "exceedingly small sRGB color space."));
        }
        if (profileName.contains("sRGB-")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "Hewlett-Packard and Microsoft designed sRGB to match the color gamut of consumer-grade CRTs "
                                                                        "from the 1990s. sRGB is the standard color space for the world wide web and is still the best "
                                                                        "choice for exporting images to the internet.<p>"
                                                                        "The sRGB color gamut was a good match to "
                                                                        "calibrated decent quality CRTs. But sRGB is not a good match to many consumer-grade LCD monitors, "
                                                                        "which often cannot display the more saturated sRGB blues and magentas (the good news: as technology "
                                                                        "progresses, wider gamuts are trickling down to consumer grade monitors).<p>"
                                                                        "Printer color gamuts can easily exceed the sRGB color gamut in cyans, greens, and yellow-greens. Colors from interpolated "
                                                                        "camera raw files also often exceed the sRGB color gamut.<p>"
                                                                        "As a very relevant aside, using perceptual "
                                                                        "intent when converting to sRGB does not magically makes otherwise out of gamut colors fit inside the "
                                                                        "sRGB color gamut! The standard sRGB color space (along with all the other the RGB profiles provided "
                                                                        "in my profile pack) is a matrix profile, and matrix profiles don't have perceptual intent tables.</p>"));
        }
        if (profileName.contains("WideRGB-")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "To avoid possible copyright infringement issues, I used 'WideRGB' as the base name for these profiles.<p>"
                                                                        "WideGamutRGB was designed by Adobe to be a wide gamut color space that uses spectral colors "
                                                                        "as its primaries. Pascale's primary values produce a profile that matches old V2 Widegamut profiles "
                                                                        "from Adobe and Canon. It is an interesting color space, but shortly after its introduction, Adobe "
                                                                        "switched their emphasis to the ProPhotoRGB color space."));
        }
        if (profileName.contains("Gray-")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>These profiles are for use with RGB images that have been converted to monotone gray (black and white). "
                                                                        "The main reason to convert from RGB to Gray is to save the file space needed to encode the image. "
                                                                        "Google places a premium on fast-loading web pages, and images are one of the slower-loading elements "
                                                                        "of a web page. So converting black and white images to Grayscale images does save some kilobytes. "
                                                                        " For grayscale images uploaded to the internet, convert the image to the V2 Gray profile with the sRGB TRC."));
        }
        if (profileName.contains("-g10")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>The profiles that end in '-g10.icc' are linear gamma (gamma=1.0, 'linear light', etc) profiles and "
                                                                        "should only be used when editing at high bit depths (16-bit floating point, 16-bit integer, 32-bit "
                                                                        "floating point, 32-bit integer). Many editing operations produce better results in linear gamma color "
                                                                        "spaces."));
        }
        if (profileName.contains("-labl")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>The profiles that end in '-labl.icc' have perceptually uniform TRCs. A few editing operations really "
                                                                        "should be done on perceptually uniform RGB. Make sure you use the V4 versions for editing high bit depth "
                                                                        "images."));
        }
        if (profileName.contains("-srgbtrc") || profileName.contains("-g22") || profileName.contains("-g18") || profileName.contains("-bt709")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>The profiles that end in '-srgbtrc.icc', '-g22.icc', and '-bt709.icc' have approximately but not exactly "
                                                                        "perceptually uniform TRCs. ProPhotoRGB's gamma=1.8 TRC is not quite as close to being perceptually uniform."));
        }
        if (d->colorSpaceSelector->cmbColorDepth->currentItem().id()=="U8") {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>When editing 8-bit images, you should use a profile with a small color gamut and an approximately or "
                                                                        "exactly perceptually uniform TRC. Of the profiles supplied in my profile pack, only the sRGB and AdobeRGB1998 "
                                                                        "(ClayRGB) color spaces are small enough for 8-bit editing. Even with the AdobeRGB1998 color space you need to "
                                                                        "be careful to not cause posterization. And of course you cannot use the linear gamma versions of these profiles "
                                                                        "for 8-bit editing."));
        }
        if (profileName.contains("-V4-")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>Use V4 profiles for editing images using high bit depth image editors that use LCMS as the Color Management Module. "
                                                                        "This includes Krita, digiKam/showFoto, and GIMP 2.9."));
        }
        if (profileName.contains("-V2-")) {
            d->colorSpaceSelector->textProfileDescription->append(i18nc("From Elle's notes.",
                                                                        "<p>Use V2 profiles for exporting finished images to be uploaded to the web or for use with imaging software that "
                                                                        "cannot read V4 profiles."));
        }
    }


    d->colorSpaceSelector->textProfileDescription->moveCursor(QTextCursor::Start);

}

QString KisAdvancedColorSpaceSelector::nameWhitePoint(QVector <double> whitePoint) {
    QString name=(QString::number(whitePoint[0]) + ", " + QString::number(whitePoint[1], 'f', 4));
    //A   (0.451170, 0.40594) (2856K)(tungsten)
    if ((whitePoint[0]>0.451170-0.005 && whitePoint[0]<0.451170 + 0.005) &&
            (whitePoint[1]>0.40594-0.005 && whitePoint[1]<0.40594 + 0.005)){
        name="A";
        return name;
    }
    //B   (0.34980, 0.35270) (4874K) (Direct Sunlight at noon)(obsolete)
    //C   (0.31039, 0.31905) (6774K) (avarage/north sky daylight)(obsolete)
    //D50 (0.34773, 0.35952) (5003K) (Horizon Light, default color of white paper, ICC profile standard illuminant)
    if ((whitePoint[0]>0.34773-0.005 && whitePoint[0]<0.34773 + 0.005) &&
            (whitePoint[1]>0.35952-0.005 && whitePoint[1]<0.35952 + 0.005)){
        name="D50";
        return name;
    }
    //D55 (0.33411,	0.34877) (5503K) (Mid-morning / Mid-afternoon Daylight)
    if ((whitePoint[0]>0.33411-0.001 && whitePoint[0]<0.33411 + 0.001) &&
            (whitePoint[1]>0.34877-0.005 && whitePoint[1]<0.34877 + 0.005)){
        name="D55";
        return name;
    }
    //D60 (0.3217, 0.3378) (~6000K) (ACES colorspace default)
    if ((whitePoint[0]>0.3217-0.001 && whitePoint[0]<0.3217 + 0.001) &&
            (whitePoint[1]>0.3378-0.005 && whitePoint[1]<0.3378 + 0.005)){
        name="D60";
        return name;
    }
    //D65 (0.31382, 0.33100) (6504K) (Noon Daylight, default for computer and tv screens, sRGB default)
    //Elle's are old school with 0.3127 and 0.3289
    if ((whitePoint[0]>0.31382-0.002 && whitePoint[0]<0.31382 + 0.002) &&
            (whitePoint[1]>0.33100-0.005 && whitePoint[1]<0.33100 + 0.002)){
        name="D65";
        return name;
    }
    //D75 (0.29968, 0.31740) (7504K) (North sky Daylight)
    if ((whitePoint[0]>0.29968-0.001 && whitePoint[0]<0.29968 + 0.001) &&
            (whitePoint[1]>0.31740-0.005 && whitePoint[1]<0.31740 + 0.005)){
        name="D75";
        return name;
    }
    //E   (1/3, 1/3)         (5454K) (Equal Energy. CIERGB default)
    if ((whitePoint[0]>(1.0/3.0)-0.001 && whitePoint[0]<(1.0/3.0) + 0.001) &&
            (whitePoint[1]>(1.0/3.0)-0.001 && whitePoint[1]<(1.0/3.0) + 0.001)){
        name="E";
        return name;
    }
    //The F series seems to sorta overlap with the D series, so I'll just leave them in comment here.//
    //F1  (0.31811, 0.33559) (6430K) (Daylight Fluorescent)
    //F2  (0.37925, 0.36733) (4230K) (Cool White Fluorescent)
    //F3  (0.41761, 0.38324) (3450K) (White Florescent)
    //F4  (0.44920, 0.39074) (2940K) (Warm White Fluorescent)
    //F5  (0.31975, 0.34246) (6350K) (Daylight Fluorescent)
    //F6  (0.38660, 0.37847) (4150K) (Lite White Fluorescent)
    //F7  (0.31569, 0.32960) (6500K) (D65 simulator, Daylight simulator)
    //F8  (0.34902, 0.35939) (5000K) (D50 simulator)
    //F9  (0.37829, 0.37045) (4150K) (Cool White Deluxe Fluorescent)
    //F10 (0.35090, 0.35444) (5000K) (Philips TL85, Ultralume 50)
    //F11 (0.38541, 0.37123) (4000K) (Philips TL84, Ultralume 40)
    //F12 (0.44256, 0.39717) (3000K) (Philips TL83, Ultralume 30)

    return name;
}

const KoColorSpace* KisAdvancedColorSpaceSelector::currentColorSpace()
{
    QString check = "";
    if (d->colorSpaceSelector->lstProfile->currentItem()) {
        check = d->colorSpaceSelector->lstProfile->currentItem()->text();
    } else {
        check = d->colorSpaceSelector->lstProfile->item(0)->text();
    }
    return KoColorSpaceRegistry::instance()->colorSpace(d->colorSpaceSelector->cmbColorModels->currentItem().id(),
                                                        d->colorSpaceSelector->cmbColorDepth->currentItem().id(),
                                                        check);
}

void KisAdvancedColorSpaceSelector::setCurrentColorModel(const KoID& id)
{
    d->colorSpaceSelector->cmbColorModels->setCurrent(id);
    fillLstProfiles();
    fillCmbDepths(id);
}

void KisAdvancedColorSpaceSelector::setCurrentColorDepth(const KoID& id)
{
    d->colorSpaceSelector->cmbColorDepth->setCurrent(id);
    fillLstProfiles();
}

void KisAdvancedColorSpaceSelector::setCurrentProfile(const QString& name)
{
    QList<QListWidgetItem *> Items= d->colorSpaceSelector->lstProfile->findItems(name, Qt::MatchStartsWith);
    d->colorSpaceSelector->lstProfile->setCurrentItem(Items.at(0));
}

void KisAdvancedColorSpaceSelector::setCurrentColorSpace(const KoColorSpace* colorSpace)
{
    setCurrentColorModel(colorSpace->colorModelId());
    setCurrentColorDepth(colorSpace->colorDepthId());
    setCurrentProfile(colorSpace->profile()->name());
}

void KisAdvancedColorSpaceSelector::colorSpaceChanged()
{
    bool valid = d->colorSpaceSelector->lstProfile->count() != 0;
    emit(selectionChanged(valid));
    if (valid) {
        emit colorSpaceChanged(currentColorSpace());
    }
}

void KisAdvancedColorSpaceSelector::installProfile()
{
    QStringList mime;
    mime << "*.icm" <<  "*.icc";
    KoFileDialog dialog(this, KoFileDialog::OpenFiles, "OpenDocumentICC");
    dialog.setCaption(i18n("Install Color Profiles"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    dialog.setNameFilters(mime);
    QStringList profileNames = dialog.urls();

    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);

    QString saveLocation = KGlobal::dirs()->saveLocation("icc_profiles");

    foreach (const QString &profileName, profileNames) {
        KUrl file(profileName);
        if (!QFile::copy(profileName, saveLocation  +  file.fileName())) {
            dbgKrita << "Could not install profile!";
            return;
        }
        iccEngine->addProfile(saveLocation  +  file.fileName());

    }

    fillLstProfiles();
}

void KisAdvancedColorSpaceSelector::downloadProfile()
{
#ifdef GHNS
    KNS3::DownloadDialog dialog( "kritaiccprofiles.knsrc", this);
    dialog.exec();
    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);
    foreach (const KNS3::Entry& e, dialog.changedEntries()) {
        foreach(const QString &file, e.installedFiles()) {
            QFileInfo fi(file);
            iccEngine->addProfile( fi.absolutePath() + '/' + fi.fileName());
        }
        foreach(const QString &file, e.uninstalledFiles()) {
            QFileInfo fi(file);
            iccEngine->removeProfile( fi.absolutePath() + '/' + fi.fileName());
        }
    }
    fillLstProfiles();
#endif
}

void KisAdvancedColorSpaceSelector::uploadProfile()
{
#ifdef GHNS
    KNS3::UploadDialog dialog("kritaiccprofiles.knsrc", this);
    const KoColorProfile *  profile = KoColorSpaceRegistry::instance()->profileByName(d->colorSpaceSelector->lstProfile->currentText());
    if (!profile)  return;
    dialog.setUploadFile(KUrl::fromLocalFile(profile->fileName()));
    dialog.setUploadName(profile->name());
    dialog.exec();
#endif
}

void KisAdvancedColorSpaceSelector::buttonUpdate()
{
    QString check = "";
    if (d->colorSpaceSelector->lstProfile->currentItem()) {
        check = d->colorSpaceSelector->lstProfile->currentItem()->text();
    } else {
        check = d->colorSpaceSelector->lstProfile->item(0)->text();
    }
    const KoColorProfile *  profile = KoColorSpaceRegistry::instance()->profileByName(check);
    if (!profile)  return;

    QFileInfo fileInfo(profile->fileName());
    if (fileInfo.isWritable()) {
        d->colorSpaceSelector->bnUploadProfile->setEnabled( true );
        return;
    }
    d->colorSpaceSelector->bnUploadProfile->setEnabled( false );
}

