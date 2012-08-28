/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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

#include "KoRdfCalendarEvent.h"
#include "KoDocumentRdf.h"
#include "KoRdfSemanticItem_p.h"
#include "KoTextRdfCore.h"
#include "KoRdfCalendarEventTreeWidgetItem.h"
#include <QUuid>
#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>
#include <KMessageBox>
#include "ksystemtimezone.h"

using namespace Soprano;

KoRdfCalendarEvent::KoRdfCalendarEvent(QObject *parent, const KoDocumentRdf *m_rdf)
    : KoRdfSemanticItem(m_rdf, parent)
{
    m_startTimespec = KSystemTimeZones::local();
    m_endTimespec = KSystemTimeZones::local();
}

static KDateTime VEventDateTimeToKDateTime(const QString &s, KDateTime::Spec &tz)
{
    kDebug(30015) << "top... tz.offset:" << tz.timeZone().currentOffset();

    if (s.endsWith('Z')) {
        tz = KSystemTimeZones::zone("UTC");
        kDebug(30015) << "tz.offset:" << tz.timeZone().currentOffset();
        kDebug(30015) << "new date string:" << s;
    }

    KDateTime ret = KDateTime::fromString(s, "yyyyMMddTHHmmss");
    if (!ret.isValid()) {
        // "2003-01-08T13:00:00"
        kDebug(30015) << "parsing dateThh:mm format...from input:" << s;
        ret = KDateTime::fromString(s, KDateTime::ISODate);
    }

    //
    // Parsed as UTC, must now adjust for given timezone
    //
    if (ret.isValid() && tz.timeZone().currentOffset()) {
        ret.setTimeSpec(tz);
    }

    //
    // convert to local tz for ease of editing.
    //
    ret = ret.toLocalZone();
    tz = KSystemTimeZones::local();
    kDebug(30015) << "date string:" << s << "\n"
        << " is valid:" << ret.isValid() << "\n"
        << " parsed:" << ret << "\n"
        << " time.tz.offset:" << ret.timeZone().currentOffset()
        << " tz.offset:" << tz.timeZone().currentOffset();
    return ret;
}

static KTimeZone toKTimeZone(Soprano::Node n)
{
    QString dt = n.dataType().toString();
    dt.remove(QRegExp("#tz$"));
    int idx = dt.lastIndexOf('/');
    if (idx > 0) {
        idx = dt.lastIndexOf('/', idx - 1);
    }
    if (idx > 0) {
        dt = dt.mid(idx + 1);
    }
    KTimeZone kt = KSystemTimeZones::zone(dt);
    kDebug(30015) << "input:" << n.dataType().toString()
        << " output tz.valid:" << kt.isValid()
        << " timezone:" << dt;
    if (!kt.isValid()) {
        // UTC "Zulu" Time
        if (dt == "2001/XMLSchema#dateTime" && n.toString().endsWith('Z')) {
            kDebug(30015) << "input:" << n.dataType().toString()
            << " is UTC...";
            kt = KSystemTimeZones::zone("UTC");
        }
    }

    if (!kt.isValid()) {
        kt = KSystemTimeZones::zone("UTC");
    }
    return kt;
}

