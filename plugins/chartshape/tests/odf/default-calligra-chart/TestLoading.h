/* This file is part of the KDE project

   @@COPYRIGHT@@

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KCHART_TESTLOADING_H_DEFAULT_CALLIGRA_CHART
#define KCHART_TESTLOADING_H_DEFAULT_CALLIGRA_CHART

#include "../TestLoadingBase.h"

namespace KoChart {

class TestLoading : public TestLoadingBase
{
    Q_OBJECT

public:
    TestLoading();

private Q_SLOTS:
    /// Tests title, subtitle and footer
    void testLabels();
    void testInternalTable();
    void testDataSets();
    void testPlotArea();
    void testLegend();
    void testAxes();
};

} // namespace KoChart

#endif // KCHART_TESTLOADING_H_DEFAULT_CALLIGRA_CHART
