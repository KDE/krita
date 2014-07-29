%Module PyKrita4.krita

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip

%ModuleHeaderCode
#pragma GCC visibility push(default)
%End

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Include application.sip
%Include document.sip
%Include image.sip
%Include mainwindow.sip
%Include krita.sip
%Include node.sip
%Include view.sip
