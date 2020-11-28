/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISGLOBALRESOURCESINTERFACE_H
#define KISGLOBALRESOURCESINTERFACE_H

#include "kritaresources_export.h"
#include "KisResourcesInterface.h"

/**
 * @brief the main resource source in Krita
 *
 * This class wraps KisResourceModel into a KisResourcesInterface and provides
 * all Krita resources to consumers.
 *
 * WARNING: this class should never be accessed in non-GUI thread
 */
class KRITARESOURCES_EXPORT KisGlobalResourcesInterface : public KisResourcesInterface
{
public:
    static KisResourcesInterfaceSP instance();

protected:
    ResourceSourceAdapter* createSourceImpl(const QString &type) const override;
};

#endif // KISGLOBALRESOURCESINTERFACE_H
