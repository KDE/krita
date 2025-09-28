/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef CSSQMLUNITCONVERTER_H
#define CSSQMLUNITCONVERTER_H

#include <QObject>

class KoSvgTextPropertiesModel;
/**
 * @brief The CssQmlUnitConverter class
 *
 * This handles converter to and from various units.
 * For the absolute units, KoUnit is used internally.
 * For the font-relative units, it will use the fontMetrics;
 */
class CssQmlUnitConverter : public QObject
{
    Q_OBJECT

    /// The DPI used to calculate pixel to physical properties
    Q_PROPERTY(qreal dpi READ dpi WRITE setDpi NOTIFY dpiChanged)

    /// Data multiplier is used if for some reason the user-value needs to be multiplied by a certain factor.
    Q_PROPERTY(qreal dataMultiplier READ dataMultiplier WRITE setDataMultiplier NOTIFY dataMultiplierChanged)

    /// dataValue and dataUnit represent the data as used by the text properties.
    Q_PROPERTY(qreal dataValue READ dataValue WRITE setDataValue NOTIFY dataValueChanged)
    Q_PROPERTY(int dataUnit READ dataUnit WRITE setDataUnit NOTIFY dataUnitChanged)

    /// userValue and userUnit represent the user-visible data. This can include synthetic units like px and mm from pt.
    Q_PROPERTY(qreal userValue READ userValue WRITE setUserValue NOTIFY userValueChanged)
    Q_PROPERTY(int userUnit READ userUnit WRITE setUserUnit NOTIFY userUnitChanged)

    /// A model for the available user units.
    Q_PROPERTY(QVariantList userUnitModel READ userUnitModel NOTIFY userUnitModelChanged)

    /// The symbol.
    Q_PROPERTY(QString symbol READ symbol NOTIFY userUnitChanged)

    /// value to represent 100%
    Q_PROPERTY(qreal percentageReference READ percentageReference WRITE setPercentageReference NOTIFY percentageReferenceChanged)

public:
    CssQmlUnitConverter(QObject *parent = nullptr);

    ~CssQmlUnitConverter();

    enum UserUnits {
        Px, ///< Pixels, dpi relative
        Pt, ///< Points, 1/72th
        Mm, ///< Millimeters
        Cm, ///< Centimeters
        Inch,///< Inches.
        //
        Percentage,
        // Font Relative
        Em, ///< FontSize
        Ex, ///< Font x-height
        Cap,///< Cap height
        Ch, ///< average proportional advance
        Ic, ///< average full-width advance
        Lh, ///< line-height
        // unique for certain properties
        Lines,  ///< Used by lineHeight
        Spaces, ///< Used by tabs
    };

    Q_ENUM(UserUnits)

    /**
     * @brief setDataUnitMap
     * @param map -- variant list of VariantMaps, where "data" is an int representing a dataUnit,
     * and "user" is an int representing a value in the UserUnits enum.
     */
    Q_INVOKABLE void setDataUnitMap(const QVariantList &unitMap);

    /**
     * @brief setFontMetricsFromTextPropertiesModel
     * Set the current font metrics from the text properties model.
     * @param textPropertiesModel
     */
    Q_INVOKABLE void setFontMetricsFromTextPropertiesModel(KoSvgTextPropertiesModel *textPropertiesModel, bool isFontSize = false, bool isLineHeight = false);

    /**
     * @brief setFromNormalLineHeight
     * set the current value from line-height:normal metrics.
     */
    Q_INVOKABLE void setFromNormalLineHeight();

    /**
     * @brief setDataValueAndUnit
     * set data unit and value in one go.
     */
    Q_INVOKABLE void setDataValueAndUnit(const qreal value, const int unit);

    qreal dpi() const;
    void setDpi(qreal newDpi);

    qreal dataMultiplier() const;
    void setDataMultiplier(qreal newDataMultiplier);

    qreal dataValue() const;
    void setDataValue(qreal newDataValue);

    int dataUnit() const;
    void setDataUnit(int newDataUnit);

    qreal userValue() const;
    void setUserValue(qreal newUserValue);

    int userUnit() const;
    void setUserUnit(int newUserUnit);

    QVariantList userUnitModel() const;

    QString symbol() const;

    qreal percentageReference() const;
    void setPercentageReference(qreal newPercentageReference);

Q_SIGNALS:

    void dpiChanged();
    void dataMultiplierChanged();

    void dataValueChanged();

    void dataUnitChanged();

    void userValueChanged();

    void userUnitChanged();

    void userUnitModelChanged();

    void percentageReferenceChanged();

private:
    // Converts from a relative value (font or percentage) to pt.
    qreal convertFromRelativeValue(const qreal value, const UserUnits type) const;
    // Converts from pt to the given relative value (font or percentage).
    qreal convertToRelativeValue(const qreal value, const UserUnits type)const ;

    struct Private;
    const QScopedPointer<Private> d;
};

#endif // CSSQMLUNITCONVERTER_H
