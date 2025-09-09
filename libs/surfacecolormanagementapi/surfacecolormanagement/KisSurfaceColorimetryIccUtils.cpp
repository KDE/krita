/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSurfaceColorimetryIccUtils.h"

#include <QDebug>

namespace KisSurfaceColorimetry {

ColorPrimaries namedPrimariesToPigmentPrimaries(NamedPrimaries primaries)
{
    switch (primaries) {
        case NamedPrimaries::primaries_srgb:
            return PRIMARIES_ITU_R_BT_709_5;
        case NamedPrimaries::primaries_bt2020:
            return PRIMARIES_ITU_R_BT_2020_2_AND_2100_0;
        case NamedPrimaries::primaries_dci_p3:
            return PRIMARIES_SMPTE_RP_431_2;
        case NamedPrimaries::primaries_display_p3:
            return PRIMARIES_SMPTE_EG_432_1;
        case NamedPrimaries::primaries_adobe_rgb:
            return PRIMARIES_ADOBE_RGB_1998;
        default:
            return PRIMARIES_UNSPECIFIED;
    }
}

TransferCharacteristics namedTransferFunctionToPigmentTransferFunction(NamedTransferFunction transfer)
{
    switch (transfer) {
        case NamedTransferFunction::transfer_function_bt1886:
            return TRC_ITU_R_BT_709_5;
        case NamedTransferFunction::transfer_function_gamma22:
            return TRC_ITU_R_BT_470_6_SYSTEM_M;
        case NamedTransferFunction::transfer_function_gamma28:
            return TRC_ITU_R_BT_470_6_SYSTEM_B_G;
        case NamedTransferFunction::transfer_function_ext_linear:
            return TRC_LINEAR;
        case NamedTransferFunction::transfer_function_srgb:
            return TRC_IEC_61966_2_1;
        case NamedTransferFunction::transfer_function_ext_srgb:
            return TRC_UNSPECIFIED; // unsupported!
        case NamedTransferFunction::transfer_function_st2084_pq:
            return TRC_ITU_R_BT_2100_0_PQ;
        case NamedTransferFunction::transfer_function_st428:
            return TRC_SMPTE_ST_428_1;
        default:
            return TRC_UNSPECIFIED;
    }
}

PigmentProfileRequest colorSpaceToRequest(ColorSpace cs)
{
    PigmentProfileRequest request;

    if (std::holds_alternative<NamedPrimaries>(cs.primaries)) {
        request.colorPrimariesType =
            namedPrimariesToPigmentPrimaries(std::get<NamedPrimaries>(cs.primaries));
    } else if (std::holds_alternative<Colorimetry>(cs.primaries)) {
        auto colorimetry = std::get<Colorimetry>(cs.primaries);
        request.colorants = {
            colorimetry.white().toxy().x, colorimetry.white().toxy().y,
            colorimetry.red().toxy().x, colorimetry.red().toxy().y,
            colorimetry.green().toxy().x, colorimetry.green().toxy().y,
            colorimetry.blue().toxy().x, colorimetry.blue().toxy().y,
        };
    }

    if (std::holds_alternative<NamedTransferFunction>(cs.transferFunction)) {
        request.transferFunction =
            namedTransferFunctionToPigmentTransferFunction(std::get<NamedTransferFunction>(cs.transferFunction));
    } else if (std::holds_alternative<uint32_t>(cs.transferFunction)) {
        auto gamma = std::get<uint32_t>(cs.transferFunction); // gamma * 10000
        if (gamma == 10000) {
            request.transferFunction = TRC_LINEAR;
        } else if (gamma == 18000) {
            request.transferFunction = TRC_GAMMA_1_8;
        } else if (gamma == 22000) {
            request.transferFunction = TRC_ITU_R_BT_470_6_SYSTEM_M;
        } else if (gamma == 24000) {
            request.transferFunction = TRC_GAMMA_2_4;
        } else if (gamma == 28000) {
            request.transferFunction = TRC_ITU_R_BT_470_6_SYSTEM_B_G;
        }
    }

    // any pq-space that is not bt2020pq is considered unsupported

    if (request.transferFunction == TRC_ITU_R_BT_2100_0_PQ &&
        request.colorPrimariesType != PRIMARIES_ITU_R_BT_2020_2_AND_2100_0) {

        request.transferFunction = TRC_UNSPECIFIED;
        request.colorants.clear();
    }

    return request;
}

} // namespace KisSurfaceColorimetry
