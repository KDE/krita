/*
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KOCANVASRESOURCESIDS_H
#define KOCANVASRESOURCESIDS_H

namespace KoCanvasResource
{
enum CanvasResourceId {
    ForegroundColor = 0,    ///< The active foreground color selected for this canvas.
    BackgroundColor,    ///< The active background color selected for this canvas.
    PageSize,           ///< The size of the (current) page in postscript points.
    Unit,               ///< The unit of this canvas
    CurrentPage,        ///< The current page number
    ActiveStyleType,    ///< the actual active style type see KoFlake::StyleType for valid values
    ActiveRange,        ///< The area where the rulers should show white
    ShowTextShapeOutlines,     ///< Paint of text shape outlines ?
    ShowFormattingCharacters,  ///< Paint of formatting characters ?
    ShowTableBorders,  ///< Paint of table borders (when not really there) ?
    ShowSectionBounds, ///< Paint of sections bounds ?
    ShowInlineObjectVisualization, ///< paint a different  background for inline objects
    ApplicationSpeciality, ///< Special features and limitations of the application
    KritaStart = 6000,       ///< Base number for Krita specific values.
    HdrExposure = KritaStart + 1,
    CurrentPattern,
    CurrentGamutMask,
    GamutMaskActive,
    CurrentGradient,
    CurrentDisplayProfile,
    CurrentKritaNode,
    CurrentPaintOpPreset,
    CurrentGeneratorConfiguration,
    CurrentCompositeOp,
    CurrentEffectiveCompositeOp,
    LodAvailability, ///<-user choice
    LodSizeThreshold, ///<-user choice
    LodSizeThresholdSupported, ///<-paintop property
    EffectiveLodAvailablility, ///<- a superposition of user choice, threshold and paintop traits
    EraserMode,
    MirrorHorizontal,
    MirrorVertical,
    MirrorHorizontalLock,
    MirrorVerticalLock,
    MirrorVerticalHideDecorations,
    MirrorHorizontalHideDecorations,
    Opacity,
    Flow,
    Size,
    HdrGamma,
    GlobalAlphaLock,
    DisablePressure,
    PreviousPaintOpPreset,
    EffectiveZoom, ///<-Used only by painting tools for non-displaying purposes
    PatternSize
};

}

#endif // KOCANVASRESOURCESIDS_H
