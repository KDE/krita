/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_PROFILE_H_
#define _KO_COLOR_PROFILE_H_

#include <boost/operators.hpp>
#include <QString>
#include <QVector>
#include <QVariant>

#include "kritapigment_export.h"

/**
 * Contains information needed for color transformation.
 */
class KRITAPIGMENT_EXPORT KoColorProfile : public boost::equality_comparable<KoColorProfile>
{

public:

    /**
     * @param fileName file name to load or save that profile
     */
    explicit KoColorProfile(const QString &fileName = QString());
    KoColorProfile(const KoColorProfile& profile);
    virtual ~KoColorProfile();

    /**
     * @return the type of this profile (icc, ctlcs etc)
     */
    virtual QString type() const {
        return QString();
    }

    /**
     * Create a copy of this profile.
     * Data that shall not change during the life time of the profile shouldn't be
     * duplicated but shared, like for instance ICC data.
     *
     * Data that shall be changed like a palette or hdr information such as exposure
     * must be duplicated while cloning.
     */
    virtual KoColorProfile* clone() const = 0;

    /**
     * Load the profile in memory.
     * @return true if the profile has been successfully loaded
     */
    virtual bool load();

    /**
     * Override this function to save the profile.
     * @param fileName destination
     * @return true if the profile has been successfully saved
     */
    virtual bool save(const QString &fileName);

    /**
     * @return true if the profile is valid, false if it isn't been loaded in memory yet, or
     * if the loaded memory is a bad profile
     */
    virtual bool valid() const = 0;

    /**
     * @return the name of this profile
     */
    QString name() const;
    /**
     * @return the info of this profile
     */
    QString info() const;
    /** @return manufacturer of the profile
     */
    QString manufacturer() const;
    /**
     * @return the copyright of the profile
     */
    QString copyright() const;
    /**
     * @return the filename of the profile (it might be empty)
     */
    QString fileName() const;
    /**
     * @param filename new filename
     */
    void setFileName(const QString &filename);

    /**
     * Return version
     */
    virtual float version() const = 0;

    /**
     * @return a string for a color model id.
     */
    virtual QString colorModelID() const {
        return QString();
    };
    /**
     * @return true if you can use this profile can be used to convert color from a different
     * profile to this one
     */
    virtual bool isSuitableForOutput() const = 0;
    /**
     * @return true if this profile is suitable to use for printing
     */
    virtual bool isSuitableForPrinting() const = 0;
    /**
     * @return true if this profile is suitable to use for display
     */
    virtual bool isSuitableForDisplay() const = 0;

    /**
     * @return which rendering intents are supported
     */
    virtual bool supportsPerceptual() const = 0;
    virtual bool supportsSaturation() const = 0;
    virtual bool supportsAbsolute() const = 0;
    virtual bool supportsRelative() const = 0;
    /**
     * @return if the profile has colorants.
     */
    virtual bool hasColorants() const = 0;
    /**
     * @return a qvector <double>(9) with the RGB colorants in XYZ
     */
    virtual QVector <qreal> getColorantsXYZ() const = 0;
    /**
     * @return a qvector <double>(9) with the RGB colorants in xyY
     */
    virtual QVector <qreal> getColorantsxyY() const = 0;
    /**
     * @return a qvector <double>(3) with the whitepoint in XYZ
     */
    virtual QVector <qreal> getWhitePointXYZ() const = 0;
    /**
     * @return a qvector <double>(3) with the whitepoint in xyY
     */
    virtual QVector <qreal> getWhitePointxyY() const = 0;
    
    /**
     * @return estimated gamma for RGB and Grayscale profiles
     */
    virtual QVector <qreal> getEstimatedTRC() const = 0;

    /**
     * @return if the profile has a TRC(required for linearisation).
     */
    virtual bool hasTRC() const = 0;
    /**
     * @return if the profile's TRCs are linear.
     */
    virtual bool isLinear() const = 0;
    /**
     * Linearizes first 3 values of QVector, leaving other values unchanged.
     * Returns the same QVector if it is not possible to linearize.
     */
    virtual void linearizeFloatValue(QVector <qreal> & Value) const = 0;
    /**
     * Delinearizes first 3 values of QVector, leaving other values unchanged.
     * Returns the same QVector if it is not possible to delinearize.
     * Effectively undoes LinearizeFloatValue.
     */
    virtual void delinearizeFloatValue(QVector <qreal> & Value) const = 0;
    /**
     * More imprecise versions of the above(limited to 16bit, and can't
     * delinearize above 1.0.) Use this for filters and images.
     */
    virtual void linearizeFloatValueFast(QVector <qreal> & Value) const = 0;
    virtual void delinearizeFloatValueFast(QVector <qreal> & Value) const = 0;

    virtual QByteArray uniqueId() const = 0;
    
    virtual bool operator==(const KoColorProfile&) const = 0;

    /**
     * @return an array with the raw data of the profile
     */
    virtual QByteArray rawData() const {
        return QByteArray();
    }

