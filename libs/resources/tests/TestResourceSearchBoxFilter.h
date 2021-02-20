/*  This file is part of the Krita project

    SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef TEST_RESOURCE_SEARCH_BOX_FILTER_H
#define TEST_RESOURCE_SEARCH_BOX_FILTER_H

#include <QObject>
#include <QScopedPointer>

#include <KisResourceSearchBoxFilter.h>

//Define mockup of a resource based on what we will be filtering for (tags, names, etc..)
struct MockResource {
    MockResource(QString name, QStringList tags)
        : m_name(name)
        , m_tags(tags){
    }

    ~MockResource(){}

    bool operator==(const MockResource& rhs){
        return (m_name == rhs.m_name) && (m_tags == rhs.m_tags);
    }

    QString m_name;
    QStringList m_tags;
};


class TestResourceSearchBoxFilter : public QObject
{
    Q_OBJECT

public:

    TestResourceSearchBoxFilter();

private Q_SLOTS:
    void testOnePartialName_data();
    void testOnePartialName();

    void testMultiplePartialNames_data();
    void testMultiplePartialNames();

    void testOneExactMatch_data();
    void testOneExactMatch();

    void testMultipleExactMatches_data();
    void testMultipleExactMatches();

    void testResourceSearch();

private:
    bool filterMatches(QString resourceName, QString filter);
    void runNameTest();

    QList<MockResource> filterResourceList(QList<MockResource> &resources, KisResourceSearchBoxFilter& filter);

};

#endif // TESTRESOURCESEARCHBOXFILTER_H
