/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSHAREDSAVINGDATA_H
#define KOSHAREDSAVINGDATA_H

#include "kritaflake_export.h"

/**
 * The KoSharedSavingData class is used to share data between shapes during saving.
 * These data can be added to the KoShapeSavingContext using KoShapeSavingContext::addSharedData().
 * A different shape can then get the data from the context using KoShapeSavingContext::sharedData().
 */
class KRITAFLAKE_EXPORT KoSharedSavingData
{
public:
    KoSharedSavingData();
    virtual ~KoSharedSavingData();
};

#endif /* KOSHAREDSAVINGDATA_H */
