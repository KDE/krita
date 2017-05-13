/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "surveyinfo.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QJsonObject>
#endif
#include <QSharedData>
#include <QUrl>
#include <QUuid>

using namespace UserFeedback;

class UserFeedback::SurveyInfoData : public QSharedData
{
public:
    QUuid uuid;
    QUrl url;
    QString target;
};


SurveyInfo::SurveyInfo() : d (new SurveyInfoData)
{
}

SurveyInfo::SurveyInfo(const SurveyInfo &other) :
    d(other.d)
{
}

SurveyInfo::~SurveyInfo()
{
}

SurveyInfo& SurveyInfo::operator=(const SurveyInfo &other)
{
    d = other.d;
    return *this;
}

bool SurveyInfo::isValid() const
{
    return !d->uuid.isNull() && d->url.isValid();
}

QUuid SurveyInfo::uuid() const
{
    return d->uuid;
}

void SurveyInfo::setUuid(const QUuid &id)
{
    d->uuid = id;
}

QUrl SurveyInfo::url() const
{
    return d->url;
}

void SurveyInfo::setUrl(const QUrl& url)
{
    d->url = url;
}

QString SurveyInfo::target() const
{
    return d->target;
}

void SurveyInfo::setTarget(const QString &target)
{
    d->target = target;
}

SurveyInfo SurveyInfo::fromJson(const QJsonObject& obj)
{
    SurveyInfo s;
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    s.setUuid(obj.value(QLatin1String("uuid")).toString());
    s.setUrl(QUrl(obj.value(QLatin1String("url")).toString()));
    s.setTarget(obj.value(QLatin1String("target")).toString());
#endif
    return s;
}
