/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <testutil.h>

namespace TestUtil
{
    QStringList getHierarchy(KisNodeSP root, const QString &prefix) {
        QStringList list;

        QString nextPrefix;
        if (root->parent()) {
            nextPrefix = prefix + "+";
            list << prefix + root->name();
        }

        KisNodeSP node = root->firstChild();
        while (node) {
            list += getHierarchy(node, nextPrefix);
            node = node->nextSibling();
        }

        return list;
    }

    bool checkHierarchy(KisNodeSP root, const QStringList &expected)
    {
        QStringList result = getHierarchy(root);
        if (result != expected) {
            qDebug() << "Failed to compare hierarchy:";
            qDebug() << "   " << ppVar(result);
            qDebug() << "   " << ppVar(expected);
            return false;
        }

        return true;
    }
}
