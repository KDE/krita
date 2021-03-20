/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    ~KisDummiesFacade() override;

    bool hasDummyForNode(KisNodeSP node) const override;
    KisNodeDummy* dummyForNode(KisNodeSP node) const override;
    KisNodeDummy* rootDummy() const override;
    int dummiesCount() const override;

private:
    void addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis) override;
    void removeNodeImpl(KisNodeSP node) override;

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_DUMMIES_FACADE_H */
