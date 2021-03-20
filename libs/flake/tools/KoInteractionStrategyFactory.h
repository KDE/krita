/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
