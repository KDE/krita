/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOGUIDEDATA_H
#define KOGUIDEDATA_H

#include <koffice_export.h>
#include <QList>
#include <qnamespace.h>
class KOFFICECORE_EXPORT KoGuidesData
{
public:
    KoGuidesData();

    /**
     * @brief Set the positions of the horizontal and vertical guide lines
     *
     * @param horizontalLines a list of positions of the horizontal guide lines
     * @param verticalLines a list of positions of the vertical guide lines
     */
    void guideLines( const QList<double> &horizontalLines, const QList<double> &verticalLines);
    /**
     * @brief Set the positions of the horizontal guide lines
     *
     * @param lines a list of positions of the horizontal guide lines
     */
    void horizontalGuideLines( const QList<double> &lines );

    /**
     * @brief Set the positions of the vertical guide lines
     *
     * @param lines a list of positions of the vertical guide lines
     */
    void verticalGuideLines( const QList<double> &lines );

    /**
     * @brief Add a guide line
     *
     * @param p the orientation of the guide line
     * @param p the position of the guide line
     */
    void addGuideLine( Qt::Orientation o, double pos );

    /**
     * @brief Display or not guide lines
     */
    bool showGuideLines() const;

    /**
     * @param show display or not guide line
     */
    void setShowGuideLines( bool show );

#if 0 //TODO
    void saveOasisSettings( KoXmlWriter &settingsWriter );
    void loadOasisSettings(const QDomDocument&settingsDoc);
#endif     

private:
    /// list of positions of horizontal guide lines
    QList<double> m_hGuideLines;
    /// list of positions of vertical guide lines
    QList<double> m_vGuideLines;
    bool m_bShowGuideLines;
};


#endif

