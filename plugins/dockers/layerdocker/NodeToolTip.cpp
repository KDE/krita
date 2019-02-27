/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

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
  Boston, MA 02110-1301, USA.
*/
#include "NodeToolTip.h"
#include "kis_node_model.h"

#include <QImage>
#include <QModelIndex>
#include <QTextDocument>
#include <QUrl>
#include <klocalizedstring.h>

#include <kis_base_node.h>

NodeToolTip::NodeToolTip()
{
}

NodeToolTip::~NodeToolTip()
{
}

QTextDocument *NodeToolTip::createDocument(const QModelIndex &index)
{
    QTextDocument *doc = new QTextDocument(this);

    QImage thumb = index.data(int(KisNodeModel::BeginThumbnailRole) + 250).value<QImage>();
    doc->addResource(QTextDocument::ImageResource, QUrl("data:thumbnail"), thumb);

    QString name = index.data(Qt::DisplayRole).toString();
    KisBaseNode::PropertyList properties = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    QString rows;
    const QString row = QString("<tr><td align=\"right\">%1:</td><td align=\"left\">%2</td></tr>");
    QString value;
    for(int i = 0, n = properties.count(); i < n; ++i) {
        if (properties[i].isMutable)
            value = properties[i].state.toBool() ? i18n("Yes") : i18n("No");
        else
            value = properties[i].state.toString();

        rows.append(row.arg(properties[i].name).arg(value));
    }

    rows = QString("<table>%1</table>").arg(rows);

    const QString image = QString("<table border=\"1\"><tr><td><img src=\"data:thumbnail\"></td></tr></table>");
    const QString body = QString("<h3 align=\"center\">%1</h3>").arg(name)
                       + QString("<p><table><tr><td>%1</td><td>%2</td></tr></table></p>").arg(image).arg(rows);
    const QString html = QString("<html><body>%1</body></html>").arg(body);

    doc->setHtml(html);

    const int margin = 16;
    doc->setTextWidth(qMin(doc->size().width() + 2 * margin, qreal(500.0)));

    doc->setDocumentMargin(margin);
    doc->setUseDesignMetrics(true);

    return doc;
}
