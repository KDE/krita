/*
 * KDE. Krita Project.
 *
 * Copyright (c) 2020 Deif Lou <ginoba@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISGRADIENTCONVERSION_H
#define KISGRADIENTCONVERSION_H

#include <QGradient>

#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <KoColor.h>
#include "kritapigment_export.h"

/**
 * @brief Namespace containing functions to convert to/from different types of gradients
 * 
 */
namespace KisGradientConversion
{
    /**
     * @brief Convert a KoAbstractGradient to a QGradientStop list
     * 
     * This function makes use of toQGradientStops(KoStopGradient)
     * and toQGradientStops(KoSegmentGradient)
     * 
     * @param gradient A KoAbstractGradient to convert from
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoAbstractGradient *gradient);
                                                        
    /**
     * @brief Convert a KoStopGradient to a QGradientStop list
     * 
     * For each stop in the stop gradient a QGradientStop is created.
     * 
     * To convert stops that have FOREGROUNDSTOP or BACKGROUNDSTOP type,
     * the color field is used (use setVariableColors on the gradient to set the
     * desired foreground and background colors)
     * 
     * @param gradient A KoStopGradient to convert from
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoStopGradient *gradient);
                                                        
    /**
     * @brief Convert a KoSegmentGradient to a QGradientStop list
     * 
     * For each segment in the segment gradient two QGradientStop are created,
     * one for the start point and another one for the end point.
     * 
     * To convert end points that have FOREGROUND_ENDPOINT, BACKGROUND_ENDPOINT,
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT type,
     * the color field is used (use setVariableColors on the gradient to set the
     * desired foreground and background colors)
     * 
     * The opacity of the QGradientStop color is set to 0 if the end point type is
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT.
     * 
     * If two QGradientStop turn out to be in the same position and have the
     * same color, they are collapsed and only one stop is added to the list
     * (This prevents having duplicated stops due to the end point of a segment
     * being equal to the start point of the next segment).
     * 
     * The middle point, interpolation and color interpolation of the segment
     * are ignored, so some information may be lost
     * 
     * @param gradient A KoSegmentGradient to convert from
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoSegmentGradient *gradient);

    /**
     * @brief Convert a KoAbstractGradient to a QGradientStop list
     * 
     * This function makes use of toQGradientStops(KoStopGradient, const KoColor&, const KoColor&)
     * and toQGradientStops(KoSegmentGradient, const KoColor&, const KoColor&)
     * 
     * @param gradient A KoAbstractGradient to convert from
     * @param foregroundColor Color used when the input gradient has a "foreground stop"
     * @param backgroundcolor Color used when the input gradient has a "background stop"
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoAbstractGradient *gradient,
                                                        const KoColor &foregroundColor,
                                                        const KoColor &backgroundcolor);
                                                        
    /**
     * @brief Convert a KoStopGradient to a QGradientStop list
     * 
     * For each stop in the stop gradient a QGradientStop is created.
     * 
     * To convert stops that have FOREGROUNDSTOP or BACKGROUNDSTOP type,
     * the foregroundColor and backgroundcolor parameters are used
     * 
     * @param gradient A KoStopGradient to convert from
     * @param foregroundColor Color used when the input gradient has a "foreground stop"
     * @param backgroundcolor Color used when the input gradient has a "background stop"
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoStopGradient *gradient,
                                                        const KoColor &foregroundColor,
                                                        const KoColor &backgroundcolor);
                                                        
    /**
     * @brief Convert a KoSegmentGradient to a QGradientStop list
     * 
     * For each segment in the segment gradient two QGradientStop are created,
     * one for the start point and another one for the end point.
     * 
     * To convert end points that have FOREGROUND_ENDPOINT, BACKGROUND_ENDPOINT,
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT type,
     * the foregroundColor and backgroundcolor parameters are used
     * 
     * The opacity of the QGradientStop color is set to 0 if the end point type is
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT.
     * 
     * If two QGradientStop turn out to be in the same position and have the
     * same color, they are collapsed and only one stop is added to the list
     * (This prevents having duplicated stops due to the end point of a segment
     * being equal to the start point of the next segment).
     * 
     * The middle point, interpolation and color interpolation of the segment
     * are ignored, so some information may be lost
     * 
     * @param gradient A KoSegmentGradient to convert from
     * @param foregroundColor Color used when the input gradient has a "foreground stop"
     * @param backgroundcolor Color used when the input gradient has a "background stop"
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoSegmentGradient *gradient,
                                                        const KoColor &foregroundColor,
                                                        const KoColor &backgroundcolor);

    /**
     * @brief Convert a KoAbstractGradient to a QGradient
     * 
     * This function makes use of toQGradient(KoStopGradient)
     * and toQGradient(KoSegmentGradient)
     * 
     * @param gradient A KoAbstractGradient to convert from
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoAbstractGradient *gradient);

    /**
     * @brief Convert a KoStopGradient to a QGradient
     * 
     * This function doesn't use the stop gradient's type and start/stop
     * positions to create different types of QGradient. This just
     * creates a QLinearGradient and sets its stops using
     * toQGradientStops(KoStopGradient).
     * To get the correct type of QGradient use KoStopGradient::toQGradient()
     * 
     * @param gradient A KoStopGradient to convert from
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoStopGradient *gradient);
                                               
    /**
     * @brief Convert a KoSegmentGradient to a QGradient
     * 
     * Creates a QLinearGradient and sets its stops using
     * toQGradientStops(KoSegmentGradient)
     * 
     * @param gradient A KoSegmentGradient to convert from
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoSegmentGradient *gradient);

    /**
     * @brief Convert a KoAbstractGradient to a QGradient
     * 
     * This function makes use of toQGradient(KoStopGradient, const KoColor&, const KoColor&)
     * and toQGradient(KoSegmentGradient, const KoColor&, const KoColor&)
     * 
     * @param gradient A KoAbstractGradient to convert from
     * @param foregroundColor Color used when the input gradient has a "foreground stop"
     * @param backgroundcolor Color used when the input gradient has a "background stop"
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoAbstractGradient *gradient,
                                               const KoColor &foregroundColor,
                                               const KoColor &backgroundcolor);

    /**
     * @brief Convert a KoStopGradient to a QGradient
     * 
     * This function doesn't use the stop gradient's type and start/stop
     * positions to create different types of QGradient. This just
     * creates a QLinearGradient and sets its stops using
     * toQGradientStops(KoStopGradient, const KoColor&, const KoColor&).
     * To get the correct type of QGradient use KoStopGradient::toQGradient()
     * 
     * @param gradient A KoStopGradient to convert from
     * @param foregroundColor Color used when the input gradient has a "foreground stop"
     * @param backgroundcolor Color used when the input gradient has a "background stop"
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoStopGradient *gradient,
                                               const KoColor &foregroundColor,
                                               const KoColor &backgroundcolor);
                                               
    /**
     * @brief Convert a KoSegmentGradient to a QGradient
     * 
     * Creates a QLinearGradient and sets its stops using
     * toQGradientStops(KoSegmentGradient, const KoColor&, const KoColor&)
     * 
     * @param gradient A KoSegmentGradient to convert from
     * @param foregroundColor Color used when the input gradient has a "foreground stop"
     * @param backgroundcolor Color used when the input gradient has a "background stop"
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoSegmentGradient *gradient,
                                               const KoColor &foregroundColor,
                                               const KoColor &backgroundcolor);

    /**
     * @brief Convert a QGradientStop list to a krita abstract gradient
     * 
     * This function makes use of toStopGradient(const QGradientStops &)
     * to create a KoStopGradient that is casted to a KoAbstractGradient
     *        
     * @param gradient A QGradientStop list with the stops of the gradient
     * @return KoAbstractGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoAbstractGradient* toAbstractGradient(const QGradientStops &gradient);
    
    /**
     * @brief Convert the stops of a QGradient to a krita abstract gradient
     * 
     * This function makes use of toStopGradient(const QGradientStops &)
     * to create a KoStopGradient that is casted to a KoAbstractGradient
     * 
     * @param gradient A QGradient with the stops of the gradient
     * @return KoAbstractGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoAbstractGradient* toAbstractGradient(const QGradient *gradient);
    
    /**
     * @brief Create a clone of a KoStopGradient and return it casted to a
     *        krita abstract gradient
     * 
     * @param gradient A KoStopGradient gradient to convert from
     * @return A clone of the input gradient casted to KoAbstractGradient
     */
    KRITAPIGMENT_EXPORT KoAbstractGradient* toAbstractGradient(KoStopGradient *gradient);
    
