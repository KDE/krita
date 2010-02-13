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
#include "RdfPrefixMapping.h"
#include <kdebug.h>

KoSopranoTableModel::KoSopranoTableModel(KoDocumentRdf* rdf)
        : m_rdf(rdf)
{
    Soprano::StatementIterator siter = model()->listStatements();
    while (siter.next()) {
        m_statementIndex << *siter;
    }
}

Soprano::Model* KoSopranoTableModel::model() const
{
    return m_rdf->model();
}

QString KoSopranoTableModel::URItoPrefexedLocalname(QString uri) const
{
    return m_rdf->getPrefixMapping()->URItoPrefexedLocalname(uri);
}
QString KoSopranoTableModel::PrefexedLocalnameToURI(QString pname)
{
    return m_rdf->getPrefixMapping()->PrefexedLocalnameToURI(pname);
}

QVariant KoSopranoTableModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    Soprano::Statement st = m_statementIndex[index.row()];
    if (index.column() == COL_ISVALID && role == Qt::CheckStateRole) {
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
    case COL_ISVALID:
        return QVariant();
    case COL_SUBJ:
        return URItoPrefexedLocalname(st.subject().toString());
    case COL_PRED:
        return URItoPrefexedLocalname(st.predicate().toString());
    case COL_OBJ:
        if (st.object().type() == Soprano::Node::ResourceNode)
            return URItoPrefexedLocalname(st.object().toString());
        return st.object().toString();
    case COL_OBJ_TYPE:
        switch (st.object().type()) {
        case Soprano::Node::EmptyNode:
            return tr("Empty");
        case Soprano::Node::ResourceNode:
            return tr("URI");
        case Soprano::Node::LiteralNode:
            return tr("Literal");
        case Soprano::Node::BlankNode:
            return tr("Blank");
        }
        return "";
    case COL_OBJ_XSDTYPE:
        return st.object().dataType().toString();
    case COL_CTX: {
        QString ctx = st.context().toString();
        QString RdfPathContextPrefix = m_rdf->getRdfPathContextPrefix();
        QString InternalContext = m_rdf->getInlineRdfContext().toString();

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

int KoSopranoTableModel::rowCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return model()->statementCount();
}

int KoSopranoTableModel::columnCount(const QModelIndex & parent) const
{
    Q_UNUSED(parent);
    return COL_COUNT;
}

QVariant KoSopranoTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case COL_ISVALID:
            return tr("Valid");
        case COL_SUBJ:
            return tr("Subject");
        case COL_PRED:
            return tr("Predicate");
        case COL_OBJ:
            return tr("Object");
        case COL_OBJ_TYPE:
            return tr("Obj Type");
        case COL_OBJ_XSDTYPE:
            return tr("DataType");
        case COL_CTX:
            return tr("Stored In");
        }
    }
    return QVariant();
}

bool KoSopranoTableModel::isInlineRdf(Soprano::Statement st) const
{
    return (st.context().toString() == m_rdf->getInlineRdfContext().toString());
}

