/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_EXTERNAL_FACTORY_BASE_H
#define __KIS_EXTERNAL_FACTORY_BASE_H

#include <kglobal.h>
#include "kis_debug.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <QScopedPointer>
#include "krita_export.h"


template <class FactoryObject, typename FactoryParam, class FactoryClass>
class KRITAIMAGE_EXPORT KisExternalFactoryBase
{
public:
    typedef boost::function<FactoryObject (FactoryParam)> Factory;

public:
    void setFactory(Factory factory) {
        KIS_ASSERT_RECOVER_NOOP(!m_factory);
        m_factory = factory;
    }

    FactoryObject create(FactoryParam param) const {
        KIS_ASSERT_RECOVER(m_factory) {
            return FactoryObject();
        }

        return m_factory(param);
    }

protected:
    static FactoryClass* instanceImpl() {
        K_GLOBAL_STATIC(FactoryClass , s_instance);
        return s_instance;
    }

private:
        Factory m_factory;
};

#endif /* __KIS_EXTERNAL_FACTORY_BASE_H */
