/* This file is part of the KDE project
 * Copyright (C) 2018 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 * Copyright (C) 2018 Eoin O'Neill <eoinoneill1991@gmail.com>
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

#ifndef KISKINECTICSCROLLER_H
#define KISKINECTICSCROLLER_H
#include <kritawidgetutils_export.h>
#include <QScroller>

class QAbstractScrollArea;

/* This is a convenience namespace for setting up global kinetic scrolling
 * with consistent settings across various UI elements within Krita. */

namespace KisKineticScroller {
KRITAWIDGETUTILS_EXPORT QScroller* createPreconfiguredScroller(QAbstractScrollArea *target);

KRITAWIDGETUTILS_EXPORT QScroller::ScrollerGestureType getConfiguredGestureType();

KRITAWIDGETUTILS_EXPORT void updateCursor(QWidget *source, QScroller::State state);
}

#endif // KISKINECTICSCROLLER_H
