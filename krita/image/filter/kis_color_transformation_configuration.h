/*
 *  Copyright (c) 2015 Thorsten Zachmann <zachmann@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_COLOR_TRANSFORMATION_CONFIGURATION_H_
#define _KIS_COLOR_TRANSFORMATION_CONFIGURATION_H_

#include "filter/kis_filter_configuration.h"
#include "kritaimage_export.h"

class KoColorSpace;
class KisColorTransformationFilter;

class KRITAIMAGE_EXPORT KisColorTransformationConfiguration : public KisFilterConfiguration
{
public:
    KisColorTransformationConfiguration(const QString & name, qint32 version);
    virtual ~KisColorTransformationConfiguration();

    KoColorTransformation *colorTransformation(const KoColorSpace *cs, const KisColorTransformationFilter * filter) const;

private:
    struct Private;
    Private* const d;
};

#endif /* _KIS_COLOR_TRANSFORMATION_CONFIGURATION_H_ */
