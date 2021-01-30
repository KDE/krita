/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_ICC_COLOR_PROFILE_H_
#define _KO_ICC_COLOR_PROFILE_H_

#include "KoColorProfile.h"
#include "KoChannelInfo.h"

class LcmsColorProfileContainer;

/**
 * This class contains an ICC color profile.
 */
class IccColorProfile : public KoColorProfile
{
public:

    using KoColorProfile::save;

    /**
     * Contains the data associated with a profile. This is
     * shared through internal representation.
     */
    class Data
    {
    public:
        Data();
        explicit Data(const QByteArray &rawData);
        ~Data();
        QByteArray rawData();
        void setRawData(const QByteArray &);
    private:
        struct Private;
        QScopedPointer<Private> const d;
    };
    /**
     * This class should be used to wrap the ICC profile
     * representation coming from various CMS engine.
     */
    class Container
    {
    public:
        Container();
        virtual ~Container();
    public:
        virtual QString name() const = 0;
        virtual QString info() const = 0;
        virtual QString manufacturer() const = 0;
        virtual QString copyright() const = 0;
        virtual bool valid() const = 0;
        virtual bool isSuitableForOutput() const = 0;
        virtual bool isSuitableForPrinting() const = 0;
        virtual bool isSuitableForDisplay() const = 0;
        virtual bool hasColorants() const = 0;
        virtual QVector <double> getColorantsXYZ() const = 0;
        virtual QVector <double> getColorantsxyY() const = 0;
        virtual QVector <double> getWhitePointXYZ() const = 0;
        virtual QVector <double> getWhitePointxyY() const = 0;
        virtual QVector <double> getEstimatedTRC() const = 0;
        virtual QByteArray getProfileUniqueId() const = 0;
    };
public:

    explicit IccColorProfile(const QString &fileName = QString());
    explicit IccColorProfile(const QByteArray &rawData);
    IccColorProfile(const IccColorProfile &rhs);
    ~IccColorProfile() override;

    KoColorProfile *clone() const override;

    bool load() override;
    virtual bool save();

    /**
    * @return an array with the raw data of the profile
    */
    QByteArray rawData() const override;
    bool valid() const override;
    float version() const override;
    QString colorModelID() const override;
    bool isSuitableForOutput() const override;
    bool isSuitableForPrinting() const override;
    bool isSuitableForDisplay() const override;
    bool supportsPerceptual() const override;
    bool supportsSaturation() const override;
    bool supportsAbsolute() const override;
    bool supportsRelative() const override;
    bool hasColorants() const override;
    bool hasTRC() const override;
    bool isLinear() const override;
    QVector <qreal> getColorantsXYZ() const override;
    QVector <qreal> getColorantsxyY() const override;
    QVector <qreal> getWhitePointXYZ() const override;
    QVector <qreal> getWhitePointxyY() const override;
    QVector <qreal> getEstimatedTRC() const override;
    void linearizeFloatValue(QVector <qreal> & Value) const override;
    void delinearizeFloatValue(QVector <qreal> & Value) const override;
    void linearizeFloatValueFast(QVector <qreal> & Value) const override;
    void delinearizeFloatValueFast(QVector <qreal> & Value) const override;
    QByteArray uniqueId() const override;
    bool operator==(const KoColorProfile &) const override;
    QString type() const override
    {
        return "icc";
    }

    /**
     * Returns the set of min/maxes for each channel in this profile.
     * These (sometimes approximate) min and maxes are suitable
     * for UI building.
     * Furthermore, then only apply to the floating point uses of this profile,
     * and not the integer variants.
     */
    const QVector<KoChannelInfo::DoubleRange> &getFloatUIMinMax(void) const;

protected:
    void setRawData(const QByteArray &rawData);
public:
    LcmsColorProfileContainer *asLcms() const;
protected:
    bool init();
    void calculateFloatUIMinMax(void);
private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif
