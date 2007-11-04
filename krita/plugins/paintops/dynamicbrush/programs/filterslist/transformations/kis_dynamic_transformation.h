/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _KISDYNAMICTRANSFORMATION_H_
#define _KISDYNAMICTRANSFORMATION_H_

#include "dynamicbrush_export.h"

#include <QObject>
#include <KoID.h>

#include "kis_serializable_configuration.h"

class QWidget;
class KisPaintInformation;
class KisDynamicShape;
class KisDynamicColoring;

/**
 * This is the base class for transformation.
 * 
 */
class DYNAMIC_BRUSH_EXPORT KisDynamicTransformation : public QObject, public KisSerializableConfiguration {
    Q_OBJECT
    public:
        static KoID SizeTransformationID;
        static KoID RotationTransformationID;
        static KoID DarkenTransformationID;
    public:
        KisDynamicTransformation(const KoID& name) : m_name(name) {}
        virtual ~KisDynamicTransformation() { }
        virtual QString id() const { return m_name.id(); }
        virtual QString name() const { return m_name.name(); }
        virtual void transformBrush(KisDynamicShape* dabsrc, const KisPaintInformation& info) =0;
        virtual void transformColoring(KisDynamicColoring* dabsrc, const KisPaintInformation& info) =0;
        virtual QWidget* createConfigWidget(QWidget* parent) =0;
        virtual void toXML(QDomDocument&, QDomElement&) const;
        virtual void fromXML(const QDomElement&);
    private:
        KoID m_name;
};

#endif
