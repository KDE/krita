/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __KoChangeTrackerDisabledRAII_h__
#define __KoChangeTrackerDisabledRAII_h__

class KoChangeTracker;

/**
 * @short Disable a change tracker and automatically reset it when
 *        this object is destroyed.
 *
 * Resource Acquisition Is Initialization pattern to temporarily
 * disable the ChangeTracker. Useful for cases where high level
 * activities like applying a semantic stylesheet are performed where
 * you might like to add a higher level action to the change tracker
 * than just text substitution.
 *
 * @author Ben Martin <ben.martin@kogmbh.com>
 * @see KoChangeTracker
 *
 */
class KoChangeTrackerDisabledRAII
{
public:
    explicit KoChangeTrackerDisabledRAII(KoChangeTracker *changeTracker);
    ~KoChangeTrackerDisabledRAII();

private:
    KoChangeTracker *m_changeTracker;
    bool m_oldval;
};

#endif
