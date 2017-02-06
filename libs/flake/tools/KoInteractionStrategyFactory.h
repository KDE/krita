/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOINTERACTIONSTRATEGYFACTORY_H
#define KOINTERACTIONSTRATEGYFACTORY_H

#include <QScopedPointer>
#include <QSharedPointer>
#include "kritaflake_export.h"

class QString;
class QPainter;
class KoInteractionStrategy;
class KoPointerEvent;
class KoViewConverter;

class KoInteractionStrategyFactory;
typedef QSharedPointer<KoInteractionStrategyFactory> KoInteractionStrategyFactorySP;

class KRITAFLAKE_EXPORT KoInteractionStrategyFactory
{
public:
    KoInteractionStrategyFactory(int priority, const QString &id);
    virtual ~KoInteractionStrategyFactory();

    QString id() const;
    int priority() const;

    virtual KoInteractionStrategy* createStrategy(KoPointerEvent *ev) = 0;
    virtual bool hoverEvent(KoPointerEvent *ev) = 0;
    virtual bool paintOnHover(QPainter &painter, const KoViewConverter &converter) = 0;
    virtual bool tryUseCustomCursor() = 0;

    static bool compareLess(KoInteractionStrategyFactorySP f1, KoInteractionStrategyFactorySP f2);

private:
    struct Private;
    QScopedPointer<Private> m_d;
};



#endif // KOINTERACTIONSTRATEGYFACTORY_H
