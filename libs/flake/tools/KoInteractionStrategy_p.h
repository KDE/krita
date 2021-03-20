/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006-2009 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KoInteractionStrategyPrivate_H
#define KoInteractionStrategyPrivate_H

#include <KoToolBase.h>

class KoInteractionStrategyPrivate
{
public:
    explicit KoInteractionStrategyPrivate(KoToolBase *t)
        : tool(t)
    {
    }
    ~KoInteractionStrategyPrivate()
    {
        tool->setStatusText(QString());
    }

    KoToolBase *tool; ///< the KoToolBase instance that controls this strategy.
};

#endif
