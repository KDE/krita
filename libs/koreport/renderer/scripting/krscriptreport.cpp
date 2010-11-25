/*
 * Kexi Report Plugin
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krscriptreport.h"
#include "krreportdata.h"
#include "KoReportItemBase.h"
#include "KoReportPluginManager.h"
#include "krscriptline.h"
#include "krscriptsection.h"
#include "KoReportItemLine.h"

namespace Scripting
{

Report::Report(KoReportReportData *r)
{
    m_reportData = r;
    m_scriptObject = 0;
}


Report::~Report()
{

}

QString Report::title()
{
    return m_reportData->m_title;
}

QString Report::name()
{
    return m_reportData->name();
}

QString Report::recordSource()
{
    return m_reportData->query();
}

QObject* Report::objectByName(const QString &n)
{
    QList<KoReportItemBase *>obs = m_reportData->objects();
    foreach(KoReportItemBase *o, obs) {
        if (o->entityName() == n) {
                    
            if (o->typeName() == "report:line") {
                        return new Scripting::Line(dynamic_cast<KoReportItemLine*>(o));
            }
            else {
                KoReportPluginManager* manager = KoReportPluginManager::self();
                KoReportPluginInterface *plugin = manager->plugin(o->typeName());
                if (plugin) {
                    QObject *obj = plugin->createScriptInstance(o);
                    if (obj) {
                        return obj;
                    }
                }
                else {
                    kDebug() << "Encountered unknown node while parsing section: " << o->typeName();
                }
            }
        }
    }
    return 0;
}

QObject* Report::sectionByName(const QString &n)
{
    KRSectionData *sec = m_reportData->section(n);
    if (sec) {
        return new Scripting::Section(sec);
    } else {
        return new QObject();
    }
}

void Report::initialize(Kross::Object::Ptr ptr)
{
    m_scriptObject = ptr;
}

void Report::eventOnOpen()
{
    if (m_scriptObject)
        m_scriptObject->callMethod("OnOpen");
}

void Report::eventOnComplete()
{
    if (m_scriptObject)
        m_scriptObject->callMethod("OnComplete");
}

void Report::eventOnNewPage()
{
    if (m_scriptObject)
        m_scriptObject->callMethod("OnNewPage");
}

}
