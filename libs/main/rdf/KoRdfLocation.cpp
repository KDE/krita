/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2011 Ben Martin hacking for fun!

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

#include "KoRdfLocation.h"
#include "KoRdfLocationEditWidget.h"
#include "KoRdfSemanticItem_p.h"
#include "KoDocumentRdf.h"
#include "KoTextRdfCore.h"
#include "KoRdfLocationTreeWidgetItem.h"

#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>

// Don't use this until we become a plugin.
#ifdef CAN_USE_MARBLE
#undef CAN_USE_MARBLE
#endif

// marble for geolocation
#ifdef CAN_USE_MARBLE
#include <marble/LatLonEdit.h>
#include <marble/MarbleWidget.h>
#include <marble/MarbleWidgetInputHandler.h>
#endif

using namespace Soprano;

KoRdfLocation::KoRdfLocation(QObject *parent, const KoDocumentRdf *m_rdf)
    : KoRdfSemanticItem(m_rdf, parent)
{
    m_isGeo84 = true;
}

KoRdfLocation::KoRdfLocation(QObject *parent, const KoDocumentRdf *m_rdf, Soprano::QueryResultIterator &it, bool isGeo84)
    : KoRdfSemanticItem(m_rdf, it, parent)
{
    m_linkSubject = it.binding("geo");
    m_dlong = KoTextRdfCore::optionalBindingAsString(it, "long", "0").toDouble();
    m_dlat  = KoTextRdfCore::optionalBindingAsString(it, "lat",  "0").toDouble();
    m_name  = QString("%1,%2").arg(m_dlong).arg(m_dlat);
    m_joiner = it.binding("joiner");
    m_isGeo84 = isGeo84;
}

KoRdfLocation::~KoRdfLocation()
{
    kDebug(30015) << "~KoRdfLocation() this:" << this << " name:" << name();
}

void KoRdfLocation::showInViewer()
{
    // open marble showing lat/long
    kDebug(30015) << "KoRdfLocation::showInViewer() long:" << dlong() << " lat:" << dlat();

#ifdef CAN_USE_MARBLE
    kDebug(30015) << "RDFLocation::showInViewer() opening a marble widget...";

    QWidget* parent = 0;
    QWidget* ret = new QWidget(parent);
    viewWidget.setupUi(ret);
    viewWidget.name->setText(m_name);
    viewWidget.map->setMapThemeId("earth/srtm/srtm.dgml");
//    viewWidget.map->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
    viewWidget.map->zoomViewBy(100);
    viewWidget.map->zoomView(1500);
    viewWidget.map->centerOn(dlong(), dlat());
    ret->show();
#endif
    
}

void KoRdfLocation::exportToFile(const QString &fileNameConst) const
{
    QString fileName = fileNameConst;
    // save to KML
    kDebug(30015) << "KoRdfLocation::exportToFile() long:" << dlong() << " lat:" << dlat();
    if (fileName.isEmpty()) {
        fileName = KFileDialog::getSaveFileName(
                       KUrl("kfiledialog:///ExportDialog"),
                       "*.kml|KML files",
                       0,
                       "Export to selected KML file");

        if (fileName.isEmpty()) {
            return;
        }
    }
    QString xmlstring;
    QTextStream xmlss(&xmlstring);
    xmlss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?> \n"
        << "<kml xmlns=\"http://www.opengis.net/kml/2.2\" > \n"
        << " \n"
        << "<Placemark> \n"
        << "  <name>" << name() << "</name> \n"
        << "  <LookAt> \n"
        << "    <longitude>" << dlong() << "</longitude> \n"
        << "    <latitude>" << dlat() << "</latitude> \n"
        << "  </LookAt> \n"
        << "</Placemark> \n"
        << "</kml>\n";
    xmlss.flush();
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(xmlstring.toLocal8Bit());
    file.close();
}

QWidget *KoRdfLocation::createEditor(QWidget *parent)
{
    kDebug(30015) << "KoRdfLocation::createEditor()";
#ifndef CAN_USE_MARBLE
    {
        KoRdfLocationEditWidget* ret = new KoRdfLocationEditWidget(parent, &editWidget);

        editWidget.setupUi(ret);
        editWidget.name->setText(m_name);
        
        return ret;
    }
#else
    KoRdfLocationEditWidget* ret = new KoRdfLocationEditWidget(parent, &editWidget);

    editWidget.setupUi(ret);
    editWidget.name->setText(m_name);

    editWidget.wlat->setDimension(Marble::Latitude);
    editWidget.wlong->setDimension(Marble::Longitude);
    editWidget.wlat->setValue(m_dlat);
    editWidget.wlong->setValue(m_dlong);

    editWidget.map->setMapThemeId("earth/srtm/srtm.dgml");
//    editWidget.map->setMapThemeId("earth/openstreetmap/openstreetmap.dgml");
    editWidget.map->zoomViewBy(100);
    editWidget.map->zoomView(1500);
    editWidget.map->centerOn(dlong(), dlat());

    ret->setupMap(editWidget.map, editWidget.wlat, editWidget.wlong);
    return ret;
#endif
}

