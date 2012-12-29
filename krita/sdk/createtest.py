#!/usr/bin/env python
import sys
import getopt
import os.path
from string import Template

GPL=Template("""/*
 *  Copyright (c) $YEAR $AUTHOR $EMAIL
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
""")

HEADER=Template("""${GPL}
#ifndef ${HEADER_GUARD}_TEST_H
#define ${HEADER_GUARD}_TEST_H

#include <QtTest>

class ${CLASSNAME}Test : public QObject
{
    Q_OBJECT
private slots:

    void testCreation();

};

#endif
""")

IMPLEMENTATION=Template("""${GPL}

#include <qtest_kde.h>

#include "${HEADER}_test.h"

#include "${HEADER}.h"

void ${CLASSNAME}Test::testCreation()
{
    ${CLASSNAME} test;
}


QTEST_KDEMAIN(${CLASSNAME}Test, GUI);
#include "${HEADER}_test.moc"
""")

CMAKE="""
########### next target ###############

set(%(HEADER)s_test_SRCS %(HEADER)s_test.cpp )
kde4_add_unit_test(%(CLASSNAME)sTest TESTNAME krita-image-%(CLASSNAME)sTest ${%(HEADER)s_test_SRCS})
target_link_libraries(%(CLASSNAME)sTest  ${KDE4_KDEUI_LIBS} komain kritaimage ${QT_QTTEST_LIBRARY})
"""


def convertheaderToClassName(header):
    if header.find(".h") >= 0:
        header = header[:-2]
    nextIsCap = True
    classname = ""
    cleanheader = ""
    for c in header:
        if "./".find(c) < 0:
            cleanheader += c
        if "._/".find(c) < 0:
            if nextIsCap:
                classname += c.upper()
                nextIsCap = False
            else:
                classname += c
        if "._/".find(c) >= 0:
            nextIsCap = True
    return (cleanheader, classname)

def createTest(header, year, author, email):
    strippedheader, classname = convertheaderToClassName(header)
    headerguard = strippedheader.upper()

    if os.path.exists(strippedheader + "_test.h"):
        print "test for " + header + " already exists."
        return

    gpl = GPL.substitute(AUTHOR=author, YEAR = year, EMAIL=email)
    headerfile = HEADER.substitute(GPL=gpl, HEADER_GUARD=headerguard, CLASSNAME=classname)
    implementationfile = IMPLEMENTATION.substitute(GPL=gpl, HEADER=strippedheader, CLASSNAME=classname)
    cmake = CMAKE % {"HEADER" : strippedheader, "CLASSNAME" : classname}

    open(strippedheader + "_test.h", "w+").write(headerfile)
    open(strippedheader + "_test.cpp", "w+").write(implementationfile)
    open("CMakeLists.txt", "a+").write(cmake)


def main():
    year = "2007"
    author = "Boudewijn Rempt"
    email = "boud@valdyas.org"
    for header in sys.argv[1:]:
        createTest(header, year, author, email)

if __name__ == "__main__":
    sys.exit(main())
