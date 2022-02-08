This is a short overview of the design:
The plugin provides a docker with color selectors.
KisColorSelectorNgDock holds a KisColorSelectorNgDockerWidget.
KisColorSelectorNgDockerWidget holds color patches and a container for the normal color selector (KisColorSelector) and for a shade selector (KisMyPaintShadeSelector or KisMinimalShadeSelector).
KisColorSelectorBase is the base class for all widgets, that can select colors (selector, shade selector, color patches).
KisColorPatches is the base class for all patch fields (currently common colors and last used colors (aka color history))

KisColorSelector is divided into a main and a subcomponent. A hsv or hsl color consists of three parameters, the main component sets two and the sub component one.
All components have a common base class, KisColorSelectorComponent.
