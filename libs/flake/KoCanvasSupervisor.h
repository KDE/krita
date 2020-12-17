/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KOCANVASSUPERVISOR_H
#define KOCANVASSUPERVISOR_H

#include <QList>

#include "kritaflake_export.h"

class KoCanvasObserverBase;

/**
 * KoCanvasSupervisor is an abstract class that can return a
 * list of canvas observers, such as dock widgets.
 */
class KRITAFLAKE_EXPORT KoCanvasSupervisor
{
public:
    KoCanvasSupervisor();
    virtual ~KoCanvasSupervisor();
    virtual QList<KoCanvasObserverBase*> canvasObservers() const = 0;
};

#endif
