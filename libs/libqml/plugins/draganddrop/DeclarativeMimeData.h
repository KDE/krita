/*
    SPDX-FileCopyrightText: 2010 BetterInbox <contact@betterinbox.com>
    Original author: Gregory Schlomoff <greg@betterinbox.com>

    SPDX-License-Identifier: MIT
*/

#ifndef DECLARATIVEMIMEDATA_H
#define DECLARATIVEMIMEDATA_H

#include <QMimeData>
#include <QColor>
#include <QUrl>
#include <QQuickItem>
#include <QJsonArray>

class DeclarativeMimeData : public QMimeData
{
    Q_OBJECT

    /**
     * A plain text (MIME type text/plain) representation of the data.
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    /**
     * A string if the data stored in the object is HTML (MIME type text/html); otherwise returns an empty string.
     */
    Q_PROPERTY(QString html READ html WRITE setHtml NOTIFY htmlChanged)

    /**
     * Url contained in the mimedata
     */
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)

    /**
     * A list of URLs contained within the MIME data object.
     * URLs correspond to the MIME type text/uri-list.
     */
    Q_PROPERTY(QJsonArray urls READ urls WRITE setUrls NOTIFY urlsChanged)

    /**
     * A color if the data stored in the object represents a color (MIME type application/x-color); otherwise QColor().
     */
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

    /**
     * The graphical item on the scene that started the drag event. It may be null.
     */
    Q_PROPERTY(QQuickItem* source READ source WRITE setSource NOTIFY sourceChanged)

    /** @see QMimeData::hasUrls */
    Q_PROPERTY(bool hasUrls READ hasUrls NOTIFY urlsChanged)
    //TODO: Image property

    /**
     * @sa QMimeData::formats
     */
    Q_PROPERTY(QStringList formats READ formats)
public:
    DeclarativeMimeData();
    DeclarativeMimeData(const QMimeData* copy);

    QUrl url() const;
    void setUrl(const QUrl &url);

    QJsonArray urls() const;
    void setUrls(const QJsonArray &urls);

    QColor color() const;
    void setColor(const QColor &color);
    Q_INVOKABLE bool hasColor() const;

    Q_INVOKABLE void setData(const QString &mimeType, const QVariant &data);

    QQuickItem* source() const;
    void setSource(QQuickItem* source);

    Q_INVOKABLE QByteArray getDataAsByteArray(const QString& format);

    /*
    QString text() const;                //TODO: Reimplement this to issue the onChanged signals
    void setText(const QString &text);
    QString html() const;
    void setHtml(const QString &html);
    */

Q_SIGNALS:
    void textChanged();        //FIXME not being used
    void htmlChanged();        //FIXME not being used
    void urlChanged();
    void urlsChanged();
    void colorChanged();
    void sourceChanged();

private:
    QQuickItem* m_source;
};

#endif // DECLARATIVEMIMEDATA_H

