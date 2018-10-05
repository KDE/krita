/*
 * Copyright (c) 2018 boud <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "KisNewsWidget.h"

#include <QDesktopServices>
#include <QUrl>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

#include "kis_config.h"
#include "KisMultiFeedRSSModel.h"


KisNewsDelegate::KisNewsDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    qDebug() << "Delegate created";
}

void KisNewsDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem optionCopy = option;
    initStyleOption(&optionCopy, index);

    QStyle *style = optionCopy.widget? optionCopy.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(optionCopy.text);
    doc.setDocumentMargin(10);

    /// Painting item without text
    optionCopy.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &optionCopy, painter);

    QAbstractTextDocumentLayout::PaintContext ctx;

    // Highlighting text if item is selected
    if (optionCopy.state & QStyle::State_Selected) {
        ctx.palette.setColor(QPalette::Text, optionCopy.palette.color(QPalette::Active, QPalette::HighlightedText));
    }

    painter->translate(optionCopy.rect.left(), optionCopy.rect.top());
    QRect clip(0, 0, optionCopy.rect.width(), optionCopy.rect.height());
    doc.setPageSize(clip.size());
    doc.drawContents(painter, clip);
    painter->restore();
}

QSize KisNewsDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem optionCopy = option;
    initStyleOption(&optionCopy, index);

    QTextDocument doc;
    doc.setHtml(optionCopy.text);
    doc.setTextWidth(optionCopy.rect.width());
    return QSize(doc.idealWidth(), doc.size().height());
}

KisNewsWidget::KisNewsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    m_rssModel = new MultiFeedRssModel(this);

    setCursor(Qt::PointingHandCursor);

    listNews->setModel(m_rssModel);
    listNews->setItemDelegate(new KisNewsDelegate(listNews));
    connect(listNews, SIGNAL(clicked(QModelIndex)), this, SLOT(itemSelected(QModelIndex)));
}

void KisNewsWidget::toggleNews(bool toggle)
{
    KisConfig cfg(false);
    cfg.writeEntry<bool>("FetchNews", toggle);

    if (toggle) {
        m_rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));
    }
    else {
        m_rssModel->removeFeed(QLatin1String("https://krita.org/en/feed/"));
    }
}

void KisNewsWidget::itemSelected(const QModelIndex &idx)
{
    if (idx.isValid()) {
        QString link = idx.data(RssRoles::LinkRole).toString();
        QDesktopServices::openUrl(QUrl(link));
    }
}
