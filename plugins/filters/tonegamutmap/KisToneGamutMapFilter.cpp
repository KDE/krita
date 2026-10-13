/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-only
 */
#include "KisToneGamutMapFilter.h"

#include <kis_filter_category_ids.h>
#include <filter/kis_filter_registry.h>
#include <kis_filter_configuration.h>
#include <KisGlobalResourcesInterface.h>

#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <kis_color_space_selector.h>

#include <KoCompositeColorTransformation.h>
#include <KoFallBackColorTransformation.h>

#include <kpluginfactory.h>
#include <klocalizedstring.h>
#include <KoGamutMappingUtils.h>


#include "ui_kistonegamutmapwidget.h"

const QString MAPPING_MODEL = "mapping_model"; ///< Enum, model in which the gamut or tone mapping is done.
const QString TONE_MAP_ENABLED = "toneMap/Enabled"; ///< bool, whether to do tone mapping.
const QString TONE_MAP_MAX_WHITE = "toneMap/reinhard/MaxWhite"; ///< double, the maximum white that needs to be tonemapped.
const QString GAMUT_MAP_ENABLED = "gamutMap/Enabled"; ///< bool, whether to do gamt mapping.
const QString GAMUT_MAP_TARGET_PROFILE = "gamutMap/TargetProfile"; ///< QString, name of the color profile that is the target gamut.
const QString GAMUT_MAP_METHOD = "gamutMap/method"; ///String, indicates whether to use the cusp map or the MINDE method.

const QString TONE_MAP_TYPE = "toneMap/CurveType";

const QString TONE_PIECE_WISE_TOE_STRENGTH = "toneMap/pieceWiseFilmic/toeStrength";
const QString TONE_PIECE_WISE_TOE_LENGTH = "toneMap/pieceWiseFilmic/toeLength";
const QString TONE_PIECE_WISE_SHOULDER_STRENGTH = "toneMap/pieceWiseFilmic/shoulderStrength";
const QString TONE_PIECE_WISE_SHOULDER_LENGTH = "toneMap/pieceWiseFilmic/shoulderLength";
const QString TONE_PIECE_WISE_SHOULDER_ANGLE = "toneMap/pieceWiseFilmic/shoulderAngle";

K_PLUGIN_FACTORY_WITH_JSON(KisToneGamutMapFilterFactory, "kritatonegamutmapfilter.json", registerPlugin<KisToneGamutMapFilterPlugin>();)

KisToneGamutMapFilterPlugin::KisToneGamutMapFilterPlugin(QObject *parent, const QVariantList &): QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KisToneGamutMapFilter()));
}

KisToneGamutMapFilterPlugin::~KisToneGamutMapFilterPlugin()
{

}

KisToneGamutMapFilter::KisToneGamutMapFilter()
    : KisColorTransformationFilter(id(), FiltersCategoryColorId, i18n("Tone/Gamut Map..."))
{
    setSupportsThreading(true);

    /// While as a colortransformation this filter supports threading,
    /// Adjustment layers and painting are disabled for two reasons:
    ///
    /// 1. There's a ton of different tone and gamut mapping algorithms.
    /// This way, we avoid being tied to a specific one, and can experiment.
    /// 2. Some of these implementations are very slow, and thus not suited
    /// for dynamic updating.
    setSupportsAdjustmentLayers(false);
    setSupportsPainting(false);
}

