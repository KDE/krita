/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2020 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISGRADIENTCONVERSION_H
#define KISGRADIENTCONVERSION_H

#include <QGradient>

#include <KoAbstractGradient.h>
#include <KoStopGradient.h>
#include <KoSegmentGradient.h>
#include <KoCanvasResourcesInterface.h>
#include "kritapigment_export.h"

/**
 * @brief Namespace containing functions to convert to/from different types of gradients
 * 
 */
namespace KisGradientConversion
{
    /**
     * @brief Convert a KoAbstractGradientSP to a QGradientStop list
     * 
     * This function makes use of toQGradientStops(KoStopGradientSP, KoCanvasResourcesInterfaceSP)
     * and toQGradientStops(KoSegmentGradientSP, KoCanvasResourcesInterfaceSP)
     * 
     * @param gradient A KoAbstractGradientSP to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 stops should take the foreground/background color
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoAbstractGradientSP gradient,
                                                        KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);
                                                        
    /**
     * @brief Convert a KoStopGradientSP to a QGradientStop list
     * 
     * For each stop in the stop gradient a QGradientStop is created.
     * 
     * To convert stops that have FOREGROUNDSTOP or BACKGROUNDSTOP type,
     * the canvasResourcesInterface is used. If the canvasResourcesInterface
     * is null, the color field of the stop is used
     * 
     * @param gradient A KoStopGradientSP to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 stops should take the foreground/background color
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoStopGradientSP gradient,
                                                        KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);
                                                        
    /**
     * @brief Convert a KoSegmentGradientSP to a QGradientStop list
     * 
     * For each segment in the segment gradient two QGradientStop are created,
     * one for the start point and another one for the end point.
     * 
     * To convert end points that have FOREGROUND_ENDPOINT, BACKGROUND_ENDPOINT,
     * FOREGROUND_TRANSPARENT_ENDPOINT or BACKGROUND_TRANSPARENT_ENDPOINT type,
     * the canvasResourcesInterface is used. If the canvasResourcesInterface
     * is null, the color field of the end point is used.
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
     * @param gradient A KoSegmentGradientSP to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 stops should take the foreground/background color
     * @return QGradientStop list containing the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradientStops toQGradientStops(KoSegmentGradientSP gradient,
                                                        KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);

    /**
     * @brief Convert a KoAbstractGradientSP to a QGradient
     * 
     * This function makes use of toQGradient(KoStopGradientSP, KoCanvasResourcesInterfaceSP)
     * and toQGradient(KoSegmentGradientSP, KoCanvasResourcesInterfaceSP)
     * 
     * @param gradient A KoAbstractGradientSP to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 stops should take the foreground/background color
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoAbstractGradientSP gradient,
                                               KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);

    /**
     * @brief Convert a KoStopGradientSP to a QGradient
     * 
     * This function doesn't use the stop gradient's type and start/stop
     * positions to create different types of QGradient. This just
     * creates a QLinearGradient and sets its stops using
     * toQGradientStops(KoStopGradientSP, KoCanvasResourcesInterfaceSP).
     * To get the correct type of QGradient use KoStopGradient::toQGradient()
     * 
     * @param gradient A KoStopGradientSP to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 stops should take the foreground/background color
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoStopGradientSP gradient,
                                               KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);
                                               
    /**
     * @brief Convert a KoSegmentGradientSP to a QGradient
     * 
     * Creates a QLinearGradient and sets its stops using
     * toQGradientStops(KoSegmentGradientSP, KoCanvasResourcesInterfaceSP)
     * 
     * @param gradient A KoSegmentGradientSP to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 stops should take the foreground/background color
     * @return QGradient containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT QGradient* toQGradient(KoSegmentGradientSP gradient,
                                               KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);

    /**
     * @brief Convert a QGradientStop list to a krita abstract gradient
     * 
     * This function makes use of toStopGradient(const QGradientStops &)
     * to create a KoStopGradientSP that is casted to a KoAbstractGradientSP
     *        
     * @param gradient A QGradientStop list with the stops of the gradient
     * @return KoAbstractGradientSP containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoAbstractGradientSP toAbstractGradient(const QGradientStops &gradient);
    
    /**
     * @brief Convert the stops of a QGradient to a krita abstract gradient
     * 
     * This function makes use of toStopGradient(const QGradientStops &)
     * to create a KoStopGradientSP that is casted to a KoAbstractGradientSP
     * 
     * @param gradient A QGradient with the stops of the gradient
     * @return KoAbstractGradientSP containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoAbstractGradientSP toAbstractGradient(const QGradient *gradient);
    
    /**
     * @brief Create a clone of a KoStopGradientSP and return it casted to a
     *        krita abstract gradient
     * 
     * @param gradient A KoStopGradientSP gradient to convert from
     * @return A clone of the input gradient casted to KoAbstractGradientSP
     */
    KRITAPIGMENT_EXPORT KoAbstractGradientSP toAbstractGradient(KoStopGradientSP gradient);
    
