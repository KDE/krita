find_package(PythonLibrary)
set_package_properties(PythonLibrary PROPERTIES
    DESCRIPTION "Python Library"
    URL "http://www.python.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(PYTHONLIBS_FOUND HAVE_PYTHONLIBS)
    
find_package(PythonModuleGeneration)
set_package_properties(PythonModuleGeneration PROPERTIES
    DESCRIPTION "Automatic generation of SIP bindings"
    URL "https://api.kde.org/ecm/"
    TYPE REQUIRED
    PURPOSE "Required by the Krita PyQt plugin")

if (PythonModuleGeneration_FOUND)
    add_subdirectory(libkis)

    ecm_generate_python_binding(
        TARGET kritalibkis
        PYTHONNAMESPACE krita
        MODULENAME PyKrita
        RULES_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libkis/rules_PyKrita.py"
        SIP_DEPENDS
            QtCore/QtCoremod.sip
            QtOpenGL/QtOpenGLmod.sip
            Qt/Qtmod.sip
            QtXml/QtXmlmod.sip
            QtGui/QtGuimod.sip
            QtWidgets/QtWidgetsmod.sip
         HEADERS
            libkis/Action.h
            libkis/Canvas.h
            libkis/Channel.h
            libkis/ColorDepth.h
            libkis/ColorManager.h
            libkis/ColorModel.h
            libkis/ColorProfile.h
            libkis/DockWidgetFactoryBase.h
            libkis/DockWidget.h
            libkis/Document.h
            libkis/Exporter.h
            libkis/Filter.h
            libkis/Generator.h
            libkis/Importer.h
            libkis/InfoObject.h
            libkis/Krita.h
            libkis/libkis.h
            libkis/Node.h
            libkis/Notifier.h
            libkis/Resource.h
            libkis/Selection.h
            libkis/Transformation.h
            libkis/ViewExtension.h
            libkis/View.h
            libkis/Window.h
    )
    add_subdirectory(plugin)

endif ()
