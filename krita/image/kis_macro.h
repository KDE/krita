/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_MACRO_H_
#define _KIS_MACRO_H_

#include <QList>
#include <QObject>
#include <krita_export.h>

#include "kis_serializable_configuration.h"
#include "kis_types.h"

class KisRecordedAction;

class KRITAIMAGE_EXPORT KisMacro : public QObject, public KisSerializableConfiguration {
    Q_OBJECT
    public:
        KisMacro(KisImageSP _image);
        KisMacro(KisImageSP _image, const QList<KisRecordedAction*>& _actions);
    public:
        void appendActions(const QList<KisRecordedAction*>& actions);
    public:
        void play();
    public: // serialization functions
        virtual void fromXML(const QDomElement&);
        virtual void toXML(QDomDocument&, QDomElement&) const;
    public slots:
        virtual void addAction(const KisRecordedAction& action);
    private:
        struct Private;
        Private* const d;
};

#endif