void KoRdfLocation::updateFromEditorData()
{
#ifndef CAN_USE_MARBLE
    return;
#else

    QString rdfBase  = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    QString predBase = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    
    if (!m_linkSubject.isValid()) {
        m_linkSubject = createNewUUIDNode();
    }
    if (!m_isGeo84) {
        if (!m_joiner.isValid()) {
            QString tmp = "";
            Node newV = createNewUUIDNode();

            QSharedPointer<Soprano::Model> m = m_rdf->model();
            Node pred = Node::createResourceNode(QUrl(rdfBase + "rest"));

            m->addStatement(linkingSubject(), pred, newV,
                            m_rdf->manifestRdfNode());
            m_joiner = newV;
        }
    }

    double newLat  = editWidget.map->centerLatitude();
    double newLong = editWidget.map->centerLongitude();

    kDebug(30015) << "RDFLocation::updateFromEditorData()";
    kDebug(30015) << "old lat:" << m_dlat;
    kDebug(30015) << "new lat:" << newLat;
    kDebug(30015) << "old long:" << m_dlong;
    kDebug(30015) << "new long:" << newLong;
    kDebug(30015) << "m_isGeo84:" << m_isGeo84;

    QString foafBase = "http://xmlns.com/foaf/0.1/";
    QString dcBase = "http://purl.org/dc/elements/1.1/";

    if (m_isGeo84) {
        //
        // http://www.w3.org/2003/01/geo/wgs84_pos ontology
        //
        QString wgs84Base = "http://www.w3.org/2003/01/geo/wgs84_pos#";

        setRdfType("uri:geo84");
        updateTriple(m_name,     editWidget.name->text(),   dcBase + "title");
        updateTriple(m_dlat,     newLat,  wgs84Base + "lat",  linkingSubject());
        updateTriple(m_dlong,    newLong, wgs84Base + "long", linkingSubject());
    } else {
        //
        // RDF ical has support for pointing to a linked list of lat, long, NIL
        //
        setRdfType("uri:rdfcal-geolocation");
        updateTriple(m_name,     editWidget.name->text(),   dcBase + "title");
        updateTriple(m_dlat,     newLat,  rdfBase + "first", linkingSubject());
        updateTriple(m_dlong,    newLong, rdfBase + "first", m_joiner);
    }

#endif

    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
    }
}

KoRdfSemanticTreeWidgetItem *KoRdfLocation::createQTreeWidgetItem(QTreeWidgetItem *parent)
{
    KoRdfLocationTreeWidgetItem *item =
        new KoRdfLocationTreeWidgetItem(parent, hKoRdfSemanticItem(this));
    return item;
}

Soprano::Node KoRdfLocation::linkingSubject() const
{
    kDebug(30015) << "KoRdfLocation::linkingSubject() subj:" << m_linkSubject;
    return m_linkSubject;
}

void KoRdfLocation::setupStylesheetReplacementMapping(QMap<QString, QString> &m)
{
    m["%DLAT%"] = QString("%1").arg(m_dlat);
    m["%DLONG%"] = QString("%1").arg(m_dlong);
    m["%ISGEO84%"] = QString("%1").arg(m_isGeo84);
}

void KoRdfLocation::exportToMime(QMimeData *md) const
{
    QTemporaryFile file;
    if (file.open()) {
        QString mimeType = "application/vnd.google-earth.kml+xml";
        exportToFile(file.fileName());
        QByteArray ba = KoTextRdfCore::fileToByteArray(file.fileName());
        md->setData(mimeType, ba);
        kDebug(30015) << "ba.sz:" << ba.size();
    }
    QString data;
    QTextStream oss(&data);
    oss << name() << ", ";
    oss << dlat() << "," << dlong() << flush;
    md->setText(data);
}

