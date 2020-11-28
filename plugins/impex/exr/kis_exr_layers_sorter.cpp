/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_exr_layers_sorter.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_image.h"
#include "exr_extra_tags.h"
#include "kis_kra_savexml_visitor.h"
#include "kis_paint_layer.h"


struct KisExrLayersSorter::Private
{
    Private(const QDomDocument &_extraData, KisImageSP _image)
        : extraData(_extraData), image(_image) {}

    const QDomDocument &extraData;
    KisImageSP image;

    QMap<QString, QDomElement> pathToElementMap;
    QMap<QString, int> pathToOrderingMap;

    QMap<KisNodeSP, int> nodeToOrderingMap;

    void createOrderingMap();
    void processLayers(KisNodeSP root);
    void sortLayers(KisNodeSP root);
};

QString getNodePath(KisNodeSP node) {
    KIS_ASSERT_RECOVER(node) { return "UNDEFINED"; }

    QString path;

    KisNodeSP parentNode = node->parent();
    while(parentNode) {
        if (!path.isEmpty()) {
            path.prepend(".");
        }
        path.prepend(node->name());

        node = parentNode;
        parentNode = node->parent();
    }

    return path;
}

void KisExrLayersSorter::Private::createOrderingMap()
{
    int index = 0;
    QDomElement el = extraData.documentElement().firstChildElement();


    while (!el.isNull()) {
        QString path = el.attribute(EXR_NAME);
        pathToElementMap.insert(path, el);
        pathToOrderingMap.insert(path, index);

        el = el.nextSiblingElement();
        index++;
    }
}

template <typename T>
T fetchMapValueLazy(const QMap<QString, T> &map, QString path)
{
    if (map.contains(path)) return map[path];


    typename QMap<QString, T>::const_iterator it = map.constBegin();
    typename QMap<QString, T>::const_iterator end = map.constEnd();

    for (; it != end; ++it) {
        if (it.key().startsWith(path)) {
            return it.value();
        }
    }

    return T();
}

void KisExrLayersSorter::Private::processLayers(KisNodeSP root)
{
    if (root && root->parent()) {
        QString path = getNodePath(root);

        nodeToOrderingMap.insert(root, fetchMapValueLazy(pathToOrderingMap, path));

        if (KisPaintLayer *paintLayer = dynamic_cast<KisPaintLayer*>(root.data())) {
            KisSaveXmlVisitor::loadPaintLayerAttributes(pathToElementMap[path], paintLayer);
        }
    }

    KisNodeSP child = root->firstChild();
    while (child) {
        processLayers(child);
        child = child->nextSibling();
    }
}

struct CompareNodesFunctor
{
    CompareNodesFunctor(const QMap<KisNodeSP, int> &map)
        : m_nodeToOrderingMap(map) {}

    bool operator() (KisNodeSP lhs, KisNodeSP rhs) {
        return m_nodeToOrderingMap[lhs] < m_nodeToOrderingMap[rhs];
    }

private:
    const QMap<KisNodeSP, int> &m_nodeToOrderingMap;
};


void KisExrLayersSorter::Private::sortLayers(KisNodeSP root)
{
    QList<KisNodeSP> childNodes;

    // first move all the children to the list
    KisNodeSP child = root->firstChild();
    while (child) {
        KisNodeSP lastChild = child;
        child = child->nextSibling();

        childNodes.append(lastChild);
        image->removeNode(lastChild);
    }

    // sort the list
    std::stable_sort(childNodes.begin(), childNodes.end(), CompareNodesFunctor(nodeToOrderingMap));

    // put the children back
    Q_FOREACH (KisNodeSP node, childNodes) {
        image->addNode(node, root, root->childCount());
    }

    // recursive calls
    child = root->firstChild();
    while (child) {
        sortLayers(child);
        child = child->nextSibling();
    }
}

KisExrLayersSorter::KisExrLayersSorter(const QDomDocument &extraData, KisImageSP image)
    : m_d(new Private(extraData, image))
{
    KIS_ASSERT_RECOVER_RETURN(!extraData.isNull());
    m_d->createOrderingMap();

    m_d->processLayers(image->root());
    m_d->sortLayers(image->root());
}

KisExrLayersSorter::~KisExrLayersSorter()
{
}
