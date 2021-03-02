/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInterstrokeDataTransactionWrapperFactory.h"

#include <kundo2command.h>

#include "KisInterstrokeData.h"
#include "KisInterstrokeDataFactory.h"

#include <kis_paint_device.h>
#include <kis_pointer_utils.h>


namespace {
struct BeginInterstrokeDataTransactionCommand : public KUndo2Command
{
    BeginInterstrokeDataTransactionCommand(KisPaintDeviceSP device, KisInterstrokeDataSP newData)
        : m_device(device),
          m_interstrokeData(newData)
    {
    }

    void redo() override {
        if (m_firstRedo) {
            if (m_device->interstrokeData() != m_interstrokeData) {
                m_dataSwapCommand.reset(m_device->createChangeInterstrokeDataCommand(m_interstrokeData));
                m_dataSwapCommand->redo();
            }

            if (m_interstrokeData) {
                m_interstrokeData->beginTransaction();
            }

            m_firstRedo = false;
        } else if (m_dataSwapCommand) {
            m_dataSwapCommand->redo();

        }
    }

    void undo() override {
        if (m_dataSwapCommand) {
            m_dataSwapCommand->undo();
        }
    }

private:
    bool m_firstRedo {true};
    KisPaintDeviceSP m_device;
    KisInterstrokeDataSP m_interstrokeData;
    QScopedPointer<KUndo2Command> m_dataSwapCommand;
};

struct EndInterstrokeDataTransactionCommand : public KUndo2Command
{
    EndInterstrokeDataTransactionCommand(KisPaintDeviceSP device)
        : m_device(device)
    {
    }

    void redo() override {
        KisInterstrokeDataSP data = m_device->interstrokeData();

        if (!m_transactionCommand && data) {
            m_transactionCommand.reset(data->endTransaction());
        }

        if (m_transactionCommand) {
            m_transactionCommand->redo();
        }
    }

    void undo() override {
        if (m_transactionCommand) {
            m_transactionCommand->undo();
        }
    }

private:
    KisPaintDeviceSP m_device;
    QScopedPointer<KUndo2Command> m_transactionCommand;
};

}


struct KisInterstrokeDataTransactionWrapperFactory::Private
{
    QScopedPointer<KisInterstrokeDataFactory> factory;
    KisPaintDeviceSP device;
};

KisInterstrokeDataTransactionWrapperFactory::KisInterstrokeDataTransactionWrapperFactory(KisInterstrokeDataFactory *factory)
    : m_d(new Private())
{
    m_d->factory.reset(factory);
}

KisInterstrokeDataTransactionWrapperFactory::~KisInterstrokeDataTransactionWrapperFactory()
{
}

KUndo2Command *KisInterstrokeDataTransactionWrapperFactory::createBeginTransactionCommand(KisPaintDeviceSP device)
{
    KisInterstrokeDataSP data = device->interstrokeData();
    if (m_d->factory) {
        if (!m_d->factory->isCompatible(data.data())) {
            data = toQShared(m_d->factory->create(device));
        }
    } else {
        data.clear();
    }

    KUndo2Command *cmd = 0;

    if (device->interstrokeData() || data) {
        m_d->device = device;
        cmd = new BeginInterstrokeDataTransactionCommand(device, data);
    }

    return cmd;
}

KUndo2Command *KisInterstrokeDataTransactionWrapperFactory::createEndTransactionCommand()
{
    return m_d->device ? new EndInterstrokeDataTransactionCommand(m_d->device) : 0;
}
