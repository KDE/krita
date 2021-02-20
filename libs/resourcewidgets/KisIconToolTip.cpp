/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 1999 Carsten Pfeiffer (pfeiffer@kde.org)
 * SPDX-FileCopyrightText: 2002 Igor Jansen (rm@kde.org)
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KisIconToolTip.h"

#include <QTextDocument>
#include <QUrl>
#include <QModelIndex>

#include <KisResourceModel.h>
#include "KoCheckerBoardPainter.h"
#include <QPainter>

#include <klocalizedstring.h>
#include "kis_assert.h"

KisIconToolTip::KisIconToolTip()
{
}

KisIconToolTip::~KisIconToolTip()
{
}

void KisIconToolTip::setFixedToolTipThumbnailSize(const QSize &size)
{
    m_fixedToolTipThumbnailSize = size;
}

void KisIconToolTip::setToolTipShouldRenderCheckers(bool value)
{
    if (value) {
        m_checkersPainter.reset(new KoCheckerBoardPainter(4));
    } else {
        m_checkersPainter.reset();
    }
}

QTextDocument *KisIconToolTip::createDocument(const QModelIndex &index)
{
    QTextDocument *doc = new QTextDocument(this);

    QImage thumb = index.data(Qt::DecorationRole).value<QImage>();
    if (thumb.isNull()) {
        thumb = index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
    }

    if (!m_fixedToolTipThumbnailSize.isEmpty() && !thumb.isNull()) {
        thumb = thumb.scaled(m_fixedToolTipThumbnailSize*devicePixelRatioF(), Qt::IgnoreAspectRatio);
    }

    if (m_checkersPainter) {
        QImage image(thumb.size(), QImage::Format_ARGB32);

        {
            QPainter gc(&image);
            m_checkersPainter->paint(gc, thumb.rect());
            gc.drawImage(QPoint(), thumb);
        }

        thumb = image;
    }

    thumb.setDevicePixelRatio(devicePixelRatioF());
    doc->addResource(QTextDocument::ImageResource, QUrl("data:thumbnail"), thumb);

    QString name = index.data(Qt::DisplayRole).toString();
    QString presetDisplayName = index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString().replace("_", " ");
    //This is to ensure that the other uses of this class don't get an empty string, while resource management should get a nice string.
    if (!presetDisplayName.isEmpty()) {
        name = presetDisplayName;
    }

    QString tags;
    QString tagsData = index.data(Qt::UserRole + KisAbstractResourceModel::Tags).toStringList().join(", ");
    if (tagsData.length() > 0) {
        const QString list = QString("<ul style=\"list-style-type: none; margin: 0px;\">%1</ul> ").arg(tagsData);
        tags = QString("<p><table><tr><td>%1:</td><td>%2</td></tr></table></p>").arg(i18n("Tags"), list);
    }

    const QString image = QString("<center><img src=\"data:thumbnail\"></center>");
    const QString body = QString("<h3 align=\"center\">%1</h3>%2%3").arg(name, image, tags);
    const QString html = QString("<html><body>%1</body></html>").arg(body);

    doc->setHtml(html);

    const int margin = 16;
    doc->setTextWidth(qMin(doc->size().width() + 2 * margin, qreal(500.0)));
    doc->setDocumentMargin(margin);
    doc->setUseDesignMetrics(true);

    return doc;
}
