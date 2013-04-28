/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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
 * Boston, MA 02110-1301, USA.
*/

#include "KoSopranoTableModel.h"
#include "KoDocumentRdf.h"
#include "KoRdfPrefixMapping.h"
#include <kdebug.h>
#include <klocale.h>

KoSopranoTableModel::KoSopranoTableModel(KoDocumentRdf *rdf)
        : m_rdf(rdf)
{
    Soprano::StatementIterator siter = model()->listStatements();
    while (siter.next()) {
        m_statementIndex << *siter;
    }
}

QSharedPointer<Soprano::Model> KoSopranoTableModel::model() const
{
    return m_rdf->model();
}

QString KoSopranoTableModel::URItoPrefexedLocalname(const QString &uri) const
{
    return m_rdf->prefixMapping()->URItoPrefexedLocalname(uri);
}
QString KoSopranoTableModel::PrefexedLocalnameToURI(const QString &pname)
{
    return m_rdf->prefixMapping()->PrefexedLocalnameToURI(pname);
}

QVariant KoSopranoTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    Soprano::Statement st = m_statementIndex[index.row()];
    if (index.column() == ColIsValid && role == Qt::CheckStateRole) {
        return st.isValid();
    }
    if (role == Qt::BackgroundRole) {
        if (!m_statementIndex[index.row()].isValid()) {
            return QColor("#BB0000");
        }
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole) {
        return QVariant();
    }
    switch (index.column()) {
    case ColIsValid:
        return QVariant();
    case ColSubj:
        return URItoPrefexedLocalname(st.subject().toString());
    case ColPred:
        return URItoPrefexedLocalname(st.predicate().toString());
    case ColObj:
        if (st.object().type() == Soprano::Node::ResourceNode)
            return URItoPrefexedLocalname(st.object().toString());
        return st.object().toString();
    case ColObjType:
        switch (st.object().type()) {
        case Soprano::Node::EmptyNode:
            return i18n("Empty");
        case Soprano::Node::ResourceNode:
            return i18n("URI");
        case Soprano::Node::LiteralNode:
            return i18n("Literal");
        case Soprano::Node::BlankNode:
            return i18n("Blank");
        }
        return QString();
    case ColObjXsdType:
        return st.object().dataType().toString();
    case ColCtx: {
        QString ctx = st.context().toString();
        QString RdfPathContextPrefix = m_rdf->rdfPathContextPrefix();
        QString InternalContext = m_rdf->inlineRdfContext().toString();

        kDebug(30015) << "InternalContext:" << InternalContext;
        kDebug(30015) << "ctx:" << ctx;

        if (ctx.startsWith(RdfPathContextPrefix)) {
            ctx = ctx.mid(RdfPathContextPrefix.size());
        }
        if (isInlineRdf(st)) {
            ctx = "inline";
        }
        return ctx;
    }
    }
    return QVariant();
}

int KoSopranoTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return model()->statementCount();
}

int KoSopranoTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return ColCount;
}

QVariant KoSopranoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case ColIsValid:
            return i18n("Valid");
        case ColSubj:
            return i18n("Subject");
        case ColPred:
            return i18n("Predicate");
        case ColObj:
            return i18n("Object");
        case ColObjType:
            return i18n("Obj Type");
        case ColObjXsdType:
            return i18n("DataType");
        case ColCtx:
            return i18n("Stored In");
        }
    }
    return QVariant();
}

bool KoSopranoTableModel::isInlineRdf(Soprano::Statement st) const
{
    return (st.context().toString() == m_rdf->inlineRdfContext().toString());
}

Qt::ItemFlags KoSopranoTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags ret = QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    if (index.column() == ColIsValid) {
        ret |= Qt::ItemIsUserCheckable;
    }
    if (index.row() >= 0) {
        Soprano::Statement st = m_statementIndex[index.row()];
        if (isInlineRdf(st)) {
            if (index.column() == ColSubj
                    || index.column() == ColObjType
                    || index.column() == ColCtx) {
                ret &= (~Qt::ItemIsEditable);
            }
        }
    }
    return ret;
}

/**
 * You MUST use this method if you want to change a Statement.
 *
 * Used by setData() to remove the old statement and replace it with the new 'n' one.
 * The internal m_statementIndex int->statement is updated
 * as well as the dataChanged signal emitted
 */
bool KoSopranoTableModel::setDataUpdateTriple(const QModelIndex &index, Soprano::Statement &old, Soprano::Statement &n)
{
    model()->addStatement(n);
    model()->removeStatement(old);
    m_statementIndex[ index.row()] = n;
    emit dataChanged(index, index);
    return true;
}

