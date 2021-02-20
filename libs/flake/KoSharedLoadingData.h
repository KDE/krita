/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2007 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSHAREDLOADINGDATA_H
#define KOSHAREDLOADINGDATA_H

#include "kritaflake_export.h"

/**
 * The KoSharedLoadingData class is used to share data between shapes during loading.
 * These data can be added to the KoShapeLoadingContext using KoShapeLoadingContext::addSharedData().
 * A different shape can then get the data from the context using KoShapeLoadingContext::sharedData().
 */
class KRITAFLAKE_EXPORT KoSharedLoadingData
{
public:
    KoSharedLoadingData();
    virtual ~KoSharedLoadingData();
};

#endif /* KOSHAREDLOADINGDATA_H */
