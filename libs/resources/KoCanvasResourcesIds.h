/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    CurrentPaintOpPresetCache, ///< the cache associated with the currently active preset (this cache may be different per canvas if the preset depends on the canvas resources)
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
