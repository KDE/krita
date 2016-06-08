/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_OUTLINE_GENERATION_POLICY_H
#define __KIS_OUTLINE_GENERATION_POLICY_H

#include <kis_current_outline_fetcher.h>

/**
 * This is a policy class that adds an ability to have a Outline Generator
 * to a KisPaintOpSettings-based class.
 *
 * \see Andrei Alexandrescu "Modern C++ Design: Generic Programming and Design Patterns Applied"
 */
template <class ParentClass>
class KisOutlineGenerationPolicy : public ParentClass
{
public:
    KisOutlineGenerationPolicy(KisCurrentOutlineFetcher::Options options)
        : m_outlineFetcher(options) {
    }

    virtual ~KisOutlineGenerationPolicy() {
    }

    const KisCurrentOutlineFetcher* outlineFetcher() const {
        return &m_outlineFetcher;
    }

    void onPropertyChanged() {
        m_outlineFetcher.setDirty();
        ParentClass::onPropertyChanged();
    }

private:
    KisCurrentOutlineFetcher m_outlineFetcher;
};

#endif /* __KIS_OUTLINE_GENERATION_POLICY_H */
