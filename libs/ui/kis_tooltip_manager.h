/*
 *  SPDX-FileCopyrightText: 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISTOOLTIPMANAGER_H
#define KISTOOLTIPMANAGER_H

#include <QObject>
#include <QMap>

class KisViewManager;
class KisTooltipManager : public QObject
{
    Q_OBJECT

public:
    KisTooltipManager(KisViewManager* view);
    ~KisTooltipManager() override;

    void record();

private Q_SLOTS:
    void captureToolip();

private:
    KisViewManager* m_view;
    bool m_recording;
    QMap<QString, QString> m_tooltipMap;
};

#endif // KISTOOLTIPMANAGER_H
