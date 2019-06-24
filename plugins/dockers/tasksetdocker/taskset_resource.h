/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#ifndef TASKSET_RESOURCE_H
#define TASKSET_RESOURCE_H

#include <KoResource.h>
#include <QStringList>


class TasksetResource : public KoResource
{

public:
    TasksetResource(const QString& filename);
    ~TasksetResource() override;

    bool load() override;
    bool loadFromDevice(QIODevice *dev) override;
    bool save() override;
    bool saveToDevice(QIODevice* dev) const override;

    QString defaultFileExtension() const override;

    void setActionList(const QStringList actions);
    QStringList actionList();

private:

    QStringList m_actions;
};

typedef QSharedPointer<TasksetResource> TasksetResourceSP;

#endif // TASKSET_RESOURCE_H
