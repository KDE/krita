/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_DUMMIES_FACADE_H
#define __KIS_DUMMIES_FACADE_H


#include "kis_dummies_facade_base.h"

/**
 * The simple implementation of KisDummiesFacadeBase. It can be used
 * in cases when KisShapeController is not accessible, e.g. when you
 * need to show layers in a filter and you need to create KisNodeModel
 */

class KRITAUI_EXPORT KisDummiesFacade : public KisDummiesFacadeBase
{
    Q_OBJECT

public:
    KisDummiesFacade(QObject *parent = 0);
    ~KisDummiesFacade();

    virtual bool hasDummyForNode(KisNodeSP node) const;
    KisNodeDummy* dummyForNode(KisNodeSP node) const;
    KisNodeDummy* rootDummy() const;
    int dummiesCount() const;

private:
    void addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void removeNodeImpl(KisNodeSP node);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_DUMMIES_FACADE_H */