KoRdfCalendarEvent::KoRdfCalendarEvent(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
    : KoRdfSemanticItem(rdf, it, parent)
{
    m_location = KoTextRdfCore::optionalBindingAsString(it, "location");
    m_summary = KoTextRdfCore::optionalBindingAsString(it, "summary");
    m_uid = KoTextRdfCore::optionalBindingAsString(it, "uid");
    m_linkSubject = it.binding("ev");
    // floating time is the default
    m_startTimespec = KSystemTimeZones::local();
    m_endTimespec = KSystemTimeZones::local();
    // check for timezones in the type of each date-time binding.
    m_startTimespec = toKTimeZone(it.binding("dtstart"));
    m_endTimespec = toKTimeZone(it.binding("dtend"));
    m_dtstart = VEventDateTimeToKDateTime(it.binding("dtstart").toString(),
                   m_startTimespec);
    m_dtend = VEventDateTimeToKDateTime(it.binding("dtend").toString(),
                                           m_endTimespec);
    kDebug(30015) << "KoRdfCalendarEvent() start:" << m_dtstart
        << " end:" << m_dtend;
    kDebug(30015) << "KoRdfCalendarEvent() long:" << KoTextRdfCore::optionalBindingAsString(it, "long")
        << " lat:" << KoTextRdfCore::optionalBindingAsString(it, "lat");
    kDebug(30015) << "KoRdfCalendarEvent() context-direct:" << it.binding("graph").toString();
    kDebug(30015) << "KoRdfCalendarEvent() context():" << context().toString();
    kDebug(30015) << "m_startTimespec.offset:" << m_startTimespec.timeZone().currentOffset();
    kDebug(30015) << "dtstart:" << m_dtstart.date();
    kDebug(30015) << "dtstart:" << m_dtstart.time();
    kDebug(30015) << "dtend:" << m_dtend.date();
    kDebug(30015) << "dtend:" << m_dtend.time();
}

KoRdfCalendarEvent::~KoRdfCalendarEvent()
{
}

QWidget* KoRdfCalendarEvent::createEditor(QWidget *parent)
{
    QWidget *ret = new QWidget(parent);

    kDebug(30015) << "createEditor()";
    kDebug(30015) << "linkingSubject:" << linkingSubject().toString();
    kDebug(30015) << "context:" << context().toString();
    editWidget.setupUi(ret);
    editWidget.summary->setText(m_summary);
    editWidget.location->setText(m_location);
    enum {
        ColArea = 0,
        ColRegion,
        ColComment,
        ColCount
    };
    editWidget.tz->sortItems(ColRegion, Qt::AscendingOrder);
    editWidget.tz->setColumnHidden(ColComment, true);
    editWidget.tz->header()->resizeSections(QHeaderView::ResizeToContents);
    editWidget.tz->headerItem()->setText(ColArea,   i18n("Area"));
    editWidget.tz->headerItem()->setText(ColRegion, i18n("Region"));
    editWidget.startDate->setDate(m_dtstart.date());
    editWidget.endDate->setDate(m_dtend.date());
    editWidget.startTime->setTime(m_dtstart.time());
    editWidget.endTime->setTime(m_dtend.time());

    kDebug(30015) << "summary:" << m_summary;
    kDebug(30015) << "location:" << m_location;
    kDebug(30015) << "dtstart:" << m_dtstart.date();
    kDebug(30015) << "dtstart:" << m_dtstart.time();
    kDebug(30015) << "dtend:" << m_dtend.date();
    kDebug(30015) << "dtend:" << m_dtend.time();
    return ret;
}

void KoRdfCalendarEvent::updateFromEditorData()
{
    QString predBase = "http://www.w3.org/2002/12/cal/icaltzd#";
    if (!m_linkSubject.isValid()) {
        m_linkSubject = createNewUUIDNode();
    }
    if (m_uid.size() <= 0) {
        m_uid = QUuid::createUuid().toString();
    }
    kDebug(30015) << "KoRdfCalendarEvent::updateFromEditorData()";
    kDebug(30015) << "context:" << context().toString();
    kDebug(30015) << "Old summary:" << m_summary;
    kDebug(30015) << "New summary:" << editWidget.summary->text();
    setRdfType(predBase + "Vevent");
    updateTriple(m_summary,   editWidget.summary->text(),   predBase + "summary");
    updateTriple(m_location,  editWidget.location->text(),  predBase + "location");
    updateTriple(m_uid,       m_uid,                        predBase + "uid");
    KDateTime::Spec startTimespec = KSystemTimeZones::local();
    QStringList selection = editWidget.tz->selection();
    if (selection.size() > 0) {
        QString tzString = selection[0];
        kDebug(30015) << "explicit time zone selected... tzString:" << tzString;
        KTimeZone ktz = KSystemTimeZones::zone(tzString);
        startTimespec = KDateTime::Spec(ktz);
        kDebug(30015) << "explicit time zone selected...startTimespec:" << ktz.name();
    }
    KDateTime::Spec endTimespec = startTimespec;
    m_startTimespec = startTimespec;
    m_endTimespec   = endTimespec;
    KDateTime dtstart(editWidget.startDate->date(), editWidget.startTime->time(), m_startTimespec);
    KDateTime dtend(editWidget.endDate->date(),   editWidget.endTime->time(),   m_endTimespec);
    kDebug(30015) << "m_startTimespec.offset:" << m_startTimespec.timeZone().currentOffset();
    kDebug(30015) << "date:" << editWidget.startDate->date();
    kDebug(30015) << "time:" << editWidget.startTime->time();
    kDebug(30015) << "dtstart:" << dtstart;
    kDebug(30015) << "qdtstart:" << dtstart.dateTime();
    LiteralValue lv(dtstart.dateTime());
    Node n = Node::createLiteralNode(lv);
    kDebug(30015) << "soprano::node:" << n.toString();
    updateTriple(m_dtstart,   dtstart,  predBase + "dtstart");
    updateTriple(m_dtend,     dtend,    predBase + "dtend");
    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
    }
}

