/* This file is part of the KDE libraries
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef KOSTANDARDACTION_H
#define KOSTANDARDACTION_H

#include "komain_export.h"

class QObject;
class KAction;
class KToggleAction;

/**
 * Convenience methods to access all standard KOffice actions.
 * See KStandardAction for usage details.
 */
namespace KoStandardAction
{
  /**
   * The standard actions.
   */
  enum StandardAction {
    ActionNone,

    ShowGuides
  };

  /**
   * Creates an action corresponding to the
   * KoStandardAction::StandardAction enum.
   */
  KOMAIN_EXPORT KAction* create(StandardAction id, const QObject *receiver, const char *slot, QObject *parent);

  /**
   * This will return the internal name of a given standard action.
   */
  KOMAIN_EXPORT const char* name(StandardAction id);

  /**
   * Show or hide guide lines
   */
  KOMAIN_EXPORT KToggleAction *showGuides(const QObject *receiver, const char *slot, QObject *parent);
}

#endif // KSTDACTION_H

