/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBATCHNODEUPDATE_H
#define KISBATCHNODEUPDATE_H

#include "kritaimage_export.h"
#include <QSharedPointer>
#include <kis_types.h>
#include <boost/operators.hpp>

/**
 * A simple class for storing the updates of multiple nodes
 * in a single place. These updates may later be "compressed",
 * which means that only the topmost root layers will get updates.
 * In such a case the update should be issued as refreshGraphAsync().
 */
class KRITAIMAGE_EXPORT KisBatchNodeUpdate
        : public std::vector<std::pair<KisNodeSP, QRect>>,
        boost::orable<KisBatchNodeUpdate>
{
public:
    KisBatchNodeUpdate() = default;
    KisBatchNodeUpdate(const KisBatchNodeUpdate &rhs) = default;
    KisBatchNodeUpdate(KisBatchNodeUpdate &&rhs) = default;
    KisBatchNodeUpdate& operator=(const KisBatchNodeUpdate &rhs) = default;

    KisBatchNodeUpdate(const std::vector<std::pair<KisNodeSP, QRect>> &rhs);

    /**
     * Add an update designated for \p node with dirty rect \p rc
     *
     * Please node that adding multiple updates for the same node,
     * will result in multiple records added into the internal
     * vector. These duplicated records may be resolved by calling
     * compress() method.
     *
     * \see compress()
     */
    void addUpdate(KisNodeSP node, const QRect &rc);

    /**
     * Compress the stored updates:
     *
     * 1) All updates for the same node will be merged into one
     *
     * 2) If the list contains a child and its parent, the two
     *    updates will be merged into one. This new update
     *    record will be designated for the parent.
     *
     *    The idea is that the parent will be updated with
     *    refreshGraphAsync(), which would update the child
     *    anyway.
     */
    void compress();

    /**
     * \see compress()
     */
    KisBatchNodeUpdate compressed() const;

    /**
     * Merge two update batches. The updates for the same nodes
     * will be merged. This merge operation does **not** do
     * parent-child compression though. You need to call compress()
     * separately for that.
     */
    KisBatchNodeUpdate& operator|(KisBatchNodeUpdate &rhs);

};

using KisBatchNodeUpdateSP = QSharedPointer<KisBatchNodeUpdate>;
using KisBatchNodeUpdateWSP = QWeakPointer<KisBatchNodeUpdate>;

#endif // KISBATCHNODEUPDATE_H
