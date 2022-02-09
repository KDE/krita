/*
 * SPDX-FileCopyrightText: 2020 boud <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TESTAGFILTERTRESOURCEPROXYMODEL_H
#define TESTAGFILTERTRESOURCEPROXYMODEL_H

#include <QObject>
#include <QtSql>
#include <QString>
#include "KisResourceTypes.h"

class KisResourceLocator;

class TestTagResourceModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void initTestCase();
    void testWithTagModelTester();
    void testRowCount();
    void testData();
    void testTagResource();
    void testUntagResource();
    void testIsResourceTagged();

    void testFilterTagResource();

    void testBeginEndInsert();

    void cleanupTestCase();

private:

    QString m_srcLocation;
    QString m_dstLocation;

    KisResourceLocator *m_locator;
    const QString resourceType = ResourceType::PaintOpPresets;

};


// *** MODEL SIGNAL CHECKER *** //

// Simple class checking the beginInsert... etc. signals
//  and comparing with expected ones.
// Make sure to connect the slots to the signals when using.
// Class must be defined in .h because it uses Q_OBJECT macro
class ModelSignalChecker : public QObject
{
    Q_OBJECT

public:

    enum TYPE
    {
        AboutInsert,
        Insert,
        AboutRemove,
        Remove
    };

    ///
    /// \brief SignalChecker constructor
    /// \param _expectedAmount number of signals expected
    /// \param _expectedFirsts list of expected first indices (in order)
    /// \param _expectedLasts list of expected last indices (in order)
    /// \param _expectedTypes list of expected types of signals (in order)
    ///
    ModelSignalChecker(QList<int> _expectedFirsts, QList<int> _expectedLasts, QList<TYPE> _expectedTypes);


    ///
    /// \brief setInfo a function to set a new set of expected infos to reuse the instance
    /// \param _expectedAmount number of signals expected
    /// \param _expectedFirsts list of expected first indices (in order)
    /// \param _expectedLasts list of expected last indices (in order)
    /// \param _expectedTypes list of expected types of signals (in order)
    ///
    void setInfo(QList<int> _expectedFirsts, QList<int> _expectedLasts, QList<TYPE> _expectedTypes);
    ///
    /// \brief reset reset the information collected by the instance so it can be reused
    ///
    void reset();
    ///
    /// \brief isCorrect
    /// \return if the collected signals information is the same as the expected
    ///
    bool isCorrect();
    ///
    /// \brief writeOut simple debug function (prints out signal information using qCritical())
    ///
    void printOut();


public Q_SLOTS:
    void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);

private:
    void addSignalInfo(int first, int last, TYPE type);


public:
    // collected information
    QList<int> firsts;
    QList<int> lasts;
    QList<TYPE> types;
    int amount;

    // expected information
    QList<int> expectedFirsts;
    QList<int> expectedLasts;
    QList<TYPE> expectedTypes;
    int expectedAmount;

};

#endif
