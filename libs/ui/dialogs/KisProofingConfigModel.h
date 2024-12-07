/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPROOFINGCONFIGMODEL_H
#define KISPROOFINGCONFIGMODEL_H

#include <QObject>

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <lager/state.hpp>

#include <KisProofingConfiguration.h>

/**
 * @brief The KisProofingConfigModel class
 *
 * This encapsulates KisProofingConfig with a lager model.
 * As KisProofingConfig describes two color conversions,
 * (From image to proof space, and from proof to monitor)
 * this model allows us to simplify the second transform
 * into Paper, Monitor and Custom modes, which should help
 * artists to use softproofing more effectively.
 */
class KisProofingConfigModel : public QObject
{
    Q_OBJECT
public:
    KisProofingConfigModel(lager::cursor<KisProofingConfiguration> _data = lager::make_state(KisProofingConfiguration(), lager::automatic_tag{}));
    lager::cursor<KisProofingConfiguration> data;

    LAGER_QT_CURSOR(KoColor, warningColor); ///< Warning color for out-of-gamut checks.
    LAGER_QT_CURSOR(QString, proofingProfile);
    LAGER_QT_CURSOR(QString, proofingModel);
    LAGER_QT_CURSOR(QString, proofingDepth);

    LAGER_QT_CURSOR(bool, storeSoftproofingInsideImage);

    LAGER_QT_CURSOR(KoColorConversionTransformation::Intent, conversionIntent);
    LAGER_QT_CURSOR(bool, convBlackPointCompensation);

    enum DisplayTransformState {
        Monitor, ///< Use whichever settings are configured in the color management settings.
        Paper,   ///< Use absolute colorimetric, 0% adaptation, and no blackpoint compensation.
        Custom   ///< Let artists configure their own.
    };
    Q_ENUM(DisplayTransformState)
    LAGER_QT_CURSOR(DisplayTransformState, displayTransformState);

    // Following are all part of the second transform if it is custom.
    LAGER_QT_CURSOR(KoColorConversionTransformation::Intent, displayIntent);
    LAGER_QT_CURSOR(bool, dispBlackPointCompensation);
    LAGER_QT_CURSOR(int, adaptationState);
    /// this just returns the constant for the maximum adaptation range. Minimum is 0.
    LAGER_QT_CONST(int, adaptationRangeMax)
Q_SIGNALS:
    void modelChanged();
};

#endif // KISPROOFINGCONFIGMODEL_H
