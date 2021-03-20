/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
