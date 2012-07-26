/* This file is part of the KDE project
   Copyright (C) 2003-2005 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "object.h"
#include "error.h"
#include "msghandler.h"

#include <klocale.h>
#include <kdebug.h>

using namespace KexiDB;

#define ERRMSG(a) \
    { if (m_msgHandler) m_msgHandler->showErrorMessage(a); }

Object::Object(MessageHandler* handler)
        : m_previousServerResultNum(0)
        , m_previousServerResultNum2(0)
        , m_msgHandler(handler)
        , d(0) //empty
{
    clearError();
}

Object::~Object()
{
}

#define STORE_PREV_ERR \
    m_previousServerResultNum = m_previousServerResultNum2; \
    m_previousServerResultName = m_previousServerResultName2; \
    m_previousServerResultNum2 = serverResult(); \
    m_previousServerResultName2 = serverResultName(); \
    KexiDBDbg << "Object ERROR: " << m_previousServerResultNum2 << ": " \
        << m_previousServerResultName2

void Object::setError(int code, const QString &msg)
{
    STORE_PREV_ERR;

    m_errno = code;
    m_errorSql = m_sql;
    if (m_errno == ERR_OTHER && msg.isEmpty())
        m_errMsg = i18n("Unspecified error encountered");
    else
        m_errMsg = msg;
    m_hasError = code != ERR_NONE;

    if (m_hasError)
        ERRMSG(this);
}

void Object::setError(const QString &msg)
{
    setError(ERR_OTHER, msg);
}

void Object::setError(const QString& title, const QString &msg)
{
    STORE_PREV_ERR;

    m_errno = ERR_OTHER;
    QString origMsgTitle(m_msgTitle);   //store

    m_msgTitle += title;
    m_errMsg = msg;
    m_errorSql = m_sql;
    m_hasError = true;
    if (m_hasError)
        ERRMSG(this);

    m_msgTitle = origMsgTitle; //revert
}

void Object::setError(KexiDB::Object *obj, const QString& prependMessage)
{
    setError(obj, obj ? obj->errorNum() : ERR_OTHER, prependMessage);
}

void Object::setError(KexiDB::Object *obj, int code, const QString& prependMessage)
{
    if (obj && (obj->errorNum() != 0 || !obj->serverErrorMsg().isEmpty())) {
        STORE_PREV_ERR;

        m_errno = obj->errorNum();
        m_hasError = obj->error();
        if (m_errno == 0) {
            m_errno = code;
            m_hasError = true;
        }
        m_errMsg = (prependMessage.isEmpty() ? QString() : (prependMessage + " "))
                   + obj->errorMsg();
        m_sql = obj->m_sql;
        m_errorSql = obj->m_errorSql;
        m_serverResult = obj->serverResult();
        if (m_serverResult == 0) //try copied
            m_serverResult = obj->m_serverResult;
        m_serverResultName = obj->serverResultName();
        if (m_serverResultName.isEmpty()) //try copied
            m_serverResultName = obj->m_serverResultName;
        m_serverErrorMsg = obj->serverErrorMsg();
        if (m_serverErrorMsg.isEmpty()) //try copied
            m_serverErrorMsg = obj->m_serverErrorMsg;
        //override
        if (code != 0 && code != ERR_OTHER)
            m_errno = code;
        if (m_hasError)
            ERRMSG(this);
    } else {
        setError(code != 0 ? code : ERR_OTHER, prependMessage);
    }
}

void Object::clearError()
{
    m_errno = 0;
    m_hasError = false;
    m_errMsg.clear();
    m_sql.clear();
    m_errorSql.clear();
    m_serverResult = 0;
    m_serverResultName.clear();
    m_serverErrorMsg.clear();
    drv_clearServerResult();
}

QString Object::serverErrorMsg()
{
    return m_serverErrorMsg;
}

int Object::serverResult()
{
    return m_serverResult;
}

QString Object::serverResultName()
{
    return m_serverResultName;
}

void Object::debugError()
{
    if (error()) {
        KexiDBDbg << "KEXIDB ERROR: " << errorMsg();
        QString s = serverErrorMsg(), sn = serverResultName();
        if (!s.isEmpty())
            KexiDBDbg << "KEXIDB SERVER ERRMSG: " << s;
        if (!sn.isEmpty())
            KexiDBDbg << "KEXIDB SERVER RESULT NAME: " << sn;
        if (serverResult() != 0)
            KexiDBDbg << "KEXIDB SERVER RESULT #: " << serverResult();
    } else
        KexiDBDbg << "KEXIDB OK.";
}

int Object::askQuestion(const QString& message,
                        KMessageBox::DialogType dlgType, KMessageBox::ButtonCode defaultResult,
                        const KGuiItem &buttonYes,
                        const KGuiItem &buttonNo,
                        const QString &dontShowAskAgainName,
                        KMessageBox::Options options,
                        MessageHandler* msgHandler)
{
    if (msgHandler)
        return msgHandler->askQuestion(message, dlgType, defaultResult, buttonYes, buttonNo,
                                       dontShowAskAgainName, options);

    if (m_msgHandler)
        return m_msgHandler->askQuestion(message, dlgType, defaultResult, buttonYes, buttonNo,
                                         dontShowAskAgainName, options);

    return defaultResult;
}
