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

#ifndef KDCHARTHEADERFOOTER_H
#define KDCHARTHEADERFOOTER_H

#include "KDChartTextArea.h"
#include "KDChartPosition.h"

namespace KDChart {

    class Chart;
    class TextAttributes;

/**
  * @brief A header or even footer displaying text above or below charts
  */
class KDCHART_EXPORT HeaderFooter : public TextArea
{
    Q_OBJECT

    Q_DISABLE_COPY( HeaderFooter )
    KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( HeaderFooter, Chart* )

public:
    HeaderFooter( Chart* parent = 0 );
    virtual ~HeaderFooter();

    virtual HeaderFooter * clone() const;

    bool compare( const HeaderFooter& other )const;

    enum HeaderFooterType{ Header,
                           Footer };

    void setType( HeaderFooterType type );
    HeaderFooterType type() const;

    void setPosition( Position position );
    Position position() const;

    void setParent( QObject* parent );

Q_SIGNALS:
    void destroyedHeaderFooter( HeaderFooter* );
    void positionChanged( HeaderFooter* );

}; // End of class HeaderFooter

}


#endif // KDCHARTHEADERFOOTER_H
