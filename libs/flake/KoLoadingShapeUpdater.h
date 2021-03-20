/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Thorsten Zachmann <zachmann@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOLOADINGSHAPEUPDATER_H
#define KOLOADINGSHAPEUPDATER_H

#include "kritaflake_export.h"

class KoShape;

/**
 * Reimplement this class when you depend on a shape during loading that is
 * not yet loaded.
 *
 * As soon as the shape you depend on is loaded the method update is called.
 * Then you can setup the data you need.
 *
 * @see KoConnectionShape
 * @see KoShapeLoadingContext::updateShape
 */
class KRITAFLAKE_EXPORT KoLoadingShapeUpdater
{
public:
    KoLoadingShapeUpdater();
    virtual ~KoLoadingShapeUpdater();

    /**
     * This function is called as soon as shape is loaded.
     *
     * @param shape The shape that just got loaded.
     */
    virtual void update(KoShape *shape) = 0;
};

#endif /* KOLOADINGSHAPEUPDATER_H */