Qt::ItemFlags KoSopranoTableModel::flags(const QModelIndex & index) const
{
    Qt::ItemFlags ret = QAbstractTableModel::flags(index)
                        | Qt::ItemIsEditable;
    if (index.column() == COL_ISVALID) {
        ret |= Qt::ItemIsUserCheckable;
    }
    Soprano::Statement st = m_statementIndex[index.row()];
    if (isInlineRdf(st)) {
        if (index.column() == COL_SUBJ
                || index.column() == COL_OBJ_TYPE
                || index.column() == COL_CTX) {
            ret &= (~Qt::ItemIsEditable);
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
bool KoSopranoTableModel::setDataUpdateTriple(const QModelIndex & index, Soprano::Statement& old, Soprano::Statement& n)
{
    model()->addStatement(n);
    model()->removeStatement(old);
    m_statementIndex[ index.row()] = n;
    emit dataChanged(index, index);
    return true;
}

bool KoSopranoTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    Q_UNUSED(role);
    if (!index.isValid()) {
        return false;
    }
    int r = index.row();
    Soprano::Statement st = m_statementIndex[ r ];
    QString uri = PrefexedLocalnameToURI(value.toString());
    Soprano::Statement n(st.subject(), st.predicate(), st.object(), st.context());
    switch (index.column()) {
    case COL_SUBJ:
        n.setSubject(Soprano::Node(QUrl(uri)));
        return setDataUpdateTriple(index, st, n);
    case COL_PRED:
        n.setPredicate(Soprano::Node(QUrl(uri)));
        return setDataUpdateTriple(index, st, n);
    case COL_OBJ: {
        if (st.object().isLiteral()) {
            n.setObject(
                Soprano::Node(
                    Soprano::LiteralValue(value.toString())));
        } else {
            n.setObject(Soprano::Node(QUrl(uri)));
        }
        return setDataUpdateTriple(index, st, n);
    }
    case COL_OBJ_TYPE: {
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
    case COL_CTX: {
        QString v = value.toString();
        if (v == "inline") {
            QString InternalContext = m_rdf->getRdfInternalMetadataWithoutSubjectURI();
            n.setContext(Soprano::Node(QUrl(InternalContext)));
        } else {
            if (!v.endsWith(".rdf"))
                v = v + ".rdf";
            n.setContext(Soprano::Node(QUrl(m_rdf->getRdfPathContextPrefix() + v)));
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
QModelIndexList KoSopranoTableModel::copyTriples(QModelIndexList srclist)
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
    QModelIndex src;
    foreach (src, srclist) {
        int r = src.row();
        kDebug(30015) << "r:" << r;
        Soprano::Statement st = m_statementIndex[ r ];
        //
        // Append a bnode to the object to ensure the "copy"
        // is unique relative to the original.
        //
        Soprano::Node obj(QUrl(st.object().toString()
                               + "-"
                               + model()->createBlankNode().toString()));
        Soprano::Statement n(st.subject(),
                             st.predicate(),
                             obj, st.context()
                            );
        model()->addStatement(n);
        m_statementIndex << n;
        QModelIndex newIdx = index(currentNewRowNum, COL_SUBJ);
        ret << newIdx;
        ++currentNewRowNum;
    }
    endInsertRows();
    return ret;
}

/**
 * Delete all the triples in srclist from the model.
 */
void KoSopranoTableModel::deleteTriples(QModelIndexList srclist)
{
    //
    // Because items after a row are shuffled back to fill
    // it's position, it is easiest to remove each item
    // starting at the largest row number and working
    // down by descending row number.
    //
    QList< int > rowsToRemoveDesc;
    QModelIndex src;
    foreach (src, srclist) {
        int r = src.row();
        rowsToRemoveDesc << r;
    }
    qSort(rowsToRemoveDesc.begin(),
          rowsToRemoveDesc.end(),
          qGreater<int>());
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

Soprano::Statement KoSopranoTableModel::statementAtIndex(QModelIndex index)
{
    return m_statementIndex[ index.row()];
}

int KoSopranoTableModel::invalidStatementCount()
{
    return invalidStatementList().size();
}

QModelIndexList KoSopranoTableModel::invalidStatementList()
{
    QModelIndexList ret;
    for (int r = 0; r < m_statementIndex.size(); ++r)  {
        Soprano::Statement s = m_statementIndex[ r ];
        if (!s.isValid()) {
            int col = COL_SUBJ;
            if (!s.subject().isValid()) {
                col = COL_SUBJ;
            }
            if (!s.predicate().isValid()) {
                col = COL_PRED;
            }
            if (!s.object().isValid()) {
                col = COL_OBJ;
            }
            QModelIndex idx = index(r, col);
            ret << idx;
        }
    }
    return ret;
}

