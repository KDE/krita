/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef KDCHARTDATASETSELECTOR_H
#define KDCHARTDATASETSELECTOR_H

#include <QFrame>

#include "KDChartDatasetProxyModel.h"


/**
 * \cond PRIVATE_API_DOCU
 *
 * ( This class is used internally by DatasetSelectorWidget. )
 */
namespace Ui {
    class DatasetSelector;
}
/**
 * \endcond
 */

namespace KDChart {

    class KDCHART_EXPORT DatasetSelectorWidget : public QFrame
    {
        Q_OBJECT

    public:
        explicit DatasetSelectorWidget ( QWidget* parent = 0 );
        ~DatasetSelectorWidget();

    public Q_SLOTS:
        void setSourceRowCount ( const int& rowCount );
        void setSourceColumnCount ( const int& columnCount );

    Q_SIGNALS:
        void configureDatasetProxyModel (
            const DatasetDescriptionVector& rowConfig,
            const DatasetDescriptionVector& columnConfig );

        void mappingDisabled ();

    private Q_SLOTS:
        void on_sbStartColumn_valueChanged ( int );
        void on_sbStartRow_valueChanged ( int );
        void on_sbColumnCount_valueChanged( int );
        void on_sbRowCount_valueChanged( int );
        void on_cbReverseRows_stateChanged ( int );
        void on_cbReverseColumns_stateChanged ( int );
        void on_groupBox_toggled ( bool );


    private:
        void resetDisplayValues ();
        void calculateMapping();

        Ui::DatasetSelector* mUi;
        int mSourceRowCount;
        int mSourceColumnCount;
    };

}

#endif
