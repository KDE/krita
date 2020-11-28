/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_NODE_SELECTION_ADAPTER_H
#define __KIS_NODE_SELECTION_ADAPTER_H

#include <QScopedPointer>
#include "kis_types.h"
#include "kritaui_export.h"

class KisNodeManager;


class KRITAUI_EXPORT KisNodeSelectionAdapter
{
public:
    KisNodeSelectionAdapter(KisNodeManager *nodeManager);
    ~KisNodeSelectionAdapter();

    KisNodeSP activeNode() const;
    void setActiveNode(KisNodeSP node);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_NODE_SELECTION_ADAPTER_H */
