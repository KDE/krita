/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSqlQueryLoader.h"

#include <algorithm>
#include <stdexcept>

#include <QtSql>
#include <QFile>
#include <QRegularExpression>
#include <QStringList>
#include <QTextStream>

#include <kis_assert.h>
#include <KisMpl.h>

namespace kismpl {
    template<typename Class, typename MemType, typename MemTypeNoRef = std::remove_reference_t<MemType>>
    inline auto mem_equal_to(MemTypeNoRef (Class::*ptr)() const noexcept, MemType &&value) {
        return detail::mem_checker<std::equal_to<>, Class, MemTypeNoRef, decltype(ptr)>{ptr, std::forward<MemType>(value)};
    }
}

void KisSqlQueryLoader::init(const QString &fileName, QString entireScript, bool singleStatementMode)
{
    m_singleStatementMode = singleStatementMode;
    m_fileName = fileName;

    QTextStream stream(&entireScript);

    // remove comments by splitting into lines
    QRegularExpression regexp("^--.*$");
    while (!stream.atEnd()) {
        const QString statement = stream.readLine().trimmed();
        if (!regexp.match(statement).hasMatch()) {
            m_statements.append(statement);
        }
    }

    // split lines into actual statements
    m_statements = m_statements.join(' ').split(';');

    // trim the statements
    std::transform(m_statements.begin(), m_statements.end(),
                    m_statements.begin(), [] (const QString &x) { return x.trimmed(); });

    // remove empty statements
    m_statements.erase(std::remove_if(m_statements.begin(), m_statements.end(),
                                        kismpl::mem_equal_to(&QString::isEmpty, true)),
                        m_statements.end());

    if (m_singleStatementMode) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_statements.size() == 1);
        if (!m_query.prepare(m_statements.first())) {
            throw SQLException(
                "Failed to prepare an sql query from file",
                m_fileName,
                0,
                m_query.lastError());
        }
    }
}

KisSqlQueryLoader::KisSqlQueryLoader(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        init(fileName, file.readAll(), false);
    } else {
        throw FileException("Could not load SQL script file", fileName, file.errorString());
    }
}

KisSqlQueryLoader::KisSqlQueryLoader(const QString &fileName, single_statement_mode_t)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly)) {
        init(fileName, file.readAll(), true);
    } else {
        throw FileException("Could not load SQL script file", fileName, file.errorString());
    }
}

KisSqlQueryLoader::KisSqlQueryLoader(const QString &scriptName, const QString &script)
{
    init(scriptName, script, false);
}

KisSqlQueryLoader::KisSqlQueryLoader(const QString &scriptName, const QString &script, single_statement_mode_t)
{
    init(scriptName, script, true);
}

KisSqlQueryLoader::~KisSqlQueryLoader()
{
}

QSqlQuery& KisSqlQueryLoader::query()
{
    return m_query;
}

void KisSqlQueryLoader::exec()
{
    if (m_singleStatementMode) {
        if (!m_query.exec()) {
            throw SQLException(
                "Failed to execute sql from file",
                m_fileName,
                0,
                m_query.lastError());
        }
    } else {
        for (int i = 0; i < m_statements.size(); i++) {
            const QString &statement = m_statements[i];
            if (!m_query.exec(statement)) {
                throw SQLException(
                    "Failed to execute sql from file",
                    m_fileName,
                    i,
                    m_query.lastError());
            }
        }
    }
}
