/*
 * Kexi Report Plugin
 * Copyright (C) 2001-2007 by OpenMFG, LLC (info@openmfg.com)
 * Copyright (C) 2007-2008 by Adam Pigg (adam@piggz.co.uk)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KRSCRIPTCONSTANTS_H
#define KRSCRIPTCONSTANTS_H

#include <QObject>

/**
 @brief Helper giving access to various scripting constants

 Contains methods for retreiving the current page number, page total
 and access to the PenStyle enums from user scripts
*/
class KRScriptConstants : public QObject
{
    Q_OBJECT
public:
    explicit KRScriptConstants(QObject *parent = 0);

    ~KRScriptConstants();
    Q_ENUMS(PenStyle)

    //!Enum values for pen styles that can be accessed from user scripts using
    //! \code
    //! constants.QtSolidLine
    //! \endcode
    //! for example
    enum PenStyle {QtNoPen = 0, QtSolidLine, QtDashLine, QtDotLine, QtDashDotLine, QtDashDotDotLine};

    void setPageNumber(int p) {
        m_currentPage = p;
    }
    void setPageTotal(int t) {
        m_totalPages = t;
    };
public Q_SLOTS:

    //! @return the current page number
    int PageNumber() {
        return m_currentPage;
    };

    //! @return the total number of pages
    int PageTotal() {
        return m_totalPages;
    };

private:
    int m_currentPage;
    int m_totalPages;


};

#endif