    /**
     * @brief Create a clone of a KoSegmentGradient and return it casted to a
     *        krita abstract gradient
     * 
     * @param gradient A KoSegmentGradient gradient to convert from
     * @return A clone of the input gradient casted to KoAbstractGradient 
     */
    KRITAPIGMENT_EXPORT KoAbstractGradient* toAbstractGradient(KoSegmentGradient *gradient);

    /**
     * @brief Convert a QGradientStop list to a KoStopGradient
     * 
     * @param gradient A QGradientStop list with the stops of the gradient
     * @return KoStopGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradient* toStopGradient(const QGradientStops &gradient);
    
    /**
     * @brief Convert the stops of a QGradient to a KoStopGradient
     * 
     * @param gradient A QGradient with the stops of the gradient
     * @return KoStopGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradient* toStopGradient(const QGradient *gradient);
    
    /**
     * @brief Convert a KoAbstractGradient to a KoStopGradient
     * 
     * If the underlying gradient is a segment gradient,
     * toStopGradient(KoSegmentGradient) is used;
     * otherwise, if it is a stop gradient, a clone is returned
     * 
     * @param gradient A KoAbstractGradient to convert from
     * @return KoStopGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradient* toStopGradient(KoAbstractGradient *gradient);

    /**
     * @brief Convert a KoAbstractGradient to a KoStopGradient
     * 
     * If the underlying gradient is a segment gradient,
     * toStopGradient(KoSegmentGradient, const KoColor&, const KoColor&)
     * is used; otherwise, if it is a stop gradient, a clone is returned
     * 
     * @param gradient A KoAbstractGradient to convert from
     * @param foregroundColor Color used when the input gradient has a FOREGROUND_TRANSPARENT_ENDPOINT stop
     * @param backgroundcolor Color used when the input gradient has a BACKGROUND_TRANSPARENT_ENDPOINT stop
     * @return KoStopGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradient* toStopGradient(KoAbstractGradient *gradient,
                                                       const KoColor &foregroundColor,
                                                       const KoColor &backgroundcolor);
    
    /**
     * @brief Convert a KoSegmentGradient to a KoStopGradient
     * 
     * For each segment in the segment gradient two stops are created,
     * one for the start point and another one for the end point.
     * 
     * To convert end points that have FOREGROUND_ENDPOINT, BACKGROUND_ENDPOINT,
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT type,
     * the the color field of the end point is used. 
     * 
     * The opacity of the stop color is set to 0 if the end point type is
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT.
     * 
     * If two stops turn out to be in the same position and have the
     * same type and color, they are collapsed and only one stop is added to the list
     * (This prevents having duplicated stops due to the end point of a segment
     * being equal to the start point of the next segment).
     * 
     * The middle point, interpolation and color interpolation of the segment
     * are ignored, so some information may be lost
     * 
     * @param gradient A KoSegmentGradient gradient to convert from
     * @return KoStopGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradient* toStopGradient(KoSegmentGradient *gradient);

    /**
     * @brief Convert a KoSegmentGradient to a KoStopGradient
     * 
     * For each segment in the segment gradient two stops are created,
     * one for the start point and another one for the end point.
     * 
     * To convert end points that have FOREGROUND_TRANSPARENT_ENDPOINT or
     * BACKGROUND_TRANSPARENT_ENDPOINT type, the stop type is set to the COLORSTOP
     * and the foregroundColor and backgroundcolor parameters are used to set
     * the stop color. 
     * 
     * The opacity of the stop color is set to 0 if the end point type is
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT.
     * 
     * If two stops turn out to be in the same position and have the
     * same type and color, they are collapsed and only one stop is added to the list
     * (This prevents having duplicated stops due to the end point of a segment
     * being equal to the start point of the next segment).
     * 
     * The middle point, interpolation and color interpolation of the segment
     * are ignored, so some information may be lost
     * 
     * @param gradient A KoSegmentGradient gradient to convert from
     * @param foregroundColor Color used when the input gradient has a FOREGROUND_TRANSPARENT_ENDPOINT stop
     * @param backgroundcolor Color used when the input gradient has a BACKGROUND_TRANSPARENT_ENDPOINT stop
     * @return KoStopGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradient* toStopGradient(KoSegmentGradient *gradient,
                                                       const KoColor &foregroundColor,
                                                       const KoColor &backgroundcolor);

    /**
     * @brief Convert a QGradientStop list to a krita segment gradient
     * 
     * If two stops have the same position a new segment beween them is not created
     * 
     * @param gradient A QGradientStop list with the stops of the gradient
     * @return KoSegmentGradient containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradient* toSegmentGradient(const QGradientStops &gradient);

    /**
     * @brief Convert a QGradient to a krita segment gradient
     * 
     * Creates a segment gradient from the stops of the QGradient using
     * toSegmentGradient(const QGradientStops &)
     *
     * @param gradient A QGradient with the stops of the gradient
     * @return KoSegmentGradient containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradient* toSegmentGradient(const QGradient *gradient);
    
    /**
     * @brief Convert a krita abstract gradient to a krita segment gradient
     * 
     * If the underlying gradient is a stop gradient, toSegmentGradient(KoStopGradient)
     * is used; otherwise, if it is a segment gradient, a clone is returned
     *
     * @param gradient A KoAbstractGradient to convert from
     * @return KoSegmentGradient containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradient* toSegmentGradient(KoAbstractGradient *gradient);
    
    /**
     * @brief Convert a krita stop gradient to a krita segment gradient
     * 
     * If two stops have the same position a new segment is not created
     * 
     * @param gradient A KoStopGradient to convert from
     * @return KoSegmentGradient containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradient* toSegmentGradient(KoStopGradient *gradient);
}

#endif
