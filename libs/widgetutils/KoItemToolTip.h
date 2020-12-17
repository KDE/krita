/*
  SPDX-FileCopyrightText: 2006 Gábor Lehel <illissius@gmail.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KO_ITEM_TOOLTIP_H
#define KO_ITEM_TOOLTIP_H

#include <QFrame>
#include "kritawidgetutils_export.h"

class QStyleOptionViewItem;
class QModelIndex;
class QTextDocument;

/**
 * Base class for tooltips that can show extensive information about
 * the contents of the data pointed to by something that contains a
 * QModelIndex. Subclasses need to use this data to create a
 * QTextDocument that is formatted to provide the complete tooltip.
 */
class KRITAWIDGETUTILS_EXPORT KoItemToolTip : public QFrame
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
     *     QImage thumb = index.data(Qt::UserRole + KisAbstractResourceModel::LargeThumbnail).value<QImage>();
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
