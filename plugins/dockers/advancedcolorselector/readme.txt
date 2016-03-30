This is a short overview of the design:
The plugin provides a docker with colour selectors.
KisColorSelectorNgDock holds a KisColorSelectorNgDockerWidget.
KisColorSelectorNgDockerWidget holds colour patches and a container for the normal colour selector (KisColourSelector) and for a shade selector (KisMyPaintShadeSelector or KisMinimalShadeSelector).
KisColorSelectorBase is the base class for all widgets, that can select colours (selector, shade selector, colour patches).
KisColorPatches is the base class for all patch fields (currently common colours and last used colours (aka colour history))

KisColorSelector is divided into a main and a subcomponent. A hsv or hsl colour consists of three parameters, the main component sets two and the sub component one.
All components have a common base class, KisColorSelectorComponent.
