/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef BUGINFO_H
#define BUGINFO_H

#include <QVariant>
#include <KisActionPlugin.h>

class KUndo2MagicString;

class BugInfo : public KisActionPlugin
{
    Q_OBJECT
public:
    BugInfo(QObject *parent, const QVariantList &);
    ~BugInfo() override;

public Q_SLOTS:

    void slotBugInfo();

};

#endif // BUGINFO_H
