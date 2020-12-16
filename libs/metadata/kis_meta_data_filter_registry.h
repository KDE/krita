/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_FILTER_REGISTRY_H_
#define _KIS_META_DATA_FILTER_REGISTRY_H_

#include <kritametadata_export.h>

#include "KoGenericRegistry.h"
#include "kis_meta_data_filter.h"

namespace KisMetaData
{

class KRITAMETADATA_EXPORT FilterRegistry : public KoGenericRegistry<const KisMetaData::Filter*>
{
public:
    FilterRegistry();
    ~FilterRegistry() override;
    static FilterRegistry* instance();
private:
    FilterRegistry(const FilterRegistry&);
    FilterRegistry& operator=(const FilterRegistry&);
};

}

#endif
