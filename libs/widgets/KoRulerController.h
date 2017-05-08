/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KORULERCONTROLLER_H
#define KORULERCONTROLLER_H

#include <QObject>
#include "kritawidgets_export.h"

class KoRuler;
class KoCanvasResourceManager;

/**
 * This class combines text options with the KoRuler object.
 * Any usage of a horizontal ruler should consider using this class to show the
 * text indent and tabs on the ruler, and allow to edit them.
 * The code to do this is pretty trivial; just instantiate this class and you can
 * forget about it.  It'll do what you want.
 */
class KRITAWIDGETS_EXPORT KoRulerController : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     * @param horizontalRuler the ruler to monitor and update.
     *  Will also be used as QObject parent for memory management purposes.
     * @param crp the resource provider for the canvas this ruler and the text tool belong to.
     */
    KoRulerController(KoRuler *horizontalRuler, KoCanvasResourceManager *crp);
    ~KoRulerController() override;

private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void canvasResourceChanged(int))
    Q_PRIVATE_SLOT(d, void indentsChanged())
    Q_PRIVATE_SLOT(d, void tabChanged(int, KoRuler::Tab *tab))
    Q_PRIVATE_SLOT(d, void tabChangeInitiated())
};

#endif
