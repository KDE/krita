/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
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
