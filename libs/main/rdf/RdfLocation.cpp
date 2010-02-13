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

#include "rdf/RdfLocation.h"
#include "rdf/RdfLocationEditWidget.h"

#include "rdf/KoDocumentRdf.h"
#include "rdf/KoDocumentRdf_p.h"

#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>

#include "ui_RdfLocationEditWidget.h"
#include "ui_RdfLocationViewWidget.h"

using namespace Soprano;


class RdfLocation::Private
{
public:
    Soprano::Node m_linkSubject;
    QString m_name;
    double  m_dlat;
    double  m_dlong;
    //
    // For geo84 simple ontology
    // geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#>
    //
    bool m_isGeo84;
    //
    // For lat, long as an Rdf list pointed at by cal:geo
    //
    Soprano::Node m_joiner;
    Ui::RdfLocationEditWidget editWidget;
    Ui::RdfLocationViewWidget viewWidget;
};

RdfLocation::RdfLocation(QObject* parent, KoDocumentRdf* m_rdf)
        : RdfSemanticItem(parent, m_rdf)
        , d(new Private())
{
    d->m_isGeo84 = true;
}

RdfLocation::RdfLocation(QObject* parent, KoDocumentRdf* m_rdf, Soprano::QueryResultIterator& it, bool isGeo84)
        : RdfSemanticItem(parent, m_rdf, it),
        d(new Private())
{
    d->m_linkSubject = it.binding("geo");
    d->m_dlong = optionalBindingAsString(it, "long", "0").toDouble();
    d->m_dlat  = optionalBindingAsString(it, "lat",  "0").toDouble();
    d->m_name  = QString("%1,%2").arg(d->m_dlong).arg(d->m_dlat);
    d->m_joiner = it.binding("joiner");
    d->m_isGeo84 = isGeo84;
}

RdfLocation::~RdfLocation()
{
}

void RdfLocation::showInViewer()
{
    // open marble showing lat/long
    kDebug(30015) << "RdfLocation::showInViewer() long:" << dlong() << " lat:" << dlat();
}

void RdfLocation::exportToFile(const QString& fileNameConst)
{
    QString fileName = fileNameConst;
    // save to KML
    kDebug(30015) << "RdfLocation::exportToFile() long:" << dlong() << " lat:" << dlat();
    if (!fileName.size()) {
        fileName = KFileDialog::getSaveFileName(
                       KUrl("kfiledialog:///ExportDialog"),
                       "*.kml|KML files",
                       0,
                       "Export to selected KML file");

        if (!fileName.size()) {
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

QWidget* RdfLocation::createEditor(QWidget * parent)
{
    kDebug(30015) << "RdfLocation::createEditor()";
    QWidget* ret = new QWidget(parent);
    return ret;
}

void RdfLocation::updateFromEditorData()
{
    return;
}

RdfSemanticTreeWidgetItem* RdfLocation::createQTreeWidgetItem(QTreeWidgetItem* parent)
{
    RdfLocationTreeWidgetItem* item =
        new RdfLocationTreeWidgetItem(parent, this);
    return item;
}

Soprano::Node RdfLocation::linkingSubject() const
{
    kDebug(30015) << "RdfLocation::linkingSubject() subj:" << d->m_linkSubject;
    return d->m_linkSubject;
}

void RdfLocation::setupStylesheetReplacementMapping(QMap< QString, QString >& m)
{
    m["%DLAT%"] = d->m_dlat;
    m["%DLONG%"] = d->m_dlong;
    m["%ISGEO84%"] = d->m_isGeo84;
}

void RdfLocation::exportToMime(QMimeData* md)
{
    QTemporaryFile file;
    if (file.open()) {
        QString mimeType = "application/vnd.google-earth.kml+xml";
        exportToFile(file.fileName());
        QByteArray ba = fileToByteArray(file.fileName());
        md->setData(mimeType, ba);
        kDebug(30015) << "ba.sz:" << ba.size();
    }
    QString data;
    QTextStream oss(&data);
    oss << name() << ", ";
    oss << dlat() << "," << dlong() << flush;
    md->setText(data);
}

QList<SemanticStylesheet*>& RdfLocation::stylesheets()
{
    static QList<SemanticStylesheet*> stylesheets;
    if (stylesheets.empty()) {
        stylesheets.append(
            new SemanticStylesheet("33314909-7439-4aa1-9a55-116bb67365f0", "name", "%NAME%"));
        stylesheets.append(
            new SemanticStylesheet("34584133-52b0-449f-8b7b-7f1ef5097b9a",
                                   "name, digital latitude, digital longitude",
                                   "%NAME%, %DLAT%, %DLONG%"));
    }
    return stylesheets;
}

QList<SemanticStylesheet*>& RdfLocation::userStylesheets()
{
    static QList<SemanticStylesheet*> ret;
    return ret;
}

void RdfLocation::importFromData(const QByteArray& ba, KoDocumentRdf* m_rdf, KoCanvasBase* host)
{
    // FIXME
}

QString RdfLocation::name()
{
    return d->m_name;
}

double RdfLocation::dlat()
{
    return d->m_dlat;
}

double RdfLocation::dlong()
{
    return d->m_dlong;
}

