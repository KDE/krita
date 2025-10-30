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
#include <KisDisplayConfig.h>
#include <KisWidgetConnectionUtils.h>

class KisDisplayColorConverter;
class KisDisplayConfig;

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
    ~KisProofingConfigModel();
    lager::cursor<KisProofingConfiguration> data;
    lager::state<KisDisplayConfig::Options, lager::automatic_tag> displayConfigOptions;

    using ColorSpaceId = std::tuple<QString,QString,QString>;

    LAGER_QT_CURSOR(KoColor, warningColor); ///< Warning color for out-of-gamut checks.

    // should be set atomically, hence "reader"; for writing use setProofingColorSpaceId
    LAGER_QT_READER(ColorSpaceId, proofingSpaceTuple);

    LAGER_QT_CURSOR(KoColorConversionTransformation::Intent, conversionIntent);
    LAGER_QT_READER(ComboBoxState, conversionIntentState);

    LAGER_QT_CURSOR(bool, convBlackPointCompensation);

    LAGER_QT_CURSOR(KisProofingConfiguration::DisplayTransformState, displayTransformMode);
    LAGER_QT_READER(ComboBoxState, displayTransformModeState);

    // Following are all part of the second transform if it is custom.
    LAGER_QT_READER(bool, enableCustomDisplayConfig);
    LAGER_QT_CURSOR(KoColorConversionTransformation::Intent, displayIntent);
    LAGER_QT_READER(KoColorConversionTransformation::Intent, effectiveDisplayIntent);
    LAGER_QT_READER(ComboBoxState, effectiveDisplayIntentState);

    LAGER_QT_CURSOR(bool, dispBlackPointCompensation);
    LAGER_QT_READER(CheckBoxState, effectiveDispBlackPointCompensationState);

    LAGER_QT_CURSOR(bool, adaptationSwitch);
    LAGER_QT_READER(CheckBoxState, adaptationSwitchState);

    void updateDisplayConfigOptions(KisDisplayConfig::Options options);
    void setProofingColorSpaceIdAtomic(const QString &model, const QString &depth, const QString &profile);

Q_SIGNALS:
    void modelChanged();

private:
    KisDisplayConfig::Options m_displayConfigOptions;
};

#endif // KISPROOFINGCONFIGMODEL_H