bool KoSopranoTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role);
    if (!index.isValid()) {
        return false;
    }
    int r = index.row();
    Soprano::Statement st = m_statementIndex[r];
    QString uri = PrefexedLocalnameToURI(value.toString());
    Soprano::Statement n(st.subject(), st.predicate(), st.object(), st.context());
    switch (index.column()) {
    case ColSubj:
        n.setSubject(Soprano::Node(QUrl(uri)));
        return setDataUpdateTriple(index, st, n);
    case ColPred:
        n.setPredicate(Soprano::Node(QUrl(uri)));
        return setDataUpdateTriple(index, st, n);
    case ColObj: {
        if (st.object().isLiteral()) {
            n.setObject(
                Soprano::Node(
                    Soprano::LiteralValue(value.toString())));
        } else {
            n.setObject(Soprano::Node(QUrl(uri)));
        }
        return setDataUpdateTriple(index, st, n);
    }
    case ColObjType: {
        QString v = value.toString();
        if (v == "URI") {
            n.setObject(Soprano::Node(QUrl(st.object().toString())));
        } else if (v == "Literal") {
            n.setObject(
                Soprano::Node(
                    Soprano::LiteralValue(st.object().toString())));
        } else {
            n.setObject(Soprano::Node(QString(st.object().toString())));
        }
        return setDataUpdateTriple(index, st, n);
    }
    case ColCtx: {
        QString v = value.toString();
        if (v == "inline") {
            QString InternalContext = m_rdf->rdfInternalMetadataWithoutSubjectURI();
            n.setContext(Soprano::Node(QUrl(InternalContext)));
        } else {
            if (!v.endsWith(".rdf"))
                v = v + ".rdf";
            n.setContext(Soprano::Node(QUrl(m_rdf->rdfPathContextPrefix() + v)));
        }
        return setDataUpdateTriple(index, st, n);
    }
    }
    return false;
}

/**
 * Add the statement 'st' to the model as the new last row.
 */
int KoSopranoTableModel::insertStatement(Soprano::Statement st)
{
    QModelIndex parent;
    int newRowNumber = rowCount();
    kDebug(30015) << "insert, newrow:" << newRowNumber << endl;
    beginInsertRows(parent, newRowNumber, newRowNumber);
    model()->addStatement(st);
    m_statementIndex << st;
    endInsertRows();
    return newRowNumber;
}

/**
 * Copy all the triples in srclist to be new rows in the model.
 * Note that the object value is modified to contain a unique
 * postfix so that the new triple copies can be inserted into
 * the Rdf model. It is a copy in a looser sense of the word.
 */
QModelIndexList KoSopranoTableModel::copyTriples(const QModelIndexList &srclist)
{
    QModelIndexList ret;
    int FirstNewRowNumber = rowCount();
    int LastNewRowNumber = FirstNewRowNumber + srclist.size() - 1;
    int currentNewRowNum = FirstNewRowNumber;
    beginInsertRows(QModelIndex(), FirstNewRowNumber, LastNewRowNumber);
    kDebug(30015) << " m_statementIndex.sz:" << m_statementIndex.size();
    kDebug(30015) << " srclist.size():" << srclist.size();
    kDebug(30015) << " first:" << FirstNewRowNumber;
    kDebug(30015) << " last:" << LastNewRowNumber;
    foreach (const QModelIndex &src, srclist) {
        int r = src.row();
        kDebug(30015) << "r:" << r;
        Soprano::Statement st = m_statementIndex[ r ];
        //
        // Append a bnode to the object to ensure the "copy"
        // is unique relative to the original.
        //
        Soprano::Node obj(QUrl(st.object().toString() + '-'
                               + model()->createBlankNode().toString()));
        Soprano::Statement n(st.subject(), st.predicate(),
                             obj, st.context());
        model()->addStatement(n);
        m_statementIndex << n;
        QModelIndex newIdx = index(currentNewRowNum, ColSubj);
        ret << newIdx;
        ++currentNewRowNum;
    }
    endInsertRows();
    return ret;
}

/**
 * Delete all the triples in srclist from the model.
 */
void KoSopranoTableModel::deleteTriples(const QModelIndexList &srclist)
{
    //
    // Because items after a row are shuffled back to fill
    // it's position, it is easiest to remove each item
    // starting at the largest row number and working
    // down by descending row number.
    //
    QList<int> rowsToRemoveDesc;
    foreach (const QModelIndex &src, srclist) {
        int r = src.row();
        rowsToRemoveDesc << r;
    }
    qSort(rowsToRemoveDesc.begin(), rowsToRemoveDesc.end(), qGreater<int>());
    int r;
    foreach (r, rowsToRemoveDesc) {
        Soprano::Statement st = m_statementIndex[ r ];
        int firstRow =  r;
        int lastRow = r;
        beginRemoveRows(QModelIndex(), firstRow, lastRow);
        model()->removeStatement(st);
        // m_statementIndex[ r ] = Soprano::Statement();
        for (int i = r; i < m_statementIndex.size() - 1; ++i) {
            m_statementIndex[ i ] = m_statementIndex[ i + 1 ];
        }
        m_statementIndex.removeLast();
        endRemoveRows();
    }
}

Soprano::Statement KoSopranoTableModel::statementAtIndex(const QModelIndex &index) const
{
    return m_statementIndex[index.row()];
}

int KoSopranoTableModel::invalidStatementCount() const
{
    return invalidStatementList().size();
}

QModelIndexList KoSopranoTableModel::invalidStatementList() const
{
    QModelIndexList ret;
    for (int r = 0; r < m_statementIndex.size(); ++r)  {
        Soprano::Statement s = m_statementIndex[ r ];
        if (!s.isValid()) {
            int col = ColSubj;
            if (!s.subject().isValid()) {
                col = ColSubj;
            }
            if (!s.predicate().isValid()) {
                col = ColPred;
            }
            if (!s.object().isValid()) {
                col = ColObj;
            }
            QModelIndex idx = index(r, col);
            ret << idx;
        }
    }
    return ret;
}

