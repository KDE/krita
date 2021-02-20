/*
 *  SPDX-FileCopyrightText: 2015 Thorsten Zachmann <zachmann@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_COLOR_TRANSFORMATION_CONFIGURATION_H_
#define _KIS_COLOR_TRANSFORMATION_CONFIGURATION_H_

#include "kis_filter_configuration.h"
#include "kritaimage_export.h"

class KoColorSpace;
class KisColorTransformationFilter;

class KisColorTransformationConfiguration;
typedef KisSharedPtr<KisColorTransformationConfiguration> KisColorTransformationConfigurationSP;


class KRITAIMAGE_EXPORT KisColorTransformationConfiguration : public KisFilterConfiguration
{
public:
    KisColorTransformationConfiguration(const QString & name, qint32 version, KisResourcesInterfaceSP resourcesInterface);
    KisColorTransformationConfiguration(const KisColorTransformationConfiguration &rhs);
    ~KisColorTransformationConfiguration() override;

    KisFilterConfigurationSP clone() const override;

    KoColorTransformation *colorTransformation(const KoColorSpace *cs, const KisColorTransformationFilter *filter) const;

private:
    struct Private;
    Private* const d;
};

#endif /* _KIS_COLOR_TRANSFORMATION_CONFIGURATION_H_ */