KoColorTransformation *KisToneGamutMapFilter::createTransformation(const KoColorSpace *cs, const KisFilterConfigurationSP config) const
{
    const KoColorSpace *mappingCs = KoColorSpaceRegistry::instance()->colorSpace(LABAColorModelID.id(), Float32BitsColorDepthID.id(), "");
    if (config->getProperty(MAPPING_MODEL).toInt() != KoGamutMappingUtils::LAB) {
        mappingCs = KoColorSpaceRegistry::instance()->colorSpace(XYZAColorModelID.id(), Float32BitsColorDepthID.id(), "");
    }
    QVector<KoColorTransformation*> allTransforms;

    if (config->getProperty(TONE_MAP_ENABLED).toBool()) {
        QHash<QString, QVariant> params;
        params["CurveType"] = config->getProperty(TONE_MAP_TYPE);

        params["maxWhite"] = config->getProperty(TONE_MAP_MAX_WHITE);

        params["toeStrength"] = config->getProperty(TONE_PIECE_WISE_TOE_STRENGTH);
        params["toeLength"] = config->getProperty(TONE_PIECE_WISE_TOE_LENGTH);
        params["shoulderStrength"] = config->getProperty(TONE_PIECE_WISE_SHOULDER_STRENGTH);
        params["shoulderLength"] = config->getProperty(TONE_PIECE_WISE_SHOULDER_LENGTH);
        params["shoulderAngle"] = config->getProperty(TONE_PIECE_WISE_SHOULDER_ANGLE);
        KoColorTransformation *toneMap = mappingCs->createColorTransformation("tonemap_adjustment", params);
        allTransforms << toneMap;
    }

    if (config->getProperty(GAMUT_MAP_ENABLED).toBool()) {
        QHash<QString, QVariant> params;
        params["targetColorProfileName"] = config->getProperty(GAMUT_MAP_TARGET_PROFILE);

        const KoColorProfile *targetProfile = KoColorSpaceRegistry::instance()->profileByName(config->getProperty(GAMUT_MAP_TARGET_PROFILE).toString());
        if (targetProfile && config->getProperty(GAMUT_MAP_METHOD).toString() == "CUSP_MAP") {
            QVariantMap cuspDescription;
            QMap<float, QPointF> tempCuspDescription = KoGamutMappingUtils::getGamutCuspDescription(targetProfile, KoGamutMappingUtils::MappingModel(config->getProperty(MAPPING_MODEL).toInt()));
            if (!tempCuspDescription.isEmpty()) {
                Q_FOREACH(const float key, tempCuspDescription.keys()) {
                    cuspDescription.insert(QString::number(key), tempCuspDescription.value(key));
                }
                params["cuspDescription"] = cuspDescription;
            }
        }

        params["mappingModel"] = config->getProperty(MAPPING_MODEL);
        KoColorTransformation *gamutMap = mappingCs->createColorTransformation("gamutmap_clip_adjustment", params);
        allTransforms << gamutMap;
    }

    if (allTransforms.isEmpty()) return nullptr;

    return new KoFallBackColorTransformation(cs, mappingCs, KoCompositeColorTransformation::createOptimizedCompositeTransform(allTransforms));
}

KisFilterConfigurationSP KisToneGamutMapFilter::defaultConfiguration(KisResourcesInterfaceSP resourcesInterface) const
{
    KisFilterConfigurationSP config = factoryConfiguration(resourcesInterface);
    config->setProperty(TONE_MAP_ENABLED, true);
    config->setProperty(TONE_MAP_MAX_WHITE, 1.0);

    config->setProperty(TONE_PIECE_WISE_TOE_STRENGTH, 0.5);
    config->setProperty(TONE_PIECE_WISE_TOE_LENGTH, 0.5);
    config->setProperty(TONE_PIECE_WISE_SHOULDER_STRENGTH, 0.5);
    config->setProperty(TONE_PIECE_WISE_SHOULDER_LENGTH, 2.0);
    config->setProperty(TONE_PIECE_WISE_SHOULDER_ANGLE, 1.0);

    config->setProperty(GAMUT_MAP_ENABLED, true);
    config->setProperty(GAMUT_MAP_METHOD, "CUSP_MAP");
    config->setProperty(GAMUT_MAP_TARGET_PROFILE, "sRGB-elle-V2-srgbtrc.icc");
    return config;
}

KisConfigWidget *KisToneGamutMapFilter::createConfigurationWidget(QWidget *parent, const KisPaintDeviceSP dev, bool useForMasks) const
{
    Q_UNUSED(useForMasks)
    return new KisToneGamutMapWidget(parent, dev);
}


