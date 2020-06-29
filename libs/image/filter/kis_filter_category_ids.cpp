/*
 *  Copyright (c) 2018 Victor Wåhlström <victor.wahlstrom@initiali.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_filter_category_ids.h"

#include <klocalizedstring.h>

const KoID FiltersCategoryAdjustId("adjust_filters", ki18nc("The category of color adjustment filters, like levels. Verb.", "Adjust"));
const KoID FiltersCategoryArtisticId("artistic_filters", ki18nc("The category of artistic filters, like raindrops. Adjective.", "Artistic"));
const KoID FiltersCategoryBlurId("blur_filters", ki18nc("The category of blur filters, like gaussian blur. Verb.", "Blur,"));
const KoID FiltersCategoryColorId("color_filters", ki18nc("The category of color transfer filters, like color to alpha. Noun.", "Colors"));
const KoID FiltersCategoryEdgeDetectionId("edge_filters", ki18nc("The category of edge detection filters. Noun.", "Edge Detection"));
const KoID FiltersCategoryEmbossId("emboss_filters", ki18nc("The category of emboss filters. Verb.", "Emboss"));
const KoID FiltersCategoryEnhanceId("enhance_filters", ki18nc("The category of enhancement filters, like sharpen. Verb.", "Enhance"));
const KoID FiltersCategoryMapId("map_filters", ki18nc("The category of mapping filters, like bump map or gradient filter map. Verb.", "Map"));
const KoID FiltersCategoryOtherId("other_filters", ki18nc("The category of filters that do not fit in a category. Noun.", "Other"));
