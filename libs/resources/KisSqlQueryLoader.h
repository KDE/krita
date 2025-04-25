/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSQLQUERYLOADER_H
#define KISSQLQUERYLOADER_H

#include <kritaresources_export.h>

#include <exception>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QFileDevice>

class KRITARESOURCES_EXPORT KisSqlQueryLoader
{
public:
    struct prepare_only_t {};
    static constexpr prepare_only_t prepare_only{};

    struct FileException : std::exception
    {
        FileException(QString _message,
            QString _filePath,
            QString _fileErrorString)
            : message(_message)
            , filePath(_filePath)
            , fileErrorString(_fileErrorString)
        {}
        
        QString message;
        QString filePath;
        QString fileErrorString;
    };

    struct SQLException : std::exception
    {
        SQLException(QString _message,
            QString _filePath,
            int _statementIndex,
            QSqlError _sqlError)
            : message(_message)
            , filePath(_filePath)
            , statementIndex(_statementIndex)
            , sqlError(_sqlError)
        {}
        
        QString message;
        QString filePath;
        int statementIndex {0};
        QSqlError sqlError;
    };

public:
    KisSqlQueryLoader(const QString &fileName);
    KisSqlQueryLoader(const QString &fileName, prepare_only_t);
    KisSqlQueryLoader(const QString &scriptName, const QString &script);
    KisSqlQueryLoader(const QString &scriptName, const QString &script, prepare_only_t);

    ~KisSqlQueryLoader();

    QSqlQuery& query();
    void exec();

private:
    void init(const QString &fileName, QString entireScript, bool singleStatementMode);

private:
    QSqlQuery m_query;
    QStringList m_statements;
    bool m_singleStatementMode {false};
    QString m_fileName;
};

#endif /* KISSQLQUERYLOADER_H */
