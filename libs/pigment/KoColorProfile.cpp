/*
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <cmath>

#include "KoColorProfile.h"
#include "DebugPigment.h"
#include "kis_assert.h"

struct Q_DECL_HIDDEN KoColorProfile::Private {
    QString name;
    QString info;
    QString fileName;
    QString manufacturer;
    QString copyright;
    int primaries {-1};
    TransferCharacteristics characteristics {TRC_UNSPECIFIED};
};

KoColorProfile::KoColorProfile(const QString &fileName) : d(new Private)
{
//     dbgPigment <<" Profile filename =" << fileName;
    d->fileName = fileName;
}

KoColorProfile::KoColorProfile(const KoColorProfile& profile)
    : d(new Private(*profile.d))
{
}

KoColorProfile::~KoColorProfile()
{
    delete d;
}

bool KoColorProfile::load()
{
    return false;
}

bool KoColorProfile::save(const QString & filename)
{
    Q_UNUSED(filename);
    return false;
}


QString KoColorProfile::name() const
{
    return d->name;
}

QString KoColorProfile::info() const
{
    return d->info;
}
QString KoColorProfile::manufacturer() const
{
    return d->manufacturer;
}
QString KoColorProfile::copyright() const
{
    return d->copyright;
}
QString KoColorProfile::fileName() const
{
    return d->fileName;
}

void KoColorProfile::setFileName(const QString &f)
{
    d->fileName = f;
}

ColorPrimaries KoColorProfile::getColorPrimaries() const
{
    if (d->primaries == -1) {
        ColorPrimaries primaries = PRIMARIES_UNSPECIFIED;
        QVector<qreal> wp = getWhitePointxyY();

        bool match = false;
        if (hasColorants()) {
            QVector<qreal> col = getColorantsxyY();
            if (col.size()<8) {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(col.size() < 8, PRIMARIES_UNSPECIFIED);
                //too few colorants.
                d->primaries = int(primaries);
                return (primaries);
            }
            QVector<double> colorants = {wp[0], wp[1], col[0], col[1], col[3], col[4], col[6], col[7]};
            QVector<double> compare;

            QVector<ColorPrimaries> primariesList = {PRIMARIES_ITU_R_BT_709_5, PRIMARIES_ITU_R_BT_601_6, PRIMARIES_ITU_R_BT_470_6_SYSTEM_M,
                                                     PRIMARIES_ITU_R_BT_2020_2_AND_2100_0, PRIMARIES_SMPTE_EG_432_1, PRIMARIES_SMPTE_RP_431_2,
                                                     PRIMARIES_SMPTE_ST_428_1, PRIMARIES_GENERIC_FILM, PRIMARIES_SMPTE_240M, PRIMARIES_EBU_Tech_3213_E,
                                                     PRIMARIES_ADOBE_RGB_1998, PRIMARIES_PROPHOTO, PRIMARIES_ITU_R_BT_470_6_SYSTEM_B_G};

            for (ColorPrimaries check: primariesList) {
                colorantsForType(check, compare);
                if (compare.size() <8) {
                    KIS_SAFE_ASSERT_RECOVER(compare.size() < 8) { continue; }
                    //too few colorants, skip.
                }
                match = true;
                for (int i=0; i<colorants.size(); i++) {
                    match = std::fabs(colorants[i] - compare[i]) < 0.00001;
                    if (!match) {
                        break;
                    }
                }
                if (match) {
                    primaries = check;
                }
            }
        }

        d->primaries = int(primaries);
    }
    return ColorPrimaries(d->primaries);
}

QString KoColorProfile::getColorPrimariesName(ColorPrimaries primaries)
{
    switch (primaries) {
    case PRIMARIES_ITU_R_BT_709_5:
        return QStringLiteral("Rec. 709");
    case PRIMARIES_ITU_R_BT_470_6_SYSTEM_M:
        return QStringLiteral("BT. 470 System M");
    case PRIMARIES_ITU_R_BT_470_6_SYSTEM_B_G:
        return QStringLiteral("BT. 470 System B, G");
    case PRIMARIES_GENERIC_FILM:
        return QStringLiteral("Generic Film");
    case PRIMARIES_SMPTE_240M:
        return QStringLiteral("SMPTE 240 M");
    case PRIMARIES_ITU_R_BT_2020_2_AND_2100_0:
        return QStringLiteral("Rec. 2020");
    case PRIMARIES_ITU_R_BT_601_6:
        return QStringLiteral("Rec. 601");
    case PRIMARIES_SMPTE_EG_432_1:
        return QStringLiteral("Display P3");
    case PRIMARIES_SMPTE_RP_431_2:
        return QStringLiteral("DCI P3");
    case PRIMARIES_SMPTE_ST_428_1:
        return QStringLiteral("XYZ primaries");
    case PRIMARIES_EBU_Tech_3213_E:
        return QStringLiteral("EBU Tech 3213 E");
    case PRIMARIES_PROPHOTO:
        return QStringLiteral("ProPhoto");
    case PRIMARIES_ADOBE_RGB_1998:
        return QStringLiteral("A98");
    case PRIMARIES_UNSPECIFIED:
        break;
    }
    return QStringLiteral("Unspecified");
}

void KoColorProfile::colorantsForType(ColorPrimaries primaries, QVector<double> &colorants)
{
    switch (ColorPrimaries(primaries)) {
    case PRIMARIES_UNSPECIFIED:
        break;
    case PRIMARIES_ITU_R_BT_470_6_SYSTEM_M:
        // Unquantisized.
        colorants = {0.310, 0.316};
        colorants.append({0.67, 0.33});
        colorants.append({0.21, 0.71});
        colorants.append({0.14, 0.08});
        //Illuminant C
        break;
    case PRIMARIES_ITU_R_BT_470_6_SYSTEM_B_G:
        // Unquantisized.
        colorants = {0.3127, 0.3290};
        colorants.append({0.64, 0.33});
        colorants.append({0.29, 0.60});
        colorants.append({0.1500, 0.06});
        break;
    case PRIMARIES_ITU_R_BT_601_6:
        colorants = {0.3127, 0.3290};
        colorants.append({0.630, 0.340});
        colorants.append({0.310, 0.595});
        colorants.append({0.155, 0.070});
        break;
    case PRIMARIES_SMPTE_240M:
        colorants = {0.3127, 0.3290};
        colorants.append({0.630, 0.340});
        colorants.append({0.310, 0.595});
        colorants.append({0.155, 0.070});
        break;
    case PRIMARIES_GENERIC_FILM:
        colorants = {0.310, 0.316};
        colorants.append({0.681, 0.319});
        colorants.append({0.243, 0.692});
        colorants.append({0.145, 0.049});
        //Illuminant C
        break;
    case PRIMARIES_ITU_R_BT_2020_2_AND_2100_0:
        //prequantization courtesy of Elle Stone.
        colorants = {0.3127, 0.3290};
        colorants.append({0.708012540607, 0.291993664388});
        colorants.append({0.169991652439, 0.797007778423});
        colorants.append({0.130997824007, 0.045996550894});
        break;
    case PRIMARIES_SMPTE_ST_428_1:
        colorants = {1/3, 1/3};
        colorants.append({1.0, 0});
        colorants.append({0, 1.0});
        colorants.append({0, 0});
        break;
    case PRIMARIES_SMPTE_RP_431_2:
        colorants = {0.314, 0.351};
        colorants.append({0.6800, 0.3200});
        colorants.append({0.2650, 0.6900});
        colorants.append({0.1500, 0.0600});
        break;
    case PRIMARIES_SMPTE_EG_432_1:
        colorants = {0.3127, 0.3290};
        colorants.append({0.6800, 0.3200});
        colorants.append({0.2650, 0.6900});
        colorants.append({0.1500, 0.0600});
        break;
    case PRIMARIES_EBU_Tech_3213_E:
        colorants = {0.3127, 0.3290};
        colorants.append({0.63, 0.34});
        colorants.append({0.295, 0.605});
        colorants.append({0.155, 0.077});
        break;
    case PRIMARIES_PROPHOTO:
        //prequantization courtesy of Elle Stone.
        colorants = {0.3457, 0.3585};
        colorants.append({0.7347, 0.2653});
        colorants.append({0.1596, 0.8404});
        colorants.append({0.0366, 0.0001});
        break;
    case PRIMARIES_ADOBE_RGB_1998:
        //prequantization courtesy of Elle Stone.
        colorants = {0.3127, 0.3290};
        colorants.append({0.639996511, 0.329996864});
        colorants.append({0.210005295, 0.710004866});
        colorants.append({0.149997606, 0.060003644});
        break;
    case PRIMARIES_ITU_R_BT_709_5:
    default:
        // Prequantisized colorants, courtesy of Elle Stone
        colorants = {0.3127, 0.3290};
        colorants.append({0.639998686, 0.330010138});
        colorants.append({0.300003784, 0.600003357});
        colorants.append({0.150002046, 0.059997204});
        break;

    }
}

TransferCharacteristics KoColorProfile::getTransferCharacteristics() const
{
    return d->characteristics;
}

void KoColorProfile::setCharacteristics(ColorPrimaries primaries, TransferCharacteristics curve)
{
    d->primaries = int(primaries);
    d->characteristics = curve;
}

QString KoColorProfile::getTransferCharacteristicName(TransferCharacteristics curve)
{
    switch (curve) {
    case TRC_ITU_R_BT_709_5:
    case TRC_ITU_R_BT_601_6:
    case TRC_ITU_R_BT_2020_2_10bit:
        return QString("rec 709 trc");
    case TRC_ITU_R_BT_2020_2_12bit:
        return QString("rec 2020 12bit trc");
    case TRC_ITU_R_BT_470_6_SYSTEM_M:
        return QString("Gamma 2.2");
    case TRC_ITU_R_BT_470_6_SYSTEM_B_G:
        return QString("Gamma 2.8");
    case TRC_SMPTE_240M:
        return QString("SMPTE 240 trc");
    case TRC_LINEAR:
        return QString("Linear");
    case TRC_LOGARITHMIC_100:
        return QString("Logarithmic 100");
    case TRC_LOGARITHMIC_100_sqrt10:
        return QString("Logarithmic 100 sqrt10");
    case TRC_IEC_61966_2_4:
        return QString("IEC 61966 2.4");
    case TRC_ITU_R_BT_1361:
    case TRC_IEC_61966_2_1:
        return QString("sRGB trc");
    case TRC_SMPTE_ST_428_1:
        return QString("SMPTE ST 428");
    case TRC_ITU_R_BT_2100_0_PQ:
        return QString("Perceptual Quantizer");
    case TRC_ITU_R_BT_2100_0_HLG:
        return QString("Hybrid Log Gamma");
    case TRC_GAMMA_1_8:
        return QString("Gamma 1.8");
    case TRC_GAMMA_2_4:
        return QString("Gamma 2.4");
    case TRC_A98:
        return QString("Gamma A98");
    case TRC_PROPHOTO:
        return QString("ProPhoto trc");
    case TRC_UNSPECIFIED:
        break;
    }

    return QString("Unspecified");
}

void KoColorProfile::setName(const QString &name)
{
    d->name = name;
}
void KoColorProfile::setInfo(const QString &info)
{
    d->info = info;
}
void KoColorProfile::setManufacturer(const QString &manufacturer)
{
    d->manufacturer = manufacturer;
}
void KoColorProfile::setCopyright(const QString &copyright)
{
    d->copyright = copyright;
}
