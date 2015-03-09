/* This file is part of the KDE project
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KRITA_SKETCH_DOCUMENTLISTMODEL_H
#define KRITA_SKETCH_DOCUMENTLISTMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

#include "krita_sketch_export.h"

class KRITA_SKETCH_EXPORT DocumentListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(DocumentType)
    Q_PROPERTY(DocumentType filter READ filter WRITE setFilter)

public:
    DocumentListModel(QObject *parent = 0);
    ~DocumentListModel();

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

