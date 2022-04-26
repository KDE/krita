/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "NodeToolTip.h"
#include "kis_node_model.h"

#include <QImage>
#include <QModelIndex>
#include <QTextDocument>
#include <QUrl>
#include <klocalizedstring.h>

#include <kis_base_node.h>
#include <kis_layer_properties_icons.h>

NodeToolTip::NodeToolTip()
{
}

NodeToolTip::~NodeToolTip()
{
}

QTextDocument *NodeToolTip::createDocument(const QModelIndex &index)
{
    QTextDocument *doc = new QTextDocument(this);

    int size = 250*devicePixelRatioF();
    QImage thumb = index.data(int(KisNodeModel::BeginThumbnailRole) + size).value<QImage>();
    thumb.setDevicePixelRatio(devicePixelRatioF());
    doc->addResource(QTextDocument::ImageResource, QUrl("data:thumbnail"), thumb);

    QString name = index.data(Qt::DisplayRole).toString();
    KisBaseNode::PropertyList properties = index.data(KisNodeModel::PropertiesRole).value<KisBaseNode::PropertyList>();
    QString rows;
    // Note: Can't use <nobr> due to https://bugreports.qt.io/browse/QTBUG-1135
    // It breaks randomly with CJK. Works fine with `white-space:pre` though.
    const QString row = QString("<tr><td align=\"right\"><p style=\"white-space:pre\">%1:</p></td><td align=\"left\">%2</td></tr>");
    QString value;
    for(int i = 0, n = properties.count(); i < n; ++i) {
        if (properties[i].id == KisLayerPropertiesIcons::layerError.id()) continue;

        if (properties[i].isMutable)
            value = properties[i].state.toBool() ? i18n("Yes") : i18n("No");
        else
            value = properties[i].state.toString();

        rows.append(row.arg(properties[i].name).arg(value));
    }

    QString dropReason = index.data(KisNodeModel::DropReasonRole).toString();

    if (!dropReason.isEmpty()) {
        dropReason = QString("<p align=\"center\"><b>%1</b></p>").arg(dropReason);
    }

    QString errorMessage;
    {
        auto it = std::find_if(properties.begin(), properties.end(),
        [] (const KisBaseNode::Property &prop) {
            return prop.id == KisLayerPropertiesIcons::layerError.id();
        });
        if (it != properties.end()) {
            doc->addResource(QTextDocument::ImageResource, QUrl("data:warn_symbol"), it->onIcon.pixmap(QSize(32,32)).toImage());
            errorMessage = QString("<table align=\"center\" border=\"0\"><tr valign=\"middle\"><td align=\"right\"><img src=\"data:warn_symbol\"></td><td align=\"left\"><b>%1</b></td></tr></table>").arg(it->state.toString());
        }
    }

    rows = QString("<table>%1</table>").arg(rows);

    const QString image = QString("<table border=\"1\"><tr><td><img src=\"data:thumbnail\"></td></tr></table>");
    const QString body = QString("<h3 align=\"center\">%1</h3>").arg(name)
                       + errorMessage
                       + dropReason
                       + QString("<p><table><tr><td>%1</td><td>%2</td></tr></table></p>").arg(image).arg(rows);
    const QString html = QString("<html><body>%1</body></html>").arg(body);

    doc->setHtml(html);

    const int margin = 16;
    doc->setTextWidth(qMin(doc->size().width() + 2 * margin, qreal(600.0)));

    doc->setDocumentMargin(margin);
    doc->setUseDesignMetrics(true);

    return doc;
}
