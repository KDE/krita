/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_DYNAMIC_PROGRAM_H_
#define _KIS_DYNAMIC_PROGRAM_H_

#include "dynamicbrush_export.h"

#include "kis_serializable_configuration.h"

#include <QString>
#include <QObject>

class KisDynamicShape;
class KisDynamicColoring;
class KisPaintInformation;
class QWidget;
class KoID;

/**
 * This is the base class of a dynamic program.
 */
class DYNAMIC_BRUSH_EXPORT KisDynamicProgram : public QObject, public KisSerializableConfiguration {
    Q_OBJECT
    protected:
        /**
         * @param name the name of this program, will be displayed in the list of programs
         */
        KisDynamicProgram(const QString& name, const QString& type);
    public:
        virtual ~KisDynamicProgram();
        /// @return the name of this program
        QString name() const;
        QString id() const;
        QString type() const;
        /**
         * Create and return an editor for that program.
         * @return a QWidget which display editing option for that program
         */
        virtual QWidget* createEditor(QWidget* parent) = 0;
        virtual void fromXML(const QDomElement&);
        virtual void toXML(QDomDocument&, QDomElement&) const;
    signals:
        void programChanged();
    private:
        struct Private;
        Private* const d;
};

class DYNAMIC_BRUSH_EXPORT KisDynamicProgramFactory {
    public:
        KisDynamicProgramFactory(QString id, QString name);
        virtual ~KisDynamicProgramFactory();
        QString id() const;
        QString name() const;
        virtual KisDynamicProgram* program(QString name) const = 0;
    private:
        struct Private;
        Private* const d;
};

class DYNAMIC_BRUSH_EXPORT KisDynamicProgramFactoryRegistry {
    public:
        virtual ~KisDynamicProgramFactoryRegistry();
        virtual KisDynamicProgramFactory* programFactory(QString id) const =0;
        virtual QList<KoID> programTypes() const =0;
};

#endif
