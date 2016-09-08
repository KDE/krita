/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KO_RESOURCE_UPDATE_MEDIATOR_H
#define __KO_RESOURCE_UPDATE_MEDIATOR_H

#include <QScopedPointer>
#include <QSharedPointer>
#include "kritaflake_export.h"

/**
 * A special mediator class that connects the resource and the
 * resource manager.  The resource manager connects to a
 * sigResourceChanged() changed and when a resource changes, the
 * manager calls connectResource() for this resource. After that, the
 * mediator should notify the manager about every change that happens
 * to the resource by emitting the corresponding signal.
 *
 * There is only one mediator for one type (key) of the resource.
 */

class KRITAFLAKE_EXPORT KoResourceUpdateMediator : public QObject
{
    Q_OBJECT
public:
    KoResourceUpdateMediator(int key);
    virtual ~KoResourceUpdateMediator();

    int key() const;
    virtual void connectResource(QVariant sourceResource) = 0;

Q_SIGNALS:
    void sigResourceChanged(int key);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KoResourceUpdateMediator> KoResourceUpdateMediatorSP;

#endif /* __KO_RESOURCE_UPDATE_MEDIATOR_H */
