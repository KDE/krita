/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2013 Camilla Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISSELECTIONEXTRAS_H
#define KISSELECTIONEXTRAS_H

#include <QObject>

class KisViewManager;

// This class prvides some extra kisselectionmanager stuff that in krita prober is in plugins
class KisSelectionExtras : public QObject
{
    Q_OBJECT
public:
    KisSelectionExtras(KisViewManager *view);
    virtual ~KisSelectionExtras();

    Q_INVOKABLE void grow(qint32 xradius, qint32 yradius);
    Q_INVOKABLE void shrink(qint32 xradius, qint32 yradius, bool edge_lock);
    Q_INVOKABLE void border(qint32 xradius, qint32 yradius);
    Q_INVOKABLE void feather(qint32 radius);

private:
    KisViewManager *m_view;
};

#endif // KISSELECTIONEXTRAS_H
