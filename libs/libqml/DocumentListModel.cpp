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
#include "DocumentListModel.h"

#include <QTimer>
#include <QLocale>


#include <klocalizedstring.h>

class DocumentListModel::Private
{
public:
    Private( DocumentListModel *qq) : q(qq), filter(DocumentListModel::UnknownType) { }

    void relayout();

    DocumentListModel* q;

    QList<DocumentInfo> allDocumentInfos;
    QList<DocumentInfo> currentDocumentInfos;

    DocumentType filter;
    QString searchPattern;

    QTimer *timer;
};

QHash<QString, DocumentListModel::DocumentType> DocumentListModel::sm_extensions = QHash<QString, DocumentListModel::DocumentType>();

DocumentListModel::DocumentListModel(QObject *parent)
    : QAbstractListModel(parent), d(new Private(this))
{
    qRegisterMetaType<DocumentListModel::DocumentInfo>();

}

DocumentListModel::~DocumentListModel()
{
}

QHash<int, QByteArray> DocumentListModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[FileNameRole] = "fileName";
    roleNames[FilePathRole] = "filePath";
    roleNames[DocTypeRole] = "docType";
    roleNames[FileSizeRole] = "fileSize";
    roleNames[AuthorNameRole] = "authorName";
    roleNames[AccessedTimeRole] = "accessedTime";
    roleNames[ModifiedTimeRole] = "modifiedTime";
    roleNames[UUIDRole] = "uuid";

    return roleNames;
}

void DocumentListModel::addDocument(const DocumentInfo &info)
{
    if (d->allDocumentInfos.contains(info))
    {
        return;
    }

    d->allDocumentInfos.append(info);
}

int DocumentListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return d->currentDocumentInfos.count();
}

int DocumentListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant DocumentListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const int row = index.row();
    const DocumentInfo &info = d->currentDocumentInfos[row];

    switch (role) {
        case FileNameRole: Q_FALLTHROUGH();
        case Qt::DisplayRole: return info.fileName;
        case FilePathRole: return info.filePath;
        case DocTypeRole: return info.docType;
        case FileSizeRole: return info.fileSize;
        case AuthorNameRole: return info.authorName;
        case AccessedTimeRole: return prettyTime(info.accessedTime);
        case ModifiedTimeRole: return prettyTime(info.modifiedTime);
        case UUIDRole: return info.uuid;
        default: return QVariant();
    }
}

QString DocumentListModel::prettyTime( const QDateTime& theTime)
{
    return QLocale().toString(theTime, QLocale::LongFormat);
}

QVariant DocumentListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    Q_UNUSED(orientation)
    Q_UNUSED(role)
    return QVariant();
}

DocumentListModel::DocumentType DocumentListModel::filter()
{
    return d->filter;
}

void DocumentListModel::setFilter( DocumentListModel::DocumentType newFilter)
{
    d->filter = newFilter;
    d->relayout();
}



DocumentListModel::DocumentType DocumentListModel::typeForFile ( const QString& file )
{
    if (sm_extensions.isEmpty()) {
        sm_extensions["odt"] = TextDocumentType;
        sm_extensions["fodt"] = TextDocumentType;
        sm_extensions["doc"] = TextDocumentType;
        sm_extensions["docx"] = TextDocumentType;
        sm_extensions["txt"] = TextDocumentType;
        sm_extensions["odp"] = PresentationType;
        sm_extensions["fodp"] = PresentationType;
        sm_extensions["ppt"] = PresentationType;
        sm_extensions["pptx"] = PresentationType;
        sm_extensions["ods"] = SpreadsheetType;
        sm_extensions["fods"] = SpreadsheetType;
        sm_extensions["xls"] = SpreadsheetType;
        sm_extensions["xlsx"] = SpreadsheetType;
        sm_extensions["pdf"] = PDFDocumentType;
    }

    QString ext = file.split('.').last().toLower();
    if (sm_extensions.contains(ext)) {
        return sm_extensions.value(ext);
    }

    return UnknownType;
}

void DocumentListModel::Private::relayout()
{
    emit q->layoutAboutToBeChanged();

    QList<DocumentInfo> newList;
    Q_FOREACH (const DocumentInfo &docInfo, allDocumentInfos) {
        if (filter == UnknownType || docInfo.docType == filter) {
            if (searchPattern.isEmpty() || docInfo.fileName.contains(searchPattern, Qt::CaseInsensitive)) {
                newList.append(docInfo);
            }
        }
    }

    currentDocumentInfos = newList;
    emit q->layoutChanged();
    q->beginResetModel();
    q->endResetModel();
}
