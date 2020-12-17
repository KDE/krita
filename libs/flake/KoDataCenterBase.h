/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006 Jan Hambrecht <jaham@gmx.net>
   SPDX-FileCopyrightText: 2006, 2009 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2008 C. Boemann <cbo@boemann.dk>
   SPDX-FileCopyrightText: 2008 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KODATACENTER_H
#define KODATACENTER_H

#include <QtGlobal>
#include "kritaflake_export.h"

class KoShapeSavingContext;
class KoStore;
class KoXmlWriter;

/**
 * The data center is for now just a sort of void pointer.
 * The data centers can be stuff like image collection, or stylemanager.
 * This abstraction is done so that shapes can get access to any possible type of data center.
 * The KoShapeControllerBase has a method that returns a map of data centers
 */
class KRITAFLAKE_EXPORT KoDataCenterBase
{
public:
    KoDataCenterBase();
    virtual ~KoDataCenterBase();

    /**
     * Load any remaining binary blobs needed
     * @returns false if an error occurred, which typically cancels the load.
     */
    virtual bool completeLoading(KoStore *store) = 0;

    /**
     * Save any remaining binary blobs
     * @returns false if an error occurred, which typically cancels the save.
     */
    virtual bool completeSaving(KoStore *store, KoXmlWriter *manifestWriter, KoShapeSavingContext *context) = 0;
};

#endif