KoRdfSemanticTreeWidgetItem *KoRdfCalendarEvent::createQTreeWidgetItem(QTreeWidgetItem* parent)
{
    KoRdfCalendarEventTreeWidgetItem *item  =
        new KoRdfCalendarEventTreeWidgetItem(parent, hKoRdfSemanticItem(this));
    return item;
}

Soprano::Node KoRdfCalendarEvent::linkingSubject() const
{
    kDebug(30015) << "linkingSubject() subj:" << m_linkSubject;
    return m_linkSubject;
}

void KoRdfCalendarEvent::setupStylesheetReplacementMapping(QMap<QString, QString> &m)
{
    m["%UID%"] = m_uid;
    m["%START%"] = KGlobal::locale()->formatDateTime(m_dtstart.dateTime());
    m["%END%"] = KGlobal::locale()->formatDateTime(m_dtend.dateTime());
    m["%SUMMARY%"] = m_summary;
    m["%LOCATION%"] = m_location;
    QMap< QString, KDateTime > times;
    times["START"] = m_dtstart;
    times["END"] = m_dtend;
    for (QMap< QString, KDateTime >::iterator ti = times.begin(); ti != times.end(); ++ti) {
        QString key = QString("%") + ti.key();
        KDateTime dt = ti.value();
        QDate qd = dt.date();
        m[key + "-YEAR%"] = qd.year();
        m[key + "-MONTH-NUM%"] = qd.month();
        m[key + "-MONTH-NAME%"] = QDate::shortMonthName(qd.month());
        m[key + "-DAY-NUM%"] = qd.day();
        m[key + "-DAY-NAME%"] = QDate::shortDayName(qd.day());
    }
}

void KoRdfCalendarEvent::exportToMime(QMimeData *md) const
{
    QTemporaryFile file;
    if (file.open()) {
        QString fileName = file.fileName();
        kDebug(30015) << "adding text/calendar data, temporary filename:" << file.fileName();
        QString mimeType = "text/calendar";
        exportToFile(file.fileName());
        QByteArray ba = KoTextRdfCore::fileToByteArray(fileName);
        md->setData(mimeType, ba);
        kDebug(30015) << "ba.sz:" << ba.size();
    }

    // plain text export for drag to other editors and console
    kDebug(30015) << "adding text/plain data";
    QString data;
    QTextStream oss(&data);
    oss << name() << ", ";
    if (location().size())
        oss << location() << ", ";
    oss << start().toString() << flush;
    md->setText(data);
}

QList<hKoSemanticStylesheet> KoRdfCalendarEvent::stylesheets() const
{
    // TODO we probably want a namespace for these (like KoXmlNS).
    QList<hKoSemanticStylesheet> stylesheets;
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("92f5d6c5-2c3a-4988-9646-2f29f3731f89",
                                     "name", "%NAME%")));
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("b4817ce4-d2c3-4ed3-bc5a-601010b33363",
                                     "summary", "%SUMMARY%")));
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("853242eb-031c-4a36-abb2-7ef1881c777e",
                                     "summary, location", "%SUMMARY%, %LOCATION%")));
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("2d6b87a8-23be-4b61-a881-876177812ad4",
                                     "summary, location, start date/time", "%SUMMARY%, %LOCATION%, %START%")));
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("115e3ceb-6bc8-445c-a932-baee09686895",
                                     "summary, start date/time", "%SUMMARY%, %START%")));
    return stylesheets;
}

QString KoRdfCalendarEvent::className() const
{
    return "Event";
}

QString KoRdfCalendarEvent::name() const
{
    return m_summary;
}

QString KoRdfCalendarEvent::location() const
{
    return m_location;
}

QString KoRdfCalendarEvent::summary() const
{
    return m_summary;
}

QString KoRdfCalendarEvent::uid() const
{
    return m_uid;
}

KDateTime KoRdfCalendarEvent::start() const
{
    return m_dtstart;
}

KDateTime KoRdfCalendarEvent::end() const
{
    return m_dtend;
}