    /**
     * @brief Create a clone of a KoSegmentGradientSP and return it casted to a
     *        krita abstract gradient
     * 
     * @param gradient A KoSegmentGradientSP gradient to convert from
     * @return A clone of the input gradient casted to KoAbstractGradientSP 
     */
    KRITAPIGMENT_EXPORT KoAbstractGradientSP toAbstractGradient(KoSegmentGradientSP gradient);

    /**
     * @brief Convert a QGradientStop list to a KoStopGradientSP
     * 
     * @param gradient A QGradientStop list with the stops of the gradient
     * @return KoStopGradientSP containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradientSP toStopGradient(const QGradientStops &gradient);
    
    /**
     * @brief Convert the stops of a QGradient to a KoStopGradientSP
     * 
     * @param gradient A QGradient with the stops of the gradient
     * @return KoStopGradientSP containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradientSP toStopGradient(const QGradient *gradient);
    
    /**
     * @brief Convert a KoAbstractGradientSP to a KoStopGradientSP
     * 
     * If the underlying gradient is a segment gradient,
     * toStopGradient(KoSegmentGradientSP, KoCanvasResourcesInterfaceSP)
     * is used; otherwise, if it is a stop gradient, a clone is returned
     * 
     * @param gradient A KoAbstractGradientSP to convert from
     * @return KoStopGradientSP containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradientSP toStopGradient(KoAbstractGradientSP gradient,
                                                        KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);
    
    /**
     * @brief Convert a KoSegmentGradientSP to a KoStopGradientSP
     * 
     * For each segment in the segment gradient two stops are created,
     * one for the start point and another one for the end point.
     * 
     * To convert end points that have FOREGROUND_TRANSPARENT_ENDPOINT or
     * BACKGROUND_TRANSPARENT_ENDPOINT type, the stop type is set to the COLORSTOP
     * and the canvasResourcesInterface is used to set the stop color if it is
     * not null. If the canvasResourcesInterface is null, the color field of
     * the end point is used. 
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
     * @param gradient A KoSegmentGradientSP gradient to convert from
     * @param canvasResourcesInterface KoCanvasResourcesInterfaceSP used if some of the gradient
     *                                 end points have FOREGROUND_TRANSPARENT_ENDPOINT or
     *                                 BACKGROUND_TRANSPARENT_ENDPOINT type
     * @return KoStopGradientSP containing stops with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoStopGradientSP toStopGradient(KoSegmentGradientSP gradient,
                                                        KoCanvasResourcesInterfaceSP canvasResourcesInterface = nullptr);

    /**
     * @brief Convert a QGradientStop list to a krita segment gradient
     * 
     * If two stops have the same position a new segment beween them is not created
     * 
     * @param gradient A QGradientStop list with the stops of the gradient
     * @return KoSegmentGradientSP containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradientSP toSegmentGradient(const QGradientStops &gradient);

    /**
     * @brief Convert a QGradient to a krita segment gradient
     * 
     * Creates a segment gradient from the stops of the QGradient using
     * toSegmentGradient(const QGradientStops &)
     *
     * @param gradient A QGradient with the stops of the gradient
     * @return KoSegmentGradientSP containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradientSP toSegmentGradient(const QGradient *gradient);
    
    /**
     * @brief Convert a krita abstract gradient to a krita segment gradient
     * 
     * If the underlying gradient is a stop gradient, toSegmentGradient(KoStopGradientSP)
     * is used; otherwise, if it is a segment gradient, a clone is returned
     *
     * @param gradient A KoAbstractGradientSP to convert from
     * @return KoSegmentGradientSP containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradientSP toSegmentGradient(KoAbstractGradientSP gradient);
    
    /**
     * @brief Convert a krita stop gradient to a krita segment gradient
     * 
     * If two stops have the same position a new segment is not created
     * 
     * @param gradient A KoStopGradientSP to convert from
     * @return KoSegmentGradientSP containing segments with the positions and colors of the gradient
     */
    KRITAPIGMENT_EXPORT KoSegmentGradientSP toSegmentGradient(KoStopGradientSP gradient);
}

#endif
