/*
 *  Copyright (c) 2009,2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_action_creator_factory.h"

#include <QString>

struct KisRecordedActionCreatorFactory::Private {
  QString id;
  QString name;
};

KisRecordedActionCreatorFactory::KisRecordedActionCreatorFactory(const QString& _id, const QString& _name) : d(new Private)
{
  d->id = _id;
  d->name = _name;
}

KisRecordedActionCreatorFactory::~KisRecordedActionCreatorFactory()
{
    delete d;
}

QString KisRecordedActionCreatorFactory::id() const
{
    return d->id;
}

QString KisRecordedActionCreatorFactory::name() const
{
    return d->name;
}

KisRecordedActionCreator* KisRecordedActionCreatorFactory::createCreator(QWidget* /*parent*/) const
{
  Q_ASSERT(requireCreator() == false);
  return 0;
}

KisRecordedAction* KisRecordedActionCreatorFactory::createAction() const
{
  Q_ASSERT(requireCreator() == true);
  return 0;
}
