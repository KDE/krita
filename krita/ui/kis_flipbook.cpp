/*
 *  Copyright (c) 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_flipbook.h"
#include <krita_export.h>

#include <KoApplication.h>
#include <KoPart.h>

#include <QFile>
#include <QDomDocument>
#include <QByteArray>
#include <QString>

#include "kis_flipbook_item.h"

KisFlipbook::KisFlipbook(QObject *parent) :
    QStandardItemModel(parent)
{
}

void KisFlipbook::setName(const QString &name)
{
    m_name = name;
}

QString KisFlipbook::name() const
{
    return m_name;
}


KisFlipbookItem *KisFlipbook::addItem(const QString &url)
{
    KisFlipbookItem *item = new KisFlipbookItem(url);
    appendRow(item);
    return item;
}

void KisFlipbook::load(const QString &url)
{
    QFile f(url);
    if (f.exists()) {
        f.open(QFile::ReadOnly);
        QByteArray ba = f.readAll();
        if (ba.size() > 0) {
            QDomDocument doc;
            doc.setContent(QString::fromUtf8(ba.constData(), ba.size()));
            QDomElement e = doc.documentElement();
            QDomNode n = e.firstChild();
            while (!n.isNull()) {
                QDomElement e = n.toElement();
                if (!e.isNull()) {
                    if (e.tagName() == "flipbook-name") {
                        m_name = e.text();
                    }
                    else if (e.tagName() == "flipbook-file") {
                        KisFlipbookItem *item = new KisFlipbookItem(e.text());
                        appendRow(item);
                    }
                }
                n = n.nextSibling();
            }
        }
    }
}


void KisFlipbook::save(const QString &url)
{
    QDomDocument doc = QDomDocument("krita-flipbook");
    QDomElement root = doc.createElement("krita-flipbook");
    doc.appendChild(root);
    QDomElement e = doc.createElement("flipbook-name");
    QDomText text = doc.createTextNode(m_name);
    e.appendChild(text);
    root.appendChild(e);

    for (int i = 0; i < rowCount(); ++i) {
        KisFlipbookItem *flipbookItem = static_cast<KisFlipbookItem*>(item(i));
        e = doc.createElement("flipbook-file");
        text = doc.createTextNode(flipbookItem->filename());
        e.appendChild(text);
        root.appendChild(e);
    }

    QFile f(url);
    f.open(QFile::WriteOnly);
    QByteArray ba = doc.toString().toUtf8();
    qint64 written = f.write(ba);
    Q_ASSERT(written == ba.size());

    KoApplication *app = dynamic_cast<KoApplication*>(qApp);
    if (app) {
        foreach(KoPart *part, app->partList()) {
            part->addRecentURLToAllShells(url);
        }
    }
}
