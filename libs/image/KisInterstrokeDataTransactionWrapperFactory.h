/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISINTERSTROKEDATATRANSACTIONWRAPPERFACTORY_H
#define KISINTERSTROKEDATATRANSACTIONWRAPPERFACTORY_H

#include <QScopedPointer>
#include "KisTransactionWrapperFactory.h"

class KisInterstrokeDataFactory;

/**
 * A factory object to extend the behavior of a normal transaction and
 * handle management of interstroke data. It will create a new interstorke
 * data using passed passed `KisInterstrokeDataFactory` or reset it if the
 * current transaction is incompatible with it.
 *
 * \see KisTransactionWrapperFactory
 * \see KisInterstrokeDataFactory
 */
class KRITAIMAGE_EXPORT KisInterstrokeDataTransactionWrapperFactory : public KisTransactionWrapperFactory
{
public:
    KisInterstrokeDataTransactionWrapperFactory(KisInterstrokeDataFactory *factory);
    ~KisInterstrokeDataTransactionWrapperFactory() override;

    KUndo2Command* createBeginTransactionCommand(KisPaintDeviceSP device) override;
    KUndo2Command* createEndTransactionCommand() override;

private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KISINTERSTROKEDATATRANSACTIONWRAPPERFACTORY_H
