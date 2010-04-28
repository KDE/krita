/*
 * Kexi Report Plugin
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
#include "krscripthandler.h"

#include "krscriptsection.h"
#include "krscriptdebug.h"
#include "krscriptreport.h"
#include "krscriptdraw.h"
#include "krscriptconstants.h"

#include <kdebug.h>
#include <kmessagebox.h>

#include <krsectiondata.h>
#include <KoReportItemBase.h>
#include <krreportdata.h>
#include <krdetailsectiondata.h>
#include <renderobjects.h>


KRScriptHandler::KRScriptHandler(const KoReportData* kodata, KoReportReportData* d)
{
    m_reportData = d;
    m_koreportData = kodata;

    m_action = 0;
    m_constants = 0;
    m_debug = 0;
    m_draw = 0;

    // Create the Kross::Action instance .
    m_action = new Kross::Action(this, "ReportScript");

    m_action->setInterpreter(d->interpreter());

    //Add constants object
    m_constants = new KRScriptConstants();
    m_action->addObject(m_constants, "constants");

    //A simple debug function to allow printing from functions
    m_debug = new KRScriptDebug();
    m_action->addObject(m_debug, "debug");

    //A simple drawing object
    m_draw = new KRScriptDraw();
    m_action->addObject(m_draw, "draw");

    //Add a general report object
    m_report = new Scripting::Report(m_reportData);

    //Add the sections
    QList<KRSectionData*> secs = m_reportData->sections();
    foreach(KRSectionData *sec, secs) {
        m_sectionMap[sec] = new Scripting::Section(sec);
        m_sectionMap[sec]->setParent(m_report);
        m_sectionMap[sec]->setObjectName(sec->name().replace('-', '_').remove("report:"));
        kDebug() << "Added" << m_sectionMap[sec]->objectName() << "to report" << m_reportData->name();
    }

    m_action->addObject(m_report, m_reportData->name());
    kDebug() << "Report name is" << m_reportData->name();

    QString code = m_koreportData->scriptCode(m_reportData->script(), m_reportData->interpreter());

    m_action->setCode(code.toLocal8Bit());

}

void KRScriptHandler::trigger()
{
    kDebug() << m_action->code();

    m_action->trigger();

    if (m_action->hadError()) {
        KMessageBox::error(0, m_action->errorMessage());
    } else {
        kDebug() << "Function Names:" << m_action->functionNames();
    }

    m_report->eventOnOpen();
}

KRScriptHandler::~KRScriptHandler()
{
    delete m_report;
    delete m_constants;
    delete m_debug;
    delete m_draw;
    delete m_action;
}

void KRScriptHandler::newPage()
{
    if (m_report) {
        m_report->eventOnNewPage();
    }
}

void KRScriptHandler::slotEnteredGroup(const QString &key, const QVariant &value)
{
    kDebug() << key << value;
    m_groups[key] = value;
    emit(groupChanged(where()));
}
void KRScriptHandler::slotExitedGroup(const QString &key, const QVariant &value)
{
    Q_UNUSED(value);
    kDebug() << key << value;
    m_groups.remove(key);
    emit(groupChanged(where()));
}

void KRScriptHandler::slotEnteredSection(KRSectionData *section, OROPage* cp, QPointF off)
{
    if (cp)
        m_draw->setPage(cp);
    m_draw->setOffset(off);

    Scripting::Section *ss = m_sectionMap[section];
    if (ss) {
        ss->eventOnRender();
    }
}

QVariant KRScriptHandler::evaluate(const QString &code)
{
    if (!m_action->hadError()) {
        return m_action->evaluate(code.toLocal8Bit());
    } else {
        return QVariant();
    }
}

void KRScriptHandler::displayErrors()
{
    if (m_action->hadError()) {
        KMessageBox::error(0, m_action->errorMessage());
    }
}

QString KRScriptHandler::where()
{
    QString w;
    QMap<QString, QVariant>::const_iterator i = m_groups.constBegin();
    while (i != m_groups.constEnd()) {
        w += '(' + i.key() + " = '" + i.value().toString() + "') AND ";
        ++i;
    }
    w = w.mid(0, w.length() - 4);
    kDebug() << w;
    return w;
}

void KRScriptHandler::registerScriptObject(QObject* obj, const QString& name)
{
    kDebug();
    if (m_action)
        m_action->addObject(obj, name);
}

