/* This file is part of the KDE project
 * Copyright (C) 2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
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

#include "ValidParentStylesProxyModel.h"

#include <QModelIndex>

#include <KoParagraphStyle.h>
#include <KoStyleManager.h>

#include <QDebug>

ValidParentStylesProxyModel::ValidParentStylesProxyModel(QObject *parent)
    : StylesFilteredModelBase(parent)
    , m_styleManager(0)
{
}

void ValidParentStylesProxyModel::setStyleManager(KoStyleManager *sm)
{
    Q_ASSERT(sm);

    m_styleManager = sm;
}

void ValidParentStylesProxyModel::createMapping()
{

    if (!m_styleManager || !m_sourceModel) {
        return;
    }
    m_sourceToProxy.clear();
    m_proxyToSource.clear();

    for (int i = 0; i < m_sourceModel->rowCount(QModelIndex()); ++i) {
        QModelIndex index = m_sourceModel->index(i, 0, QModelIndex());
        int id = (int)index.internalId();
        KoParagraphStyle *paragraphStyle = m_styleManager->paragraphStyle(id);
        if (paragraphStyle) {
            bool ok = true;
            KoParagraphStyle *testStyle = paragraphStyle;
            while (testStyle && ok) {
                ok = testStyle->styleId() != m_currentChildStyleId;
                testStyle = testStyle->parentStyle();
            }
            if (!ok) {
                continue; //we cannot inherit ourself even indirectly through the parent chain
            }
            m_proxyToSource.append(i); //the style is ok for parenting
        } else {
            KoCharacterStyle *characterStyle = m_styleManager->characterStyle(id);
            if (characterStyle) {
                bool ok = true;
                KoCharacterStyle *testStyle = characterStyle;
                while (testStyle && ok) {
                    ok = testStyle->styleId() != m_currentChildStyleId;
                    testStyle = testStyle->parentStyle();
                }
                if (!ok) {
                    continue; //we cannot inherit ourself even indirectly through the parent chain
                }
                m_proxyToSource.append(i); //the style is ok for parenting
            }
        }
    }
    m_sourceToProxy.fill(-1, m_sourceModel->rowCount(QModelIndex()));
    for (int i = 0; i < m_proxyToSource.count(); ++i) {
        m_sourceToProxy[m_proxyToSource.at(i)] = i;
    }
}

void ValidParentStylesProxyModel::setCurrentChildStyleId(int styleId)
{
    m_currentChildStyleId = styleId;
    emit layoutAboutToBeChanged();
    createMapping();
    emit layoutChanged();
}
