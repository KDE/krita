/* This file is part of the KDE project
 * Copyright (C) 2008 Thomas Zander <zander@kde.org>
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
#include "StylesModel.h"
#include <QSet>
#include <QDebug>
#include <QSignalMapper>

#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoCharacterStyle.h>

#include <KIcon>

StylesModel::StylesModel(KoStyleManager *manager, QObject *parent)
        : QAbstractItemModel(parent),
        m_styleManager(0),
        m_currentParagraphStyle(0),
        m_currentCharacterStyle(0),
        m_pureParagraphStyle(true),
        m_pureCharacterStyle(true),
        m_styleMapper(new QSignalMapper(this))
{
    setStyleManager(manager);
    m_paragIcon = KIcon("kotext-paragraph");
    m_charIcon = KIcon("kotext-character");
    connect(m_styleMapper, SIGNAL(mapped(int)), this, SLOT(updateName(int)));
}

StylesModel::~StylesModel()
{
}

void StylesModel::recalculate()
{
    m_relations.clear();
    if (m_styleManager == 0) {
        m_styleList.clear();
        reset();
        return;
    }

    QList<int> treeRoot;
    QSet<int> paragraphStyles;
    QSet<int> characterStyles;
    treeRoot << m_styleManager->defaultParagraphStyle()->styleId();
    paragraphStyles << treeRoot[0];
    foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles()) {
        KoParagraphStyle *root = style;
        while (root->parentStyle()) {
            const int key = root->parentStyle()->styleId();
            // the multiHash has the nasty habit or returning an inverted list, so lets 'sort in' by inserting them again
            QList<int> prevValues = m_relations.values(key);
            m_relations.remove(key);
            m_relations.insert(key, root->styleId());
            while (!prevValues.isEmpty())
                m_relations.insert(key, prevValues.takeLast());

            characterStyles << root->characterStyle()->styleId();
            root = root->parentStyle();
        }
        Q_ASSERT(root);
        Q_ASSERT(root->characterStyle());
        characterStyles << root->characterStyle()->styleId();
        if (!paragraphStyles.contains(root->styleId())) {
            int index = 0;
            foreach (int styleId, treeRoot) { // sort in sorting
                // default style is 100 and should be sorted at the top.
                if (styleId != 100 && m_styleManager->paragraphStyle(styleId)->name() > root->name())
                    break;
                index++;
            }
            treeRoot.insert(index, root->styleId());
        }
        paragraphStyles << root->styleId();
    }

    foreach(KoCharacterStyle *style, m_styleManager->characterStyles()) {
        if (! characterStyles.contains(style->styleId()))
            treeRoot << style->styleId();
    }

    int firstChangedRow = -1;
    int index = 0;
    foreach(int rootId, treeRoot) {
        if (index >= m_styleList.count()) {
            if (firstChangedRow == -1)
                firstChangedRow = index;
            break;
        }
        if (m_styleList[index] != rootId) {
            firstChangedRow = index;
            break;
        }
    }

    if (m_styleList.count() == treeRoot.count()) {
        int maxRow = qMax(m_styleList.count(), treeRoot.count()) - 1;
        m_styleList = treeRoot;
        emit dataChanged(createIndex(firstChangedRow, 0, 0), createIndex(maxRow, 1, 0));
    } else {
        m_styleList = treeRoot;
        layoutChanged();
    }
}

QModelIndex StylesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || row < 0 || column > 1)
        return QModelIndex();

    if (! parent.isValid()) {
        if (row >= m_styleList.count())
            return QModelIndex();
        return createIndex(row, column, m_styleList[row]);
    }
    int id = (int) parent.internalId();
    KoParagraphStyle *pstyle = m_styleManager->paragraphStyle(id);
    if (pstyle == 0) { // that means it has to be a charStyle. Thats easy, there is no hierarchy there.
        if (row >= m_styleList.count())
            return QModelIndex();
        return createIndex(row, column, m_styleList[row]);
    }

    if (row == 0) // return child char style
        return createIndex(row, column, pstyle->characterStyle()->styleId());
    if (m_relations.contains(id)) {
        QList<int> children = m_relations.values(id);
        if (row > children.count())
            return QModelIndex();
        return createIndex(row, column, children[row-1]);
    }
    return QModelIndex();
}

QModelIndex StylesModel::parent(const QModelIndex &child) const
{
    if (child.isValid()) {
        int id = (int) child.internalId();
        if (m_styleList.contains(id)) // is the root, parent is invalid.
            return QModelIndex();
        KoParagraphStyle *childStyle = m_styleManager->paragraphStyle(id);
        if (childStyle && childStyle->parentStyle())
            createIndex(0, 0, childStyle->parentStyle()->styleId());

        // this is stupid; forcing me to return a parent implies I can't have one node multiple times in a tree!
        // and most real-life data models actually don't allow traversal in two ways :(
        // *sigh* lets just return the first one...
        return parent(id, m_styleList);
    }
    return QModelIndex();
}

QModelIndex StylesModel::parent(int needle, const QList<int> &haystack) const
{
    Q_ASSERT(haystack.count());
    int row = -1;
    foreach(int id, haystack) {
        row++;
        KoParagraphStyle *style = m_styleManager->paragraphStyle(id);
        if (style == 0)
            continue;
        if (style->characterStyle()->styleId() == needle) // found it!
            return createIndex(row, 0, style->styleId());
        QList<int> children = m_relations.values(id);
        if (children.isEmpty())
            continue;
        int index = children.indexOf(needle);
        if (index >= 0)
            return createIndex(row, 0, id);
        children.insert(0, style->characterStyle()->styleId());
        QModelIndex mi = parent(needle, children);
        if (mi.isValid())
            return mi;
    }

    return QModelIndex();
}

int StylesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_styleList.count();
    if (parent.column() == 1)
        return 0;
    int id = (int) parent.internalId();
    const bool isParagStyle = m_styleManager->paragraphStyle(id) != 0;
    if (isParagStyle)
        return m_relations.values(id).count() + 1;
    return 0;
}

int StylesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant StylesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int id = (int) index.internalId();
    switch (role) {
    case Qt::DisplayRole: {
        if (index.column() == 1) {
            if (id == m_currentParagraphStyle) {
                if (m_pureParagraphStyle)
                    return QString(QChar(0x25CF));
                return QString(QChar(0x25D0));
            }
            if (id == m_currentCharacterStyle) {
                if (m_pureCharacterStyle)
                    return QString(QChar(0x25CF));
                return QString(QChar(0x25D0));
            }
            return QString(QChar(0x25CC));
        }
        KoParagraphStyle *paragStyle = m_styleManager->paragraphStyle(id);
        if (paragStyle)
            return paragStyle->name();
        KoCharacterStyle *characterStyle =  m_styleManager->characterStyle(id);
        if (characterStyle)
            return characterStyle->name();
        break;
    }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (m_styleManager->paragraphStyle(id))
                return m_paragIcon;
            return m_charIcon;
        }
        break;
    default: break;
    };
    return QVariant();
}

bool StylesModel::hasChildren(const QModelIndex &parent) const
{
    return rowCount(parent) > 0;
}

Qt::ItemFlags StylesModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return (Qt::ItemIsDragEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void StylesModel::setCurrentParagraphStyle(int styleId, bool unchanged)
{
    if (m_currentParagraphStyle == styleId && unchanged == m_pureParagraphStyle)
        return;
    QModelIndex prev = parent(styleId, m_styleList);
    m_currentParagraphStyle = styleId;
    m_pureParagraphStyle = unchanged;

    if (prev.isValid())
        emit dataChanged(prev, prev);
    QModelIndex newCurrent = parent(styleId, m_styleList);
    emit dataChanged(newCurrent, newCurrent);
}

void StylesModel::setCurrentCharacterStyle(int styleId, bool unchanged)
{
    if (m_currentCharacterStyle == styleId && unchanged == m_pureCharacterStyle)
        return;
    QModelIndex prev = parent(styleId, m_styleList);
    m_currentCharacterStyle = styleId;
    m_pureCharacterStyle = unchanged;
    if (prev.isValid())
        emit dataChanged(prev, prev);
    QModelIndex newCurrent = parent(styleId, m_styleList);
    emit dataChanged(newCurrent, newCurrent);
}

KoParagraphStyle *StylesModel::paragraphStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->paragraphStyle(index.internalId());
}

KoCharacterStyle *StylesModel::characterStyleForIndex(const QModelIndex &index) const
{
    return m_styleManager->characterStyle(index.internalId());
}

void StylesModel::setStyleManager(KoStyleManager *sm)
{
    if (sm == m_styleManager)
        return;
    if (m_styleManager) {
        disconnect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));
        disconnect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    }
    m_styleManager = sm;

    if (m_styleManager == 0) {
        recalculate();
        return;
    }

    connect(sm, SIGNAL(styleAdded(KoCharacterStyle*)), this, SLOT(addCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoCharacterStyle*)), this, SLOT(removeCharacterStyle(KoCharacterStyle*)));
    connect(sm, SIGNAL(styleAdded(KoParagraphStyle*)), this, SLOT(addParagraphStyle(KoParagraphStyle*)));
    connect(sm, SIGNAL(styleRemoved(KoParagraphStyle*)), this, SLOT(removeParagraphStyle(KoParagraphStyle*)));

    foreach(KoParagraphStyle *style, m_styleManager->paragraphStyles())
        addParagraphStyle(style, false);
    foreach(KoCharacterStyle *style, m_styleManager->characterStyles())
        addCharacterStyle(style, false);

    recalculate();
}

// called when the stylemanager adds a style
void StylesModel::addParagraphStyle(KoParagraphStyle *style, bool recalc)
{
    Q_ASSERT(style);
    if (recalc)
        recalculate();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager adds a style
void StylesModel::addCharacterStyle(KoCharacterStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->setMapping(style, style->styleId());
    connect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeParagraphStyle(KoParagraphStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

// called when the stylemanager removes a style
void StylesModel::removeCharacterStyle(KoCharacterStyle *style, bool recalc)
{
    if (recalc)
        recalculate();
    m_styleMapper->removeMappings(style);
    disconnect(style, SIGNAL(nameChanged(const QString&)), m_styleMapper, SLOT(map()));
}

void StylesModel::updateName(int styleId)
{
    // TODO, no idea how to do this more correct for children...
    int row = m_styleList.indexOf(styleId);
    if (row >= 0) {
        QModelIndex index = createIndex(row, 0, styleId);
        emit dataChanged(index, index);
    }
}
