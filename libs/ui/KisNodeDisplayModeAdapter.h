/*
 *  SPDX-FileCopyrightText: 2018 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
