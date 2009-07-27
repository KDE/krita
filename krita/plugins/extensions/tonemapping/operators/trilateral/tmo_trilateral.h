/**
 * Copyright (c) 2007 Shaine Joseph
 * @file tmo_trilateral.h
 * @brief Tone map luminance channel using trilateral filter model
 */

#ifndef _tmo_trilateral_h_
#define _tmo_trilateral_h_

#include "pfs.h"

/*
 * @brief trilateral filter tone-mapping
 *
 * @param Y input luminance
 * @param L output tonemapped intensities
 * @param contrast
 * @param sigma
 * @param shift
 * @param saturation
 */

void tmo_trilateral(const pfs::Array2D *Y, pfs::Array2D *L,
                    float contrast, float sigma, float shift, float saturation);

#endif /* _tmo_trilateral_h_ */
