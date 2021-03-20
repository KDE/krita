/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 KO GmbH. Contact : Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KRITA_SKETCH_DOCUMENTLISTMODEL_H
#define KRITA_SKETCH_DOCUMENTLISTMODEL_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QColor>
#include <QUrl>
#include <QFont>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT DocumentListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(DocumentType)
    Q_PROPERTY(DocumentType filter READ filter WRITE setFilter)

public:
    DocumentListModel(QObject *parent = 0);
    ~DocumentListModel();

    QHash<int, QByteArray> roleNames() const;

    enum CustomRoles {
        FileNameRole = Qt::UserRole + 1,
        FilePathRole,
        DocTypeRole,
        FileSizeRole,
        AuthorNameRole,
        AccessedTimeRole,
        ModifiedTimeRole,
        UUIDRole,
    };

    enum DocumentType {
        UnknownType,
        TextDocumentType,
        PresentationType,
        SpreadsheetType,
        PDFDocumentType,
    };

    struct DocumentInfo {
        bool operator==(const DocumentInfo &other) const {
            return filePath == other.filePath;
        }
        QString filePath;
        QString fileName;
        DocumentType docType;
        QString fileSize;
        QString authorName;
        QDateTime accessedTime;
        QDateTime modifiedTime;
        QString uuid;
    };

    // reimp from QAbstractListModel
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;


    DocumentType filter();

    static QString prettyTime(const QDateTime &theTime);
    static DocumentType typeForFile(const QString &file);

public Q_SLOTS:
    void addDocument(const DocumentListModel::DocumentInfo &info);
    void setFilter(DocumentType newFilter);

private:

    class Private;
    const QScopedPointer<Private> d;

    static QHash<QString, DocumentType> sm_extensions;
};

Q_DECLARE_METATYPE(DocumentListModel::DocumentInfo);

#endif // KRITA_SKETCH_DOCUMENTLISTMODEL_H

