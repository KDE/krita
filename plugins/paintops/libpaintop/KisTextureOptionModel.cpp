/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisTextureOptionModel.h"

#include <kis_assert.h>
#include <KisLager.h>

#include <KoResourceLoadResult.h>
#include <KisResourcesInterface.h>

namespace {

auto patternResourceLens = [] (KisResourcesInterfaceSP resourcesInterface) {
    return lager::lenses::getset(
        [resourcesInterface] (const KisEmbeddedTextureData &data) {
            KoResourceLoadResult result = data.loadLinkedPattern(resourcesInterface);

            KoResourceSP resource;

            if (result.type() != KoResourceLoadResult::ExistingResource) {
                if (!data.isNull()) {
                    qWarning() << "Failed to load embedded resource" << result.signature();
                }
                resource = resourcesInterface->source(ResourceType::Patterns).fallbackResource();
            } else {
                resource = result.resource();
            }

            return resource;
        },
        [] (const KisEmbeddedTextureData &, KoResourceSP resource) {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resource, KisEmbeddedTextureData());
            return KisEmbeddedTextureData::fromPattern(resource.dynamicCast<KoPattern>());
        });
};

}

KisTextureOptionModel::KisTextureOptionModel(lager::cursor<KisTextureOptionData> _optionData, KisResourcesInterfaceSP resourcesInterface)
    : optionData(_optionData)
    , LAGER_QT(isEnabled) {optionData[&KisTextureOptionData::isEnabled]}
    , LAGER_QT(textureResource) {optionData[&KisTextureOptionData::textureData].zoom(patternResourceLens(resourcesInterface))}
    , LAGER_QT(scale) {optionData[&KisTextureOptionData::scale]}
    , LAGER_QT(brightness) {optionData[&KisTextureOptionData::brightness]}
    , LAGER_QT(contrast) {optionData[&KisTextureOptionData::contrast]}
    , LAGER_QT(neutralPoint) {optionData[&KisTextureOptionData::neutralPoint]}
    , LAGER_QT(offsetX) {optionData[&KisTextureOptionData::offsetX]}
    , LAGER_QT(offsetY) {optionData[&KisTextureOptionData::offsetY]}
    , LAGER_QT(maximumOffsetX) {optionData[&KisTextureOptionData::maximumOffsetX]}
    , LAGER_QT(maximumOffsetY) {optionData[&KisTextureOptionData::maximumOffsetY]}
    , LAGER_QT(isRandomOffsetX) {optionData[&KisTextureOptionData::isRandomOffsetX]}
    , LAGER_QT(isRandomOffsetY) {optionData[&KisTextureOptionData::isRandomOffsetY]}
    , LAGER_QT(texturingMode) {optionData[&KisTextureOptionData::texturingMode]
            .zoom(kislager::lenses::do_static_cast<KisTextureOptionData::TexturingMode, int>)}
    , LAGER_QT(cutOffPolicy) {optionData[&KisTextureOptionData::cutOffPolicy]}
    , LAGER_QT(cutOffLeftNormalized) {optionData[&KisTextureOptionData::cutOffLeft]
            .zoom(kislager::lenses::scale_int_to_real(1.0/255.0))}
    , LAGER_QT(cutOffRightNormalized) {optionData[&KisTextureOptionData::cutOffRight]
            .zoom(kislager::lenses::scale_int_to_real(1.0/255.0))}
    , LAGER_QT(invert) {optionData[&KisTextureOptionData::invert]}
{
    LAGER_QT(textureResource).bind(std::bind(&KisTextureOptionModel::updateOffsetLimits, this, std::placeholders::_1));
}

KisTextureOptionData KisTextureOptionModel::bakedOptionData() const
{
    KisTextureOptionData data = optionData.get();
    data.textureData = KisEmbeddedTextureData::fromPattern(textureResource().dynamicCast<KoPattern>());
    return data;
}

void KisTextureOptionModel::updateOffsetLimits(KoResourceSP resource)
{
    KoPatternSP pattern = resource.dynamicCast<KoPattern>();
    KIS_SAFE_ASSERT_RECOVER_RETURN(pattern);

    setmaximumOffsetX(pattern->width() / 2);
    setmaximumOffsetY(pattern->height() / 2);
}
