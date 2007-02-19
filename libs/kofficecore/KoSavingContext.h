/* This file is part of the KDE project
 * Copyright (c) 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KOSAVINGCONTEXT_H
#define KOSAVINGCONTEXT_H

#include <kofficecore_export.h>

class KoGenStyles;

class KOFFICECORE_EXPORT KoSavingContext
{
public:    
    enum SavingMode { Store, Flat };
    
    /**
     * Constructor
     * @param mainStyles
     * @param savingMode either Store (a KoStore will be used) or Flat (all data must be inline in the XML)
     */
    explicit KoSavingContext( KoGenStyles& mainStyles, SavingMode savingMode = Store );

    ~KoSavingContext();

    KoGenStyles& mainStyles() { return m_mainStyles; }
    
    /// @return the saving mode: Store (a KoStore will be used) or Flat (all data must be inline in the XML)
    SavingMode savingMode() const { return m_savingMode; }

private:
    KoGenStyles& m_mainStyles;
    SavingMode m_savingMode;

    class Private;
    Private * const d;
};

#endif /* KOSAVINGCONTEXT_H */


