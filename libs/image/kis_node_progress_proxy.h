/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_NODE_PROGRESS_PROXY_H_
#define _KIS_NODE_PROGRESS_PROXY_H_

#include <KoProgressProxy.h>
#include <QObject>

#include <kis_types.h>

#include "kritaimage_export.h"

/**
 * This class implements \ref KoProgressProxy and allows node to report progress.
 */
class KRITAIMAGE_EXPORT KisNodeProgressProxy : public QObject, public KoProgressProxy
{
    Q_OBJECT
    friend class KisNode;
    /**
     * Create a proxy to report progress when processing, this proxy is associated
     * with a node, it will report progress in the node progress bar. This proxy
     * will be deleted when @p _node is deleted.
     */
    explicit KisNodeProgressProxy(KisNode* _node);
    ~KisNodeProgressProxy() override;

public:
    int maximum() const override;
    void setValue(int value) override;
    void setRange(int minimum, int maximum) override;
    void setFormat(const QString & format) override;
    /**
     * @return the current percentage (return -1 if no progress)
     */
    int percentage() const;
Q_SIGNALS:
    /**
     * Emitted when the percentage of the proxy is changed.
     * @param _percentage is the progress value in percent
     * @param _node is the node that own this \ref KisNodeProgressProxy
     */
    void percentageChanged(int _percentage, const KisNodeSP& _node);

private:
    /**
     * To be called when the node is and will be no longer available
     * and this object is going to be deleted as well.
     */
    void prepareDestroying();

private:
    struct Private;
    Private* const d;
};

#endif
