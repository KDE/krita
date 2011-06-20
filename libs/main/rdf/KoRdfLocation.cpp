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

#include "KoRdfLocation.h"
#include "KoRdfLocationEditWidget.h"
#include "KoRdfSemanticItem_p.h"
#include "KoDocumentRdf.h"
#include "KoDocumentRdf_p.h"
#include "KoTextRdfCore.h"
#include "KoRdfLocationTreeWidgetItem.h"

#include <QTemporaryFile>
#include <kdebug.h>
#include <kfiledialog.h>

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
}

void KoRdfLocation::showInViewer()
{
    // open marble showing lat/long
    kDebug(30015) << "KoRdfLocation::showInViewer() long:" << dlong() << " lat:" << dlat();
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
    QWidget *ret = new QWidget(parent);
    return ret;
}

void KoRdfLocation::updateFromEditorData()
{
}

KoRdfSemanticTreeWidgetItem *KoRdfLocation::createQTreeWidgetItem(QTreeWidgetItem *parent)
{
    KoRdfLocationTreeWidgetItem *item =
        new KoRdfLocationTreeWidgetItem(parent, this);
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

QList<KoSemanticStylesheet*> KoRdfLocation::stylesheets() const
{
    QList<KoSemanticStylesheet*> stylesheets;
    stylesheets.append(
        new KoSemanticStylesheet("33314909-7439-4aa1-9a55-116bb67365f0", "name", "%NAME%"));
    stylesheets.append(
        new KoSemanticStylesheet("34584133-52b0-449f-8b7b-7f1ef5097b9a",
                                 "name, digital latitude, digital longitude",
                                 "%NAME%, %DLAT%, %DLONG%"));
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

