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

private:
    struct Private;
    Private* const d;
};

#endif
