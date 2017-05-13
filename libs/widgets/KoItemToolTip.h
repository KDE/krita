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

#ifndef KO_ITEM_TOOLTIP_H
#define KO_ITEM_TOOLTIP_H

#include <QFrame>
#include "kritawidgets_export.h"

class QStyleOptionViewItem;
class QModelIndex;
class QTextDocument;

/**
 * Base class for tooltips that can show extensive information about
 * the contents of the data pointed to by something that contains a
 * QModelIndex. Subclasses need to use this data to create a
 * QTextDocument that is formatted to provide the complete tooltip.
 *
 * (KoItemToolTip is currently used in kopainter/KoResourceChooser)
 */
class KRITAWIDGETS_EXPORT KoItemToolTip : public QFrame
{
    Q_OBJECT
public:
    KoItemToolTip();
    ~KoItemToolTip() override;
    void showTip(QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:

    /**
     * Re-implement this to provide the actual tooltip contents.
     * For instance:
     * @code
     *    QTextDocument *doc = new QTextDocument(this);
     *
     *     QImage thumb = index.data(KoResourceModel::LargeThumbnailRole).value<QImage>();
     *     doc->addResource(QTextDocument::ImageResource, QUrl("data:thumbnail"), thumb);
     *
     *     QString name = index.data(Qt::DisplayRole).toString();
     *
     *     const QString image = QString("<img src=\"data:thumbnail\">");
     *     const QString body = QString("<h3 align=\"center\">%1</h3>").arg(name) + image;
     *     const QString html = QString("<html><body>%1</body></html>").arg(body);
     *
     *     doc->setHtml(html);
     *     doc->setTextWidth(qMin(doc->size().width(), 500.0));
     *
     *     return doc;
     * @endcode
     */
    virtual QTextDocument *createDocument(const QModelIndex &index) = 0;

private:
    class Private;
    Private* const d;

    void updatePosition(QWidget *widget, const QPoint &pos, const QStyleOptionViewItem &option);

public:
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void timerEvent(QTimerEvent *e) override;
    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif
