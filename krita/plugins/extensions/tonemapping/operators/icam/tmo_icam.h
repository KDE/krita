/**
 * @file tmo_icam.h
 * @brief Tone map luminance channel using icam model
 */

#ifndef _tmo_icam_h_
#define _tmo_icam_h_

/*
 * @brief icam tone-mapping
 *
 * @param Y input luminance
 * @param L output tonemapped intensities
 * @param variance
 * @param variance2
 * @param D
 * @param prescaling
 * @param percentile
 * @param indep
 */

namespace icam
{

void tmo_icam(const pfs::Array2D *Y, pfs::Array2D *L,
              float variance, float variance2, float D, float prescaling, float percentile, bool indep);

}

#endif /* _tmo_icam_h_ */
