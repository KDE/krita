/* This file is part of the KDE project
   Copyright (C) 2017 Alexey Kapustin <akapust1n@mail.ru>


   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "kis_toolsinfosource.h"

#include <QMutexLocker>
#include <QSysInfo>
#include <QThread>
#include <QVariant>
#include <iostream>

using namespace KisUserFeedback;
using namespace KUserFeedback;

ToolsInfoSource::ToolsInfoSource()
    : AbstractDataSource(QStringLiteral("Tools"), Provider::DetailedSystemInformation)
{
}

QString ToolsInfoSource::description() const
{
    return QObject::tr("Inforamation about tools");
}

QVariant ToolsInfoSource::data()
{
    return m_tools;
}

void ToolsInfoSource::activateTool(QString toolName)
{
    QMutexLocker locker(&m_mutex);
    m_currentTools.insert(toolName, QTime::currentTime());
    std::cout<<"ACTIVATE TOOL "<<toolName.toStdString()<<std::endl;
}

void ToolsInfoSource::deactivateTool(QString toolName)
{
    QMutexLocker locker(&m_mutex);
    QTime deactivateTime = QTime::currentTime();
    QTime activateTime = m_currentTools.value(toolName);
    m_currentTools.remove(toolName);
    int timeUse = activateTime.second() - deactivateTime.second();
    QVariantMap m;
    m.insert(QStringLiteral("toolname"), toolName);
    m.insert(QStringLiteral("timeUse"), timeUse);

    m_tools.push_back(m);
    std::cout<<"DE_ACTIVATE TOOL "<<toolName.toStdString()<<std::endl;

}