#ifdef KDEPIMLIBS_FOUND
static KCal::CalendarResources *StdCalendar()
{
    static KCal::CalendarResources *ret = 0;
    if (!ret) {
        ret = new KCal::CalendarResources(KSystemTimeZones::local());
        ret->readConfig();
        KCal::CalendarResourceManager* manager = ret->resourceManager();
        KCal::CalendarResourceManager::Iterator it;
        for (it = manager->begin(); it != manager->end(); ++it) {
            (*it)->load();
        }
    }
    return ret;
}

KCal::Event *KoRdfCalendarEvent::toKEvent() const
{
    KCal::Event *event = new KCal::Event();
    event->setDtStart(start());
    event->setDtEnd(end());
    event->setSummary(summary());
    event->setLocation(location());
    event->setUid(uid());
    return event;
}
#endif

void KoRdfCalendarEvent::fromKEvent(KCal::Event *event)
{
#ifdef KDEPIMLIBS_FOUND
    m_dtstart = event->dtStart();
    m_dtend   = event->dtEnd();
    m_summary = event->summary();
    m_location = event->location();
    m_uid = event->uid();
    Soprano::Node n = Soprano::LiteralValue(m_dtstart.dateTime());
    KDateTime::Spec tz = toKTimeZone(n);
    KDateTime roundTrip = VEventDateTimeToKDateTime(n.toString(), tz);

    kDebug(30015) << "summary:" << m_summary;
    kDebug(30015) << "location:" << m_location;
    kDebug(30015) << "uid:" << m_uid;
    kDebug(30015) << "dtstart:" << m_dtstart;
    kDebug(30015) << "dtstart.offset:" << m_dtstart.timeZone().currentOffset();
    kDebug(30015) << "dtstart.utc:" << m_dtstart.toUtc();
    kDebug(30015) << "  local.offset:" << KSystemTimeZones::local().currentOffset();
    kDebug(30015) << "dtstart.roundTrip:" << roundTrip;
    kDebug(30015) << "dtend:" << m_dtend;
    kDebug(30015) << "dtstart.rdfnode:" << n;
    kDebug(30015) << "dtstart.roundTrip.offset:" << tz.timeZone().currentOffset();
#endif
}

void KoRdfCalendarEvent::saveToKCal()
{
#ifdef KDEPIMLIBS_FOUND
    KCal::CalendarResources *calendarResource = StdCalendar();
    calendarResource->load();
    KCal::Event *event = toKEvent();
    if (calendarResource->addEvent(event)) {
        kDebug(30015) << "Added calendar entry:" << name();
        calendarResource->save();
    } else {
        KMessageBox::error(0, i18n("Could not add entry\n%1", name()));
    }
#endif
}

void KoRdfCalendarEvent::exportToFile(const QString &fileNameConst) const
{
    QString fileName = fileNameConst;
#ifdef KDEPIMLIBS_FOUND
    if (fileName.isEmpty()) {
        fileName = KFileDialog::getSaveFileName(
                       KUrl("kfiledialog:///ExportDialog"),
                       "*.ics|ICalendar files",
                       0,
                       "Export to selected iCal file");

        if (fileName.isEmpty()) {
            kDebug(30015) << "no filename given, cancel export..";
            return;
        }
    }
    KCal::CalendarLocal cal(KSystemTimeZones::local());
    cal.addEvent(toKEvent());
    if (!cal.save(fileName)) {
        KMessageBox::error(0, i18n("Could not save iCal file\n%1", fileName));
    }
    kDebug(30015) << "wrote to export file:" << fileName;
#endif
}

void KoRdfCalendarEvent::importFromData(const QByteArray &ba, KoDocumentRdf *_rdf, KoCanvasBase *host)
{
#ifdef KDEPIMLIBS_FOUND
    kDebug(30015) << "data.sz:" << ba.size();
    kDebug(30015) << "_rdf:" << _rdf;
    if (_rdf) {
        m_rdf = _rdf;
    }
    KCal::VCalFormat v;
    KCal::Calendar *cal = new KCal::CalendarLocal(KSystemTimeZones::local());
    bool rc = v.fromRawString(cal, ba);
    kDebug(30015) << "parse rc:" << rc;
    KCal::Event::List events = cal->rawEvents();
    kDebug(30015) << "found event count:" << events.size();
    if (events.size() >= 1) {
        KCal::Event* e = events[0];
        fromKEvent(e);
    }
    delete cal;
    importFromDataComplete(ba, documentRdf(), host);
#endif
}
