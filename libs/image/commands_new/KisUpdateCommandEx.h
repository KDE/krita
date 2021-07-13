/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISUPDATECOMMANDEX_H
#define KISUPDATECOMMANDEX_H

#include <kritaimage_export.h>
#include <kis_command_utils.h>

#include <kis_types.h>
#include <boost/none.hpp>
#include <KisBatchNodeUpdate.h>

class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisUpdateCommandEx : public KisCommandUtils::FlipFlopCommand
{
public:
    KisUpdateCommandEx(KisBatchNodeUpdateSP updateData,
                       KisUpdatesFacade *updatesFacade,
                       State state);

    KisUpdateCommandEx(KisBatchNodeUpdateSP updateData,
                       KisUpdatesFacade *updatesFacade,
                       State state,
                       QWeakPointer<boost::none_t> blockUpdatesCookie);

    ~KisUpdateCommandEx();

    void partB() override;

private:
    KisBatchNodeUpdateSP m_updateData;
    QWeakPointer<boost::none_t> m_blockUpdatesCookie;
    KisUpdatesFacade *m_updatesFacade;
};

#endif // KISUPDATECOMMANDEX_H