QList<hKoSemanticStylesheet> KoRdfLocation::stylesheets() const
{
    QList<hKoSemanticStylesheet> stylesheets;
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("33314909-7439-4aa1-9a55-116bb67365f0", "name", "%NAME%")));
    stylesheets.append(
        hKoSemanticStylesheet(
            new KoSemanticStylesheet("34584133-52b0-449f-8b7b-7f1ef5097b9a",
                                     "name, digital latitude, digital longitude",
                                     "%NAME%, %DLAT%, %DLONG%")));
    return stylesheets;
}

QString KoRdfLocation::className() const
{
    return "Location";
}

void KoRdfLocation::importFromData(const QByteArray& ba, KoDocumentRdf* m_rdf, KoCanvasBase* host)
{
    Q_UNUSED(ba);
    Q_UNUSED(m_rdf);
    Q_UNUSED(host);
#ifdef __GNUC__
    #warning FIXME: implement importFromData
#endif

}

QString KoRdfLocation::name() const
{
    return m_name;
}

double KoRdfLocation::dlat() const
{
    return m_dlat;
}

double KoRdfLocation::dlong() const
{
    return m_dlong;
}

void KoRdfLocation::setName(const QString &name)
{
    QString rdfBase  = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";

    if (!m_linkSubject.isValid()) {
        m_linkSubject = createNewUUIDNode();
    }
    if (!m_isGeo84) {
        if (!m_joiner.isValid()) {
            QString tmp = "";
            Node newV = createNewUUIDNode();

            QSharedPointer<Soprano::Model> m = m_rdf->model();
            Node pred = Node::createResourceNode(QUrl(rdfBase + "rest"));

            m->addStatement(linkingSubject(), pred, newV,
                            m_rdf->manifestRdfNode());
            m_joiner = newV;
        }
    }
    QString dcBase = "http://purl.org/dc/elements/1.1/";

    if (m_isGeo84) {
        //
        // http://www.w3.org/2003/01/geo/wgs84_pos ontology
        //
        setRdfType("uri:geo84");
        updateTriple(m_name, name, dcBase + "title");
    } else {
        //
        // RDF ical has support for pointing to a linked list of lat, long, NIL
        //
        setRdfType("uri:rdfcal-geolocation");
        updateTriple(m_name, name, dcBase + "title");
    }
    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
    }

}

void KoRdfLocation::setDlat(double dlat)
{
    QString rdfBase  = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";

    if (!m_linkSubject.isValid()) {
        m_linkSubject = createNewUUIDNode();
    }
    if (!m_isGeo84) {
        if (!m_joiner.isValid()) {
            QString tmp = "";
            Node newV = createNewUUIDNode();

            QSharedPointer<Soprano::Model> m = m_rdf->model();
            Node pred = Node::createResourceNode(QUrl(rdfBase + "rest"));

            m->addStatement(linkingSubject(), pred, newV,
                            m_rdf->manifestRdfNode());
            m_joiner = newV;
        }
    }
    if (m_isGeo84) {
        //
        // http://www.w3.org/2003/01/geo/wgs84_pos ontology
        //
        QString wgs84Base = "http://www.w3.org/2003/01/geo/wgs84_pos#";
        setRdfType("uri:geo84");
        updateTriple(m_dlat, dlat,  wgs84Base + "lat",  linkingSubject());
    } else {
        //
        // RDF ical has support for pointing to a linked list of lat, long, NIL
        //
        setRdfType("uri:rdfcal-geolocation");
        updateTriple(m_dlat, dlat,  rdfBase + "first", linkingSubject());
    }

    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
    }

}

void KoRdfLocation::setDlong(double dlong)
{
    QString rdfBase  = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";

    if (!m_linkSubject.isValid()) {
        m_linkSubject = createNewUUIDNode();
    }
    if (!m_isGeo84) {
        if (!m_joiner.isValid()) {
            Node newV = createNewUUIDNode();

            QSharedPointer<Soprano::Model> m = m_rdf->model();
            Node pred = Node::createResourceNode(QUrl(rdfBase + "rest"));

            m->addStatement(linkingSubject(), pred, newV, m_rdf->manifestRdfNode());
            m_joiner = newV;
        }
    }

    if (m_isGeo84) {
        //
        // http://www.w3.org/2003/01/geo/wgs84_pos ontology
        //
        QString wgs84Base = "http://www.w3.org/2003/01/geo/wgs84_pos#";
        setRdfType("uri:geo84");
        updateTriple(m_dlong, dlong, wgs84Base + "long", linkingSubject());
    } else {
        //
        // RDF ical has support for pointing to a linked list of lat, long, NIL
        //
        updateTriple(m_dlong, dlong, rdfBase + "first", m_joiner);
    }

    if (documentRdf()) {
        const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
    }

}
