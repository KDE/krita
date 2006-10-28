/*
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
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
#ifndef PROGRESSUPDATER_P_H
#define PROGRESSUPDATER_P_H

#include <QObject>

class KisProgressUpdater;

/// @internal
/// the private member for the KisUpdater
class KisUpdaterPrivate : public QObject {
    Q_OBJECT
public:
    KisUpdaterPrivate(KisProgressUpdater *parent, int weight);

    inline bool interrupted() const { return m_interrupted; }
    inline int progress() const { return m_progress; }
    inline int weight() const { return m_weight; }
    void cancel();
    inline void interrupt() { m_interrupted = true; }

    void setProgress(int percent);

private:
    int m_progress; // always in percent
    int m_weight;
    bool m_interrupted;
    KisProgressUpdater *m_parent;
};

#endif