KisToneGamutMapWidget::KisToneGamutMapWidget(QWidget *parent, KisPaintDeviceSP dev)
    : KisConfigWidget(parent)
    , ui(new Ui::KisToneGamutMapWidget)
{
    Q_ASSERT(dev);
    ui->setupUi(this);

    ui->targetGamut->showDepth(false);
    ui->targetGamut->showColorBrowserButton(false);

    ui->cmbMappingModel->addItem(i18n("LAB"), KoGamutMappingUtils::LAB);
    ui->cmbMappingModel->addItem(i18n("XYZ (ICtCp)"), KoGamutMappingUtils::ICtCp);
    ui->cmbMappingModel->addItem(i18n("XYZ (OkLAB)"), KoGamutMappingUtils::OkLAB);

    ui->cmbToneCurveType->addItem(i18n("Reinhard"), KoGamutMappingUtils::Reinhard);
    ui->cmbToneCurveType->addItem(i18n("Piecewise Filmic"), KoGamutMappingUtils::PiecewiseFilmic);

    ui->cmbGamutMapMethod->addItem(i18n("Use Chroma-Lightness Cusp Map"), "CUSP_MAP");
    ui->cmbGamutMapMethod->addItem(i18n("Minimum Delta E"), "MINDE");

    ui->cmbGamutMapMethod->setItemData(0, i18nc("@info:tooltip", "Use a chroma-lightness cusp map for a faster,"
                                                " if more inaccurate gamut mapping. Only works for RGB and CMYK profiles,"
                                                " will fall back on Minimum Delta E."), Qt::ToolTipRole);
    ui->cmbGamutMapMethod->setItemData(1, i18nc("@info:tooltip", "Will use the Minimum Delta E method,"
                                                " where colors are brought into gamut until they minimally different from the transformed color."
                                                " Slow but accurate."), Qt::ToolTipRole);

    connect(ui->cmbMappingModel, SIGNAL(activated(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->cmbToneCurveType, SIGNAL(activated(int)), ui->stkToneMap, SLOT(setCurrentIndex(int)));
    connect(ui->cmbToneCurveType, SIGNAL(activated(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->chkToneMap, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->spnMaxWhite, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->spnToeStrength, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnToeLength, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnShoulderStrength, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnShoulderLength, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->spnShoulderAngle, SIGNAL(valueChanged(double)), SIGNAL(sigConfigurationItemChanged()));

    connect(ui->chkGamutMap, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->targetGamut, SIGNAL(colorSpaceChanged(const KoColorSpace*)), SIGNAL(sigConfigurationItemChanged()));
    connect(ui->cmbGamutMapMethod, SIGNAL(activated(int)), SIGNAL(sigConfigurationItemChanged()));

    QStringList yAxisValues;
    for (int i = 0; i < 10; i++) {
        yAxisValues << QString::number(float(i)/10.0, 'f', 1);
    }
    ui->toneCurvePreview->setYAxisValues(yAxisValues, 64);

}

KisToneGamutMapWidget::~KisToneGamutMapWidget()
{
    delete ui;
}

void KisToneGamutMapWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    QVariant value;

    if (config->getProperty(MAPPING_MODEL, value)) {
        ui->cmbMappingModel->setCurrentIndex(ui->cmbMappingModel->findData(value));
    }
    if (config->getProperty(TONE_MAP_TYPE, value)) {
        ui->cmbToneCurveType->setCurrentIndex(ui->cmbToneCurveType->findData(value));
        ui->stkToneMap->setCurrentIndex(ui->cmbToneCurveType->currentIndex());
    }

    if (config->getProperty(TONE_MAP_ENABLED, value)) {
        ui->chkToneMap->setChecked(value.toBool());
    }
    if (config->getProperty(TONE_MAP_MAX_WHITE, value)) {
        ui->spnMaxWhite->setValue(value.toDouble());
    }
    if (config->getProperty(TONE_PIECE_WISE_TOE_STRENGTH, value)) {
        ui->spnToeStrength->setValue(value.toDouble());
    }
    if (config->getProperty(TONE_PIECE_WISE_TOE_LENGTH, value)) {
        ui->spnToeLength->setValue(value.toDouble());
    }
    if (config->getProperty(TONE_PIECE_WISE_SHOULDER_STRENGTH, value)) {
        ui->spnShoulderStrength->setValue(value.toDouble());
    }
    if (config->getProperty(TONE_PIECE_WISE_SHOULDER_LENGTH, value)) {
        ui->spnShoulderLength->setValue(value.toDouble());
    }
    if (config->getProperty(TONE_PIECE_WISE_SHOULDER_ANGLE, value)) {
        ui->spnShoulderAngle->setValue(value.toDouble());
    }


    if (config->getProperty(GAMUT_MAP_ENABLED, value)) {
        ui->chkGamutMap->setChecked(value.toBool());
    }
    if (config->getProperty(GAMUT_MAP_TARGET_PROFILE, value)) {
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(value.toString());
        if (profile) {
            ui->targetGamut->setCurrentColorSpace(
                KoColorSpaceRegistry::instance()->colorSpace(profile->colorModelID(),
                                                             Integer16BitsColorDepthID.id(),
                                                             profile));
        }
    }
    int idx = ui->cmbGamutMapMethod->findData(config->getProperty(GAMUT_MAP_METHOD));
    ui->cmbGamutMapMethod->setCurrentIndex(idx);
    Q_EMIT sigConfigurationItemChanged();
}

KisPropertiesConfigurationSP KisToneGamutMapWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("tone_gamut_map", 1, KisGlobalResourcesInterface::instance());
    config->setProperty(MAPPING_MODEL, ui->cmbMappingModel->currentData());

    config->setProperty(TONE_MAP_ENABLED, ui->chkToneMap->isChecked());
    config->setProperty(TONE_MAP_MAX_WHITE, ui->spnMaxWhite->value());

    config->setProperty(TONE_MAP_TYPE, ui->cmbToneCurveType->currentData());

    config->setProperty(TONE_PIECE_WISE_TOE_STRENGTH, ui->spnToeStrength->value());
    config->setProperty(TONE_PIECE_WISE_TOE_LENGTH, ui->spnToeLength->value());
    config->setProperty(TONE_PIECE_WISE_SHOULDER_STRENGTH, ui->spnShoulderStrength->value());
    config->setProperty(TONE_PIECE_WISE_SHOULDER_LENGTH, ui->spnShoulderLength->value());
    config->setProperty(TONE_PIECE_WISE_SHOULDER_ANGLE, ui->spnShoulderAngle->value());


    if (ui->cmbToneCurveType->currentData() == KoGamutMappingUtils::PiecewiseFilmic) {
        KoGamutMappingUtils::PiecewiseFilmicCurve curve
            = KoGamutMappingUtils::PiecewiseFilmicCurve::fromUserParams(ui->spnToeStrength->value(), ui->spnToeLength->value(), ui->spnShoulderStrength->value(), ui->spnShoulderLength->value(), ui->spnShoulderAngle->value());
        QPolygonF poly;
        const float white = 1.0/curve.inverseMaxWhite;
        QStringList xAxisValues;
        for (int x = 0; x < 20; x++) {
            const float val = (float(x)/19.0)*white;
            const float y = curve.evalCurve(val);
            if (x%2 == 0) {
                xAxisValues << QString::number(val, 'f', 1);

            }
            poly.append(QPointF(val/white, y));
        }
        ui->toneCurvePreview->setGreyscaleCurve(poly);
        ui->toneCurvePreview->setProfileDataAvailable(true);
        ui->toneCurvePreview->setToolTip(i18n("Current Maximum White: %1", QString::number(white, 'f', 2)));
        ui->toneCurvePreview->setXAxisValues(xAxisValues);
        ui->toneCurvePreview->update();
    } else {
        QPolygonF poly;
        const float white = ui->spnMaxWhite->value();
        QStringList xAxisValues;
        for (int x = 0; x < 20; x++) {
            const float val = (float(x)/19.0)*white;
            const float y = KoGamutMappingUtils::reinhardToneMapping(val, white);
            if (x%2 == 0) {
                xAxisValues << QString::number(val, 'f', 1);
            }
            poly.append(QPointF(val/white, y));
        }
        ui->toneCurvePreview->setGreyscaleCurve(poly);
        ui->toneCurvePreview->setProfileDataAvailable(true);
        ui->toneCurvePreview->setToolTip(i18n("Current Maximum White: %1", QString::number(white, 'f', 2)));
        ui->toneCurvePreview->setXAxisValues(xAxisValues);
        ui->toneCurvePreview->update();
    }

    config->setProperty(GAMUT_MAP_ENABLED, ui->chkGamutMap->isChecked());
    if (ui->targetGamut->currentColorSpace()) {
        config->setProperty(GAMUT_MAP_TARGET_PROFILE, ui->targetGamut->currentColorSpace()->profile()->name());
    }
    config->setProperty(GAMUT_MAP_METHOD, ui->cmbGamutMapMethod->currentData());
    return config;
}


#include "KisToneGamutMapFilter.moc"
