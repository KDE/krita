%Module PyKrita.krita

%ModuleHeaderCode
#pragma GCC visibility push(default)
%End

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip
%Import QtWidgets/QtWidgetsmod.sip

%Include Conversions.sip


%Include Shape.sip
%Include GroupShape.sip

%Include Canvas.sip
%Include Channel.sip
%Include DockWidgetFactoryBase.sip
%Include DockWidget.sip
%Include Document.sip
%Include Filter.sip
%Include InfoObject.sip
%Include Extension.sip
%Include View.sip
%Include Window.sip
%Include Krita.sip
%Include Node.sip

%Include GroupLayer.sip
%Include CloneLayer.sip
%Include FilterLayer.sip
%Include FileLayer.sip
%Include FillLayer.sip
%Include VectorLayer.sip
%Include FilterMask.sip
%Include SelectionMask.sip
%Include TransformMask.sip

%Include Notifier.sip
%Include Resource.sip
%Include Selection.sip
%Include Extension.sip
%Include PresetChooser.sip
%Include Scratchpad.sip
%Include Palette.sip
%Include PaletteView.sip
%Include ManagedColor.sip
%Include Swatch.sip

%Include KisCubicCurve.sip
