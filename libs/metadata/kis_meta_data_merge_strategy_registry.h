/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef _KIS_META_DATA_MERGE_STRATEGY_REGISTRY_H_
#define _KIS_META_DATA_MERGE_STRATEGY_REGISTRY_H_

#include <kritametadata_export.h>

#include "KoGenericRegistry.h"
#include "kis_meta_data_merge_strategy.h"

namespace KisMetaData
{

class KRITAMETADATA_EXPORT MergeStrategyRegistry : public KoGenericRegistry<const KisMetaData::MergeStrategy*>
{
public:
    MergeStrategyRegistry();
    ~MergeStrategyRegistry() override;
    static MergeStrategyRegistry* instance();
private:

    MergeStrategyRegistry(const MergeStrategyRegistry&);
    MergeStrategyRegistry& operator=(const MergeStrategyRegistry&);
};

}

#endif
