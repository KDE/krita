/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_NODE_QUERY_PATH_EDITOR_H_
#define _KIS_NODE_QUERY_PATH_EDITOR_H_

#include <QWidget>

class KisNodeQueryPath;
/**
 * This class is used to edit @ref KisNodeQueryPath
 */
class KisNodeQueryPathEditor : public QWidget
{
    Q_OBJECT
public:
    KisNodeQueryPathEditor(QWidget* parent);
    ~KisNodeQueryPathEditor() override;
    void setNodeQueryPath(const KisNodeQueryPath& path);
    /**
     * Generate a node query path based on the state of the widgets
     */
    KisNodeQueryPath nodeQueryPath() const;
Q_SIGNALS:
    void nodeQueryPathChanged();
private Q_SLOTS:
    void currentLayerEnabled(bool v);
    void customPathEnabled(bool v);
    void slotPopupQuickHelp();
private:
    struct Private;
    Private* const d;
};

#endif
