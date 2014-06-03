/* This file is part of the KDE project
   Copyright (C) 2004-2013 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "msghandler.h"

using namespace KexiDB;

MessageTitle::MessageTitle(Object* o, const QString& msg)
        : m_obj(o)
        , m_prevMsgTitle(o->m_msgTitle)
{
    m_obj->m_msgTitle = msg;
}

MessageTitle::~MessageTitle()
{
    m_obj->m_msgTitle = m_prevMsgTitle;
}

//------------------------------------------------

MessageHandler::MessageHandler(QWidget *parent)
        : m_messageHandlerParentWidget(parent)
        , m_messageHandlerProxy(0)
        , m_enableMessages(true)
{
}

MessageHandler::~MessageHandler()
{
}

void MessageHandler::showErrorMessage(const QString &msg, const QString &details)
{
    if (m_messageHandlerProxy) {
        m_messageHandlerProxy->showErrorMessage(msg, details);
    }
    else {
        showErrorMessageInternal(msg, details);
    }
}

void MessageHandler::showErrorMessage(KexiDB::Object *obj, const QString& msg)
{
    if (m_messageHandlerProxy) {
        m_messageHandlerProxy->showErrorMessage(obj, msg);
    }
    else {
        showErrorMessageInternal(obj, msg);
    }
}

int MessageHandler::askQuestion(const QString& message,
                                KMessageBox::DialogType dlgType, KMessageBox::ButtonCode defaultResult,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const QString &dontShowAskAgainName,
                                KMessageBox::Options options)
{
    if (m_messageHandlerProxy) {
        return m_messageHandlerProxy->askQuestion(message, dlgType, defaultResult, buttonYes,
                                                  buttonNo, dontShowAskAgainName, options);
    }
    else {
        return askQuestion(message, dlgType, defaultResult, buttonYes,
                           buttonNo, dontShowAskAgainName, options);
    }
}

int MessageHandler::askQuestionInternal(const QString& message,
                                        KMessageBox::DialogType dlgType, KMessageBox::ButtonCode defaultResult,
                                        const KGuiItem &buttonYes,
                                        const KGuiItem &buttonNo,
                                        const QString &dontShowAskAgainName,
                                        KMessageBox::Options options)
{
    Q_UNUSED(message);
    Q_UNUSED(dlgType);
    Q_UNUSED(buttonYes);
    Q_UNUSED(buttonNo);
    Q_UNUSED(dontShowAskAgainName);
    Q_UNUSED(options);
    return defaultResult;
}

void MessageHandler::redirectMessagesTo(MessageHandler *otherHandler)
{
    m_messageHandlerProxy = otherHandler;
}
