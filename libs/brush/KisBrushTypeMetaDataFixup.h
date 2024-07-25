/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHTYPEMETADATAFIXUP_H
#define KISBRUSHTYPEMETADATAFIXUP_H

#include "kritabrush_export.h"
#include "KisResourceLoaderRegistry.h"

class BRUSH_EXPORT KisBrushTypeMetaDataFixup : public KisResourceLoaderRegistry::ResourceCacheFixup
{
public:
    QStringList executeFix() override;
};

#endif // KISBRUSHTYPEMETADATAFIXUP_H
