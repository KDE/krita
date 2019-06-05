/*
 *  Copyright (c) 2019 Tusooa Zhu <tusooa@vista.aero>
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

#include "KisSnapshotModel.h"

#include <QMap>
#include <QList>
#include <QPointer>
#include <QPair>
#include <QString>

#include <KisDocument.h>
#include <KisView.h>
#include <KisViewManager.h>

struct KisSnapshotModel::Private
{
    Private();
    virtual ~Private();

    QPointer<KisDocument> curDocument();
    bool switchToDocument(QPointer<KisDocument> doc);

    using DocPList = QList<QPair<QString, QPointer<KisDocument> > >;

    DocPList curDocList;

    QMap<QPointer<KisDocument>, DocPList> documentGroups;
    QPointer<KisCanvas2> curCanvas;
};

KisSnapshotModel::Private::Private()
{
}

KisSnapshotModel::Private::~Private()
{
}

QPointer<KisDocument> KisSnapshotModel::Private::curDocument()
{
    if (curCanvas && curCanvas->imageView()) {
        return curCanvas->imageView()->document();
    }
    return 0;
}

bool KisSnapshotModel::Private::switchToDocument(QPointer<KisDocument> doc)
{
    if (curCanvas && curCanvas->imageView()) {
        KisView *view = curCanvas->imageView();
        KisDocument *curDoc = curDocument();
        if (curDoc && doc) {
            curDoc->copyFromDocument(*doc);
        }
        // FIXME: more things need to be done
        return true;
    }
    return false;
}

KisSnapshotModel::KisSnapshotModel()
    : QAbstractListModel()
    , m_d(new Private)
{
}

KisSnapshotModel::~KisSnapshotModel()
{
}

int KisSnapshotModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    } else {
        return m_d->curDocList.size();
    }
}

QVariant KisSnapshotModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount(QModelIndex())) {
        return QVariant();
    }
    int i = index.row();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return m_d->curDocList[i].first;
        break;
    }
    return QVariant();
}

bool KisSnapshotModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= rowCount(QModelIndex())) {
        return false;
    }
    int i = index.row();
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        m_d->curDocList[i].first = value.toString();
        emit dataChanged(index, index);
        return true;
        break;
    }
    return false;
}

void KisSnapshotModel::setCanvas(QPointer<KisCanvas2> canvas)
{
    if (m_d->curCanvas == canvas) {
        return;
    }

    if (m_d->curCanvas) {
        if (m_d->curDocument()) {
            m_d->documentGroups.insert(m_d->curDocument(), m_d->curDocList);
        } else {
            Q_FOREACH (auto const &i, m_d->curDocList) {
                delete i.second.data();
            }
        }
    }
    if (!m_d->curDocList.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_d->curDocList.size() - 1);
        m_d->curDocList.clear();
        endRemoveRows();
    }
    m_d->curCanvas = canvas;

    QPointer<KisDocument> curDoc = m_d->curDocument();
    if (curDoc) {
        QMap<QPointer<KisDocument>, Private::DocPList>::const_iterator i = m_d->documentGroups.constFind(curDoc);
        if (i != m_d->documentGroups.constEnd()) {
            Private::DocPList docList = i.value();
            beginInsertRows(QModelIndex(), docList.size(), docList.size());
            m_d->curDocList = docList;
            endInsertRows();
        }
        // we have not found any existing group containing the current document
    }

}

bool KisSnapshotModel::slotCreateSnapshot()
{
    QPointer<KisDocument> clonedDoc(m_d->curDocument()->lockAndCloneForSaving());
    if (clonedDoc) {
        beginInsertRows(QModelIndex(), m_d->curDocList.size(), m_d->curDocList.size());
        m_d->curDocList << qMakePair(i18n("Snapshot"), clonedDoc);
        endInsertRows();
        return true;
    }
    return false;
}

bool KisSnapshotModel::slotRemoveSnapshot(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_d->curDocList.size()) {
        return false;
    }
    int i = index.row();
    beginRemoveRows(QModelIndex(), i, i);
    QPair<QString, QPointer<KisDocument> > pair = m_d->curDocList.takeAt(i);
    endRemoveRows();
    delete pair.second.data();
    return true;
}

bool KisSnapshotModel::slotSwitchToSnapshot(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= m_d->curDocList.size()) {
        return false;
    }

    return m_d->switchToDocument(m_d->curDocList[index.row()].second);
}
