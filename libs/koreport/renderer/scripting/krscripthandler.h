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
#ifndef KRSCRIPTHANDLER_H
#define KRSCRIPTHANDLER_H
#include <QObject>
#include <QString>
#include <krsectiondata.h>
#include <kross/core/action.h>
#include "krscriptconstants.h"
#include <kdeversion.h>
#include "KoReportData.h"

class KRScriptFunctions;
class KRScriptDebug;
class KRReportData;
class OROPage;
class KRScriptDraw;

namespace Scripting
{
class Report;
class Section;
}
class KRScriptHandler : public QObject
{
    Q_OBJECT
public:
    KRScriptHandler(const KoReportData *, KRReportData*);
    ~KRScriptHandler();

    QVariant evaluate(const QString&);
    void displayErrors();
    void registerScriptObject(QObject*, const QString&);
    void trigger();

public slots:

    void slotEnteredSection(KRSectionData*, OROPage*, QPointF);
    void slotEnteredGroup(const QString&, const QVariant&);
    void slotExitedGroup(const QString&, const QVariant&);
    void setPageNumber(int p) {
        m_constants->setPageNumber(p);
    }
    void setPageTotal(int t) {
        m_constants->setPageTotal(t);
    }
    void newPage();

signals:
    void groupChanged(const QString& whereCondition);    

private:
    KRScriptConstants *m_constants;
    KRScriptDebug *m_debug;
    KRScriptDraw *m_draw;

    Scripting::Report *m_report;

#if !KDE_IS_VERSION(4,2,88)
    QString fieldFunctions();
#endif

    const KoReportData *m_koreportData;

    QString m_source;
    KRReportData  *m_reportData;

    Kross::Action* m_action;

    QMap<QString, QVariant> m_groups;
    QMap<KRSectionData*, Scripting::Section*> m_sectionMap;
    QString where();
};

#endif

