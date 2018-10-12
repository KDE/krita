/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISNODEDISPLAYMODEADAPTER_H
#define KISNODEDISPLAYMODEADAPTER_H

#include "kritaui_export.h"
#include <QObject>

class KRITAUI_EXPORT KisNodeDisplayModeAdapter : public QObject
{
    Q_OBJECT
public:
    KisNodeDisplayModeAdapter(QObject *parent = 0);

    bool showRootNode() const;
    void setShowRootNode(bool value);

    bool showGlobalSelectionMask() const;
    void setShowGlobalSelectionMask(bool value);

Q_SIGNALS:
    void sigNodeDisplayModeChanged(bool showRootNode, bool showGlobalSelectionMask);

private Q_SLOTS:
    void slotSettingsChanged();

private:
    void slotSettingsChangedImpl(bool suppressSignals);

private:
    bool m_showRootNode = false;
    bool m_showGlobalSelectionMask = false;
};

#endif // KISNODEDISPLAYMODEADAPTER_H
