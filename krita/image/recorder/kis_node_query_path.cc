/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_node_query_path.h"

#include <QStringList>
#include <kis_node.h>
#include <kis_image.h>

struct PathElement {
    enum Type {
        Wildcard,
        Parent,
        Index
    };
    PathElement(Type _type) : type(_type) {
        Q_ASSERT(type == Wildcard || type == Parent);
    }
    PathElement(int _i) : type(Index), index(_i) {}
    Type type;
    unsigned int index;
};

struct KisNodeQueryPath::Private {
    QList<PathElement> elements;
    bool relative;
    /// This function will remove uneeded call to parent, for instance, "1/../3/../5" => "5"
    void simplifyPath() {
        // No elements then return
        if (elements.isEmpty()) return;
        QList<PathElement> newelements;
        int i = 0;
        for (; i < elements.count() && elements[i].type == PathElement::Parent; ++i) {
            newelements.push_back(PathElement::Parent);
        }
        // Loop ofver the element of the list
        for (; i < elements.count(); ++i) {
            PathElement pe = elements[i];
            // If it's the last element, or the next element isn't a parent
            if (pe.type != PathElement::Parent) {
                newelements.push_back(pe);
            } else {
                if (newelements.isEmpty() || newelements.last().type == PathElement::Parent) {
                    newelements.push_back(PathElement::Parent);
                } else {
                    newelements.removeLast();
                }
            }
        }
        // Set the new list
        elements = newelements;
    }
    void queryLevel(int _level, KisNodeSP _node, QList<KisNodeSP>& _result) {
        if (_level < elements.size()) {
            _result.push_back(_node);
        } else {
            PathElement pe = elements[_level];

            switch (pe.type) {
            case PathElement::Wildcard: {
                for (KisNodeSP child = _node->firstChild();
                        child != 0; child = child->nextSibling()) {
                    queryLevel(_level + 1, child, _result);
                }
            }
            break;
            case PathElement::Parent: {
                if (_node->parent()) {
                    queryLevel(_level + 1, _node->parent(), _result);
                } else {
                    kWarning() << "No parent";
                }
                break;
            }
            case PathElement::Index: {
                if (pe.index < _node->childCount()) {
                    queryLevel(_level + 1, _node->at(pe.index), _result);
                } else {
                    kWarning() << "No parent";
                }
                break;
            }
            }
        }
    }
};

KisNodeQueryPath::KisNodeQueryPath() : d(new Private)
{
}

KisNodeQueryPath::~KisNodeQueryPath()
{
}

KisNodeQueryPath::KisNodeQueryPath(const KisNodeQueryPath& nqp) : d(new Private(*nqp.d))
{
}

KisNodeQueryPath& KisNodeQueryPath::operator=(const KisNodeQueryPath & nqp)
{
    *d = *nqp.d;
    return *this;
}

bool KisNodeQueryPath::isRelative() const
{
    return d->relative;
}


QList<KisNodeSP> KisNodeQueryPath::queryNodes(KisImageWSP image, KisNodeSP currentNode) const
{
    KisNodeSP _node;
    if (d->relative) {
        _node = currentNode;
    } else {
        _node = image->root();
    }

    QList<KisNodeSP> result;

    d->queryLevel(0, _node, result);

    return result;
}

QString KisNodeQueryPath::toString() const
{
    QString str = "";
    for (int i = 0; i < d->elements.count(); ++i) {
        PathElement pe = d->elements[i];
        switch (pe.type) {
        case PathElement::Wildcard:
            str += '*';
            break;
        case PathElement::Parent:
            str += "..";
            break;
        case PathElement::Index:
            str += QString::number(pe.index);
            break;
        }
    }
    return str;
}

KisNodeQueryPath KisNodeQueryPath::fromString(const QString& _path)
{
    KisNodeQueryPath path;
    if (_path.size() == 0) return path;
    path.d->relative = !(_path.at(0) == '\\');
    QStringList indexes = _path.split("\\");
    foreach(const QString& index, indexes) {
        if (index == "*") {
            path.d->elements.push_back(PathElement::Wildcard);
        } else if (index == "..") {
            path.d->elements.push_back(PathElement::Parent);
        } else {
            path.d->elements.push_back(index.toInt());
        }
    }
    path.d->simplifyPath();
    return path;
}

KisNodeQueryPath KisNodeQueryPath::absolutePath(KisNodeSP node)
{
    KisNodeQueryPath path;
    path.d->relative = false;
    KisNodeSP parent = 0;
    while ((parent = node->parent())) {
        int index = parent->index(node);
        if (index >= 0) {
            path.d->elements.push_front(index);
        }
        node = parent;
    }
    return path;
}


