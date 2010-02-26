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
#include <krreportdata.h>
#include <krobjectdata.h>
#include "krscriptlabel.h"
#include "krscriptfield.h"
#include "krscripttext.h"
#include "krscriptbarcode.h"
#include "krscriptimage.h"
#include "krscriptline.h"
#include "krscriptchart.h"
#include "krscriptsection.h"

namespace Scripting
{

Report::Report(KRReportData *r)
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
    QList<KRObjectData *>obs = m_reportData->objects();
    foreach(KRObjectData *o, obs) {
        if (o->entityName() == n) {
            switch (o->type()) {
            case KRObjectData::EntityLabel:
                return new Scripting::Label(o->toLabel());
                break;
            case KRObjectData::EntityField:
                return new Scripting::Field(o->toField());
                break;
            case KRObjectData::EntityText:
                return new Scripting::Text(o->toText());
                break;
            case KRObjectData::EntityBarcode:
                return new Scripting::Barcode(o->toBarcode());
                break;
            case KRObjectData::EntityLine:
                return new Scripting::Line(o->toLine());
                break;
            case KRObjectData::EntityChart:
                return new Scripting::Chart(o->toChart());
                break;
            case KRObjectData::EntityImage:
                return new Scripting::Image(o->toImage());
                break;
            default:
                return new QObject();
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
