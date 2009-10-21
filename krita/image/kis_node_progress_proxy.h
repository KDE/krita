/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_NODE_PROGRESS_PROXY_H_
#define _KIS_NODE_PROGRESS_PROXY_H_

#include <KoProgressUpdater.h>
#include <KoProgressProxy.h>
#include <QObject>

#include <kis_types.h>

#include "krita_export.h"

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
    KisNodeProgressProxy(KisNode* _node);
    ~KisNodeProgressProxy();

public:
    /**
     * @return the node associated with this proxy.
     */
    const KisNodeSP node() const;
    virtual int maximum() const ;
    virtual void setValue(int value);
    virtual void setRange(int minimum, int maximum);
    virtual void setFormat(const QString & format);
    /**
     * @return the current percentage (return -1 if no progress)
     */
    int percentage() const;
signals:
    /**
     * Emitted when the percentage of the proxy is changed.
     * @param _percentage is the progress value in percent
     * @param _node is the node that own this \ref KisNodeProgressProxy
     */
    void percentageChanged(int _percentage, const KisNodeSP& _node);
private:
    struct Private;
    Private* const d;
};

#endif
