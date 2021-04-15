/*
 *  SPDX-FileCopyrightText: 2021 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOCOLORPROFILECONSTANTS_H
#define KOCOLORPROFILECONSTANTS_H


/**
 * @brief The colorPrimaries enum
 * Enum of colorants, follows ITU H.273 for values 0 to 255,
 * and has extra known quantities starting at 256.
 *
 * This is used for profile generation and file encoding.
 */
enum ColorPrimaries {
    //0 is resevered
    PRIMARIES_ITU_R_BT_709_5 = 1, // sRGB, rec 709
    PRIMARIES_UNSPECIFIED = 2,
    //3 is reserved
    PRIMARIES_ITU_R_BT_470_6_SYSTEM_M = 4,
    PRIMARIES_ITU_R_BT_470_6_SYSTEM_B_G,
    PRIMARIES_ITU_R_BT_601_6,
    PRIMARIES_SMPTE_240M, // Old HDTv standard.
    PRIMARIES_GENERIC_FILM, // H.273 says 'color filters using illuminant C'
    PRIMARIES_ITU_R_BT_2020_2_AND_2100_0,
    PRIMARIES_SMPTE_ST_428_1, // XYZ
    PRIMARIES_SMPTE_RP_431_2, //DCI P3, or Digital Cinema Projector
    PRIMARIES_SMPTE_EG_432_1, //Display P3
    // 13-21 are reserved.
    PRIMARIES_EBU_Tech_3213_E = 22,
    // 23-255 are reserved by ITU/ISO
    PRIMARIES_ADOBE_RGB_1998 = 256,
    PRIMARIES_PROPHOTO
};

/**
 * @brief The transferCharacteristics enum
 * Enum of transfer characteristics, follows ITU H.273 for values 0 to 255,
 * and has extra known quantities starting at 256.
 *
 * This is used for profile generation and file encoding.
 */
enum TransferCharacteristics {
    // 0 is reserved.
    TRC_ITU_R_BT_709_5 = 1, //Different from sRGB, officially HDTV
    TRC_UNSPECIFIED, // According to H.273: Image characteristics are unknown or are determined by the application.
    // 3 is reserved.
    TRC_ITU_R_BT_470_6_SYSTEM_M = 4, //Assumed gamma 2.2
    TRC_ITU_R_BT_470_6_SYSTEM_B_G, // Assume gamma 2.8
    TRC_ITU_R_BT_601_6, //SDTV
    TRC_SMPTE_240M,
    TRC_LINEAR,
    TRC_LOGARITHMIC_100,
    TRC_LOGARITHMIC_100_sqrt10,
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
    TRC_PROPHOTO, //Gamma 1.8 unless under 16/512
    TRC_A98 //Gamma of 256/563
};

#endif // KOCOLORPROFILECONSTANTS_H

