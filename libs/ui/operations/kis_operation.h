/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_OPERATION_H
#define __KIS_OPERATION_H

#include <QString>
#include <kundo2magicstring.h>
#include <kritaui_export.h>
#include "kis_properties_configuration.h"
#include "operations/kis_operation_configuration.h"

class KisViewManager;
class KisProcessingApplicator;

class KRITAUI_EXPORT KisOperation
{
public:
    KisOperation(const QString &id);
    virtual ~KisOperation();

    QString id() const;

    virtual void runFromXML(KisViewManager *view, const KisOperationConfiguration &config);

protected:
    KisProcessingApplicator* beginAction(KisViewManager *view, const KUndo2MagicString &actionName);
    void endAction(KisProcessingApplicator *applicator, const QString &xmlData);
private:
    const QString m_id;
};

#endif /* __KIS_OPERATION_H */
