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

/**
 * A utility class to load an SQL query from a file and execute it.
 *
 * The class reports all the errors in a form of exceptions, making
 * it much easier to write code with multiple steps. The class can
 * also be used in tandem with KisDatabaseTransactionLock to handle
 * the transactions properly.
 *
 * Usage:
 *
 * Query provided inline:
 *
 *      \code{.cpp}
 *
 *      try {
 *
 *          /// Start the transaction
 *          KisDatabaseTransactionLock transactionLock(database);
 *
 *          // load the query, instead of the filename pass a fake filename that
 *          // starts with `inline://` prefix
 *          KisSqlQueryLoader loader("inline://enable_foreign_keys",
 *                                   QString("PRAGMA foreign_keys = %1")
 *                                       .arg(isEnabled ? "on" : "off"));
 *          // execute the query
 *          loader.exec();
 *
 *          // commit the transaction
 *          transactionLock.commit();
 *
 *      } catch (const KisSqlQueryLoader::SQLException &e) {
 *          // in case of an error an exception will be raised, which can be
 *          // handled gracefully; please note that the transaction will be
 *          // automatically rolled back by KisDatabaseTransactionLock
 *
 *          qWarning().noquote() << "ERROR: failed to execute query:" << e.message;
 *          qWarning().noquote() << "       file:" << e.filePath;
 *          qWarning().noquote() << "       statement:" << e.statementIndex;
 *          qWarning().noquote() << "       error:" << e.sqlError.text();
 *
 *          return false;
 *      }
 *
 *      \endcode
 *
 * Query loaded from the file:
 *
 *      \code{.cpp}
 *
 *      try {
 *
 *          /// Start the transaction
 *          KisDatabaseTransactionLock transactionLock(database);
 *
 *          // load the query, just pass the filename in a form acceptable
 *          // by QFile
 *          KisSqlQueryLoader loader(":/create_index_metadata_key.sql");
 *          // execute the query
 *          loader.exec();
 *
 *          // commit the transaction
 *          transactionLock.commit();
 *
 *      } catch (const KisSqlQueryLoader::SQLException &e) {
 *          // in case of an error an exception will be raised, which can be
 *          // handled gracefully; please note that the transaction will be
 *          // automatically rolled back by KisDatabaseTransactionLock
 *
 *          qWarning().noquote() << "ERROR: failed to execute query:" << e.message;
 *          qWarning().noquote() << "       file:" << e.filePath;
 *          qWarning().noquote() << "       statement:" << e.statementIndex;
 *          qWarning().noquote() << "       error:" << e.sqlError.text();
 *
 *          return false;
 *      } catch (const KisSqlQueryLoader::FileException &e) {
 *          // for file-related errors there will be another type of an exception,
 *          // please don't forget to handle it as well
 *
 *          qWarning().noquote() << "ERROR: failed to execute query:" << message;
 *          qWarning().noquote() << "       error" << e.message;
 *          qWarning().noquote() << "       file:" << e.filePath;
 *          qWarning().noquote() << "       file-error:" << e.fileErrorString;
 *          return false;
 *      }
 *
 *      \endcode
 */

class KRITARESOURCES_EXPORT KisSqlQueryLoader
{
public:
    struct single_statement_mode_t {};
    static constexpr single_statement_mode_t single_statement_mode{};

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
    /**
     * Load the query from a file \p fileName. This form of the constructor
     * does **not** try prepare the query. It means that:
     *
     * 1) The query is allowed to have **multiple** statements. All these statements
     *    will be executed in the exec() stage.
     *
     * 2) One **cannot** bind any values to the query.
     */
    KisSqlQueryLoader(const QString &fileName);

    /**
     * Load the query from a file \p fileName in a "single statement mode". In this
     * mode the loader immediately prepares the query, which means you can bind any
     * values to the query before calling exec().
     */
    KisSqlQueryLoader(const QString &fileName, single_statement_mode_t);

    /**
     * Load the script from an explicit string \p script. \p scriptName is
     * the fake "file name" of the script, which is used for logging and error
     * reporting. Please add prefix `inline://` to these script names to be
     * consistent.
     *
     * This form of the constructor loads the script in a multi-statement more,
     * i.e. it doesn't prepare the query and does not allow value binding.
     */
    KisSqlQueryLoader(const QString &scriptName, const QString &script);

    /**
     * Load the script from an explicit string \p script. \p scriptName is
     * the fake "file name" of the script, which is used for logging and error
     * reporting. Please add prefix `inline://` to these script names to be
     * consistent.
     *
     * This form of the constructor loads the script in a single-statement mode,
     * which prepares the query immediately and allows value binding.
     */
    KisSqlQueryLoader(const QString &scriptName, const QString &script, single_statement_mode_t);

    ~KisSqlQueryLoader();

    QSqlQuery& query();

    /**
     * Execute all statements of the query
     *
     * In single-statement mode, the values can be bound to the query
     * before calling exec().
     */
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