    /**
     * @brief The colorPrimaries enum
     * Enum of colorants, follows ITU H.273 for values 0 to 255,
     * and has extra known quantities starting at 256.
     *
     * This is used for profile generation and file encoding.
     */
    enum colorPrimaries {
        //0 is resevered
        Primaries_ITU_R_BT_709_5 = 1, // sRGB, rec 709
        Primaries_Unspecified = 2,
        //3 is reserved
        Primaries_ITU_R_BT_470_6_System_M = 4,
        Primaries_ITU_R_BT_470_6_System_B_G,
        Primaries_ITU_R_BT_601_6,
        Primaries_SMPTE_240M, // Old HDTv standard.
        Primaries_generic_film, // H.273 says 'color filters using illuminant C'
        Primaries_ITU_R_BT_2020_2_and_2100_0,
        Primaries_SMPTE_ST_428_1, // XYZ
        Primaries_SMPTE_RP_431_2, //DCI P3, or Digital Cinema Projector
        Primaries_SMPTE_EG_432_1, //Display P3
        // 13-21 are reserved.
        Primaries_EBU_Tech_3213_E = 22,
        // 23-255 are reserved by ITU/ISO
        Primaries_Adobe_RGB_1998 = 256,
        Primaries_ProPhoto
    };

    /**
     * @brief getColorPrimaries
     * @return colorprimaries, defaults to 'unspecified' if no match is possible.
     */
    virtual colorPrimaries getColorPrimaries() const;

    /**
     * @brief getColorPrimariesName
     * @param primaries
     * @return human friendly name of the primary.
     */
    static QString getColorPrimariesName(colorPrimaries primaries);
    /**
     * @brief colorantsForPrimaries
     * fills a QVector<float> with the xy values of the whitepoint and red, green, blue colorants for
     * a given predefined value. Will not change the vector when the primaries are set to 'undefined'.
     * @param primaries predefined value.
     * @param colorants the vector to fill.
     */
    static void colorantsForType(colorPrimaries primaries, QVector<double> &colorants);

    /**
     * @brief The transferCharacteristics enum
     * Enum of transfer characteristics, follows ITU H.273 for values 0 to 255,
     * and has extra known quantities starting at 256.
     *
     * This is used for profile generation and file encoding.
     */
    enum transferCharacteristics {
        // 0 is reserved.
        TRC_ITU_R_BT_709_5 = 1, //Different from sRGB, officially HDTV
        TRC_Unspecified, // According to H.273: Image characteristics are unknown or are determined by the application.
        // 3 is reserved.
        TRC_ITU_R_BT_470_6_System_M = 4, //Assumed gamma 2.2
        TRC_ITU_R_BT_470_6_System_B_G, // Assume gamma 2.8
        TRC_ITU_R_BT_601_6, //SDTV
        TRC_SMPTE_240M,
        TRC_linear,
        TRC_logarithmic_100,
        TRC_logarithmic_100_sqrt10,
        TRC_IEC_61966_2_4, //xvYCC, not to be confused with sRGB and scRGB.
        TRC_ITU_R_BT_1361, // H.273: "Extended color gamut system (historical)"
        TRC_IEC_61966_2_1, //sRGB, different from rec709
        TRC_ITU_R_BT_2020_2_10bit, //formally, rec 709, can also be gamma 2.4 UHDTV
        TRC_ITU_R_BT_2020_2_12bit, //formally, a precise version of rec 709. UHDTV
        TRC_ITU_R_BT_2100_0_PQ, //Perceptual Quantizer, cannot be represented in icc 4, also named SMPTE 2048 curve, HDR
        TRC_SMPTE_ST_428_1, //O = (48 * I / 52.37) ^ (1 / 2.6), cannot be represented in icc 4
        TRC_ITU_R_BT_2100_0_HLG, //Hybrid Log Gamma, cannot be represented in icc 4, HDR
        // 19-255 are reserved by ITU/ISO
        TRC_GAMMA_1_8 = 256, //Gamma 1.8
        TRC_GAMMA_2_4, //Gamma 2.4
        TRC_ProPhoto, //Gamma 1.8 unless under 16/512
        TRC_A98 //Gamma of 256/563
    };

    /**
     * @brief getTransferCharacteristics
     * This function should be subclassed at some point so we can get the value from the lcms profile.
     * @return transferfunction number.
     */
    virtual transferCharacteristics getTransferCharacteristics() const;


    /**
     * @brief getTransferCharacteristicName
     * @param curve the number
     * @return name of the characteristic
     */
    static QString getTransferCharacteristicName(transferCharacteristics curve);

protected:
    /**
     * Allows to define the name of this profile.
     */
    void setName(const QString &name);
    /**
     * Allows to set the information string of that profile.
     */
    void setInfo(const QString &info);
    /**
     * Allows to set the manufacturer string of that profile.
     */
    void setManufacturer(const QString &manufacturer);
    /**
     * Allows to set the copyright string of that profile.
     */
    void setCopyright(const QString &copyright);

    /**
     * @brief setCharacteristics
     * ideally, we'd read this from the icc profile curve, but that can be tricky, instead
     * we'll set it on profile creation.
     * @param curve
     */
    void setCharacteristics(colorPrimaries primaries, transferCharacteristics curve);

private:
    struct Private;
    Private* const d;
};

#endif
