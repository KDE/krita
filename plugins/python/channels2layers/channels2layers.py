#-----------------------------------------------------------------------------
# Channels to Layers
# SPDX-FileCopyrightText: 2019 - Grum 999
# -----------------------------------------------------------------------------
# SPDX-License-Identifier: GPL-3.0-or-later
# -----------------------------------------------------------------------------
# A Krita plugin designed to split channels from a layer to sub-layers
# . RGB
# . CMY
# . CMYK
# . RGB as grayscale values
# . CMY as grayscale values
# . CMYK as grayscale values
# -----------------------------------------------------------------------------

import re
from krita import (
        Extension,
        InfoObject,
        Node,
        Selection
    )
from PyQt5.Qt import *
from PyQt5 import QtCore
from PyQt5.QtCore import (
        pyqtSlot,
        QBuffer,
        QByteArray,
        QIODevice
    )
from PyQt5.QtGui import (
        QColor,
        QImage,
        QPixmap,
    )
from PyQt5.QtWidgets import (
        QApplication,
        QCheckBox,
        QComboBox,
        QDialog,
        QDialogButtonBox,
        QFormLayout,
        QGroupBox,
        QHBoxLayout,
        QLabel,
        QLineEdit,
        QMessageBox,
        QProgressBar,
        QProgressDialog,
        QVBoxLayout,
        QWidget
    )


PLUGIN_VERSION = '1.1.0'

EXTENSION_ID = 'pykrita_channels2layers'
PLUGIN_MENU_ENTRY = i18n('Channels to layers')
PLUGIN_DIALOG_TITLE = "{0} - {1}".format(i18n('Channels to layers'), PLUGIN_VERSION)

# Define DialogBox types
DBOX_INFO = 'i'
DBOX_WARNING ='w'


# Define Output modes
OUTPUT_MODE_RGB = i18n('RGB Colors')
OUTPUT_MODE_CMY = i18n('CMY Colors')
OUTPUT_MODE_CMYK = i18n('CMYK Colors')
OUTPUT_MODE_LRGB = i18n('RGB Grayscale levels')
OUTPUT_MODE_LCMY = i18n('CMY Grayscale levels')
OUTPUT_MODE_LCMYK = i18n('CMYK Grayscale levels')

OUTPUT_PREVIEW_MAXSIZE = 320

# Define original layer action
ORIGINAL_LAYER_KEEPUNCHANGED = i18n('Unchanged')
ORIGINAL_LAYER_KEEPVISIBLE = i18n('Visible')
ORIGINAL_LAYER_KEEPHIDDEN = i18n('Hidden')
ORIGINAL_LAYER_REMOVE = i18n('Remove')

# define dialog option minimum dimension
DOPT_MIN_WIDTH = OUTPUT_PREVIEW_MAXSIZE * 5 + 200
DOPT_MIN_HEIGHT = 480



OUTPUT_MODE_NFO = {
    OUTPUT_MODE_RGB : {
        'description' : 'Extract channels (Red, Green, Blue) and create a colored layer per channel',
        'groupLayerName' : 'RGB',
        'layers' : [
                    {
                        'color' : 'B',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.blue)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'inverse_subtract'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                }
                            ]
                    },
                    {
                        'color' : 'G',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.green)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'inverse_subtract'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                }
                            ]
                    },
                    {
                        'color' : 'R',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.red)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'inverse_subtract'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                }
                            ]
                    }
                ]
    },
    OUTPUT_MODE_CMY : {
        'description' : 'Extract channels (Cyan, Mangenta, Yellow) and create a colored layer per channel',
        'groupLayerName' : 'CMY',
        'layers' : [
                    {
                        'color' : 'Y',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.yellow)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                }
                            ]
                    },
                    {
                        'color' : 'M',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.magenta)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                }
                            ]
                    },
                    {
                        'color' : 'C',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.cyan)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                }
                            ]
                    }
                ]
    },
    OUTPUT_MODE_CMYK : {
        'description' : 'Extract channels (Cyan, Mangenta, Yellow, Black) and create a colored layer per channel',
        'groupLayerName' : 'CMYK',
        'layers' : [
                    {
                        'color' : 'K',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=5'  # desaturate method = max
                                }
                            ]
                    },
                    {
                        'color' : 'Y',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.yellow)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'duplicate',
                                    'value' : '@K'
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'divide'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                }
                            ]
                    },
                    {
                        'color' : 'M',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.magenta)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'duplicate',
                                    'value' : '@K'
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'divide'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                }
                            ]
                    },
                    {
                        'color' : 'C',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.cyan)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'duplicate',
                                    'value' : '@K'
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'divide'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                }
                            ]
                    }
                ]
    },
    OUTPUT_MODE_LRGB : {
        'description' : 'Extract channels (Red, Green, Blue) and create a grayscale layer per channel',
        'groupLayerName' : 'RGB[GS]',
        'layers' : [
                    {
                        'color' : 'B',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.blue)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'inverse_subtract'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=5'  # desaturate method = max
                                }
                            ]
                    },
                    {
                        'color' : 'G',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.green)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'inverse_subtract'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=5'  # desaturate method = max
                                }
                            ]
                    },
                    {
                        'color' : 'R',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.red)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'inverse_subtract'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=5'  # desaturate method = max
                                }
                            ]
                    }
                ]
    },
    OUTPUT_MODE_LCMY : {
        'description' : 'Extract channels (Cyan, Mangenta, Yellow) and create a grayscale layer per channel',
        'groupLayerName' : 'CMY[GS]',
        'layers' : [
                    {
                        'color' : 'Y',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.yellow)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=4'  # desaturate method = min
                                }
                            ]
                    },
                    {
                        'color' : 'M',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.magenta)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=4'  # desaturate method = min
                                }
                            ]
                    },
                    {
                        'color' : 'C',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.cyan)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=4'  # desaturate method = min
                                }
                            ]
                    }
                ]
    },
    OUTPUT_MODE_LCMYK : {
        'description' : 'Extract channels (Cyan, Mangenta, Yellow, Black) and create a grayscale layer per channel',
        'groupLayerName' : 'CMYK[GS]',
        'layers' : [
                    {
                        'color' : 'K',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=5'  # desaturate method = max
                                }
                            ]
                    },
                    {
                        'color' : 'Y',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.yellow)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'duplicate',
                                    'value' : '@K'
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'divide'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=4'  # desaturate method = min
                                }
                            ]
                    },
                    {
                        'color' : 'M',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.magenta)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'duplicate',
                                    'value' : '@K'
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'divide'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=4'  # desaturate method = min
                                }
                            ]
                    },
                    {
                        'color' : 'C',
                        'process': [
                                {
                                    'action' : 'duplicate',
                                    'value' : '@original'
                                },
                                {
                                    'action' : 'new',
                                    'value' : {
                                                'type' : 'filllayer',
                                                'color' :  QColor(Qt.cyan)
                                            }
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'add'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'duplicate',
                                    'value' : '@K'
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'divide'
                                },
                                {
                                    'action' : 'merge down',
                                    'value' : None
                                },
                                {
                                    'action' : 'blending mode',
                                    'value' : 'multiply'
                                },
                                {
                                    'action' : 'filter',
                                    'value' : 'name=desaturate;type=4'  # desaturate method = min
                                }
                            ]
                    }
                ]
    }
}

TRANSLATIONS_DICT = {
    'colorDepth' : {
        'U8' : '8bits',
        'U16' : '16bits',
        'F16' : '16bits floating point',
        'F32' : '32bits floating point'
    },
    'colorModel' : {
        'A' : 'Alpha mask',
        'RGBA' : 'RGB with alpha channel',
        'XYZA' : 'XYZ with alpha channel',
        'LABA' : 'LAB with alpha channel',
        'CMYKA' : 'CMYK with alpha channel',
        'GRAYA' : 'Gray with alpha channel',
        'YCbCrA' : 'YCbCr with alpha channel'
    },
    'layerType' : {
        'paintlayer' : 'Paint layer',
        'grouplayer' : 'Group layer',
        'filelayer' : 'File layer',
        'filterlayer' : 'Filter layer',
        'filllayer' : 'Fill layer',
        'clonelayer' : 'Clone layer',
        'vectorlayer' : 'Vector layer',
        'transparencymask' : 'Transparency mask',
        'filtermask' : 'Filter mask',
        'transformmask': 'Transform mask',
        'selectionmask': 'Selection mask',
        'colorizemask' : 'Colorize mask'
    }
}




class ChannelsToLayers(Extension):

    def __init__(self, parent):
        # Default options
        self.__outputOptions = {
                'outputMode': OUTPUT_MODE_RGB,
                'originalLayerAction': ORIGINAL_LAYER_KEEPHIDDEN,
                'layerGroupName': '{mode}-{source:name}',
                'layerColorName': '{mode}[{color:short}]-{source:name}'
            }

        self.__sourceDocument = None
        self.__sourceLayer = None

        # Always initialise the superclass.
        # This is necessary to create the underlying C++ object
        super().__init__(parent)
        self.parent = parent


    def setup(self):
        pass


    def createActions(self, window):
        action = window.createAction(EXTENSION_ID, PLUGIN_MENU_ENTRY, "tools/scripts")
        action.triggered.connect(self.action_triggered)

    def dBoxMessage(self, msgType, msg):
        """Simplified function for DialogBox 'OK' message"""
        if msgType == DBOX_WARNING:
            QMessageBox.warning(
                    QWidget(),
                    PLUGIN_DIALOG_TITLE,
                    i18n(msg)
                )
        else:
            QMessageBox.information(
                    QWidget(),
                    PLUGIN_DIALOG_TITLE,
                    i18n(msg)
                )


    def action_triggered(self):
        """Action called when script is executed from Kitra menu"""
        if self.checkCurrentLayer():
            if self.openDialogOptions():
                self.run()


    def translateDictKey(self, key, value):
        """Translate key from dictionary (mostly internal Krita internal values) to human readable values"""
        returned = i18n('Unknown')

        if key in TRANSLATIONS_DICT.keys():
            if value in TRANSLATIONS_DICT[key].keys():
                returned = i18n(TRANSLATIONS_DICT[key][value])

        return returned


    def checkCurrentLayer(self):
        """Check if current layer is valid
           - A document must be opened
           - Active layer properties must be:
             . Layer type:  a paint layer
             . Color model: RGBA
             . Color depth: 8bits
        """
        self.__sourceDocument = Application.activeDocument()
        # Check if there's an active document
        if self.__sourceDocument is None:
            self.dBoxMessage(DBOX_WARNING, "There's no active document!")
            return False

        self.__sourceLayer = self.__sourceDocument.activeNode()

        # Check if current layer can be processed
        if self.__sourceLayer.type() != "paintlayer" or self.__sourceLayer.colorModel() != "RGBA" or self.__sourceLayer.colorDepth() != "U8":
            self.dBoxMessage(DBOX_WARNING, "Selected layer must be a 8bits RGBA Paint Layer!"
                                           "\n\nCurrent layer '{0}' properties:"
                                           "\n- Layer type: {1}"
                                           "\n- Color model: {2} ({3})"
                                           "\n- Color depth: {4}"
                                           "\n\n> Action is cancelled".format(self.__sourceLayer.name(),
                                                                         self.translateDictKey('layerType', self.__sourceLayer.type()),
                                                                         self.__sourceLayer.colorModel(), self.translateDictKey('colorModel', self.__sourceLayer.colorModel()),
                                                                         self.translateDictKey('colorDepth', self.__sourceLayer.colorDepth())
                            ))
            return False

        return True



    def toQImage(self, layerNode, rect=None):
        """Return `layerNode` content as a QImage (as ARGB32)

        The `rect` value can be:
        - None, in this case will return all `layerNode` content
        - A QRect() object, in this case return `layerNode` content reduced to given rectangle bounds
        """
        srcRect = layerNode.bounds()

        if len(layerNode.childNodes()) == 0:
            projectionMode = False
        else:
            projectionMode = True

        if projectionMode == True:
            img = QImage(layerNode.projectionPixelData(srcRect.left(), srcRect.top(), srcRect.width(), srcRect.height()), srcRect.width(), srcRect.height(), QImage.Format_ARGB32)
        else:
            img = QImage(layerNode.pixelData(srcRect.left(), srcRect.top(), srcRect.width(), srcRect.height()), srcRect.width(), srcRect.height(), QImage.Format_ARGB32)

        return img.scaled(rect.width(), rect.height(), Qt.IgnoreAspectRatio, Qt.SmoothTransformation)


    def openDialogOptions(self):
        """Open dialog box to let user define channel extraction options"""

        tmpDocument = None
        previewBaSrc = QByteArray()
        lblPreview = [QLabel(), QLabel(), QLabel(), QLabel()]
        lblPreviewLbl = [QLabel(), QLabel(), QLabel(), QLabel()]


        # ----------------------------------------------------------------------
        # Define signal and slots for UI widgets
        @pyqtSlot('QString')
        def ledLayerGroupName_Changed(value):
            self.__outputOptions['layerGroupName'] = value

        @pyqtSlot('QString')
        def ledLayerColorName_Changed(value):
            self.__outputOptions['layerColorName'] = value

        @pyqtSlot('QString')
        def cmbOutputMode_Changed(value):
            self.__outputOptions['outputMode'] = value
            buildPreview()

        @pyqtSlot('QString')
        def cmbOriginalLayerAction_Changed(value):
            self.__outputOptions['originalLayerAction'] = value

        def buildPreview():
            pbProgress.setVisible(True)

            backupValue = self.__outputOptions['layerColorName']
            self.__outputOptions['layerColorName'] = '{color:long}'

            # create a temporary document to work
            tmpDocument = Application.createDocument(imgThumbSrc.width(), imgThumbSrc.height(), "tmp", "RGBA", "U8", "", 120.0)

            # create a layer used as original layer
            originalLayer = tmpDocument.createNode("Original", "paintlayer")
            tmpDocument.rootNode().addChildNode(originalLayer, None)
            # and set original image content
            originalLayer.setPixelData(previewBaSrc, 0, 0, tmpDocument.width(), tmpDocument.height())

            # execute process
            groupLayer = self.process(tmpDocument, originalLayer, pbProgress)

            self.__outputOptions['layerColorName'] = backupValue

            originalLayer.setVisible(False)
            groupLayer.setVisible(True)

            for layer in groupLayer.childNodes():
                layer.setBlendingMode('normal')
                layer.setVisible(False)
            tmpDocument.refreshProjection()

            index = 0
            for layer in groupLayer.childNodes():
                layer.setVisible(True)
                tmpDocument.refreshProjection()

                lblPreview[index].setPixmap(QPixmap.fromImage(tmpDocument.projection(0, 0, tmpDocument.width(), tmpDocument.height())))
                lblPreviewLbl[index].setText("<i>{0}</i>".format(layer.name()))
                layer.setVisible(False)

                index+=1

            if index > 3:
                lblPreview[3].setVisible(True)
                lblPreviewLbl[3].setVisible(True)
            else:
                lblPreview[3].setVisible(False)
                lblPreviewLbl[3].setVisible(False)


            tmpDocument.close()

            pbProgress.setVisible(False)


        # ----------------------------------------------------------------------
        # Create dialog box
        dlgMain = QDialog(Application.activeWindow().qwindow())
        dlgMain.setWindowTitle(PLUGIN_DIALOG_TITLE)
        # resizeable with minimum size
        dlgMain.setSizeGripEnabled(True)
        dlgMain.setMinimumSize(DOPT_MIN_WIDTH, DOPT_MIN_HEIGHT)
        dlgMain.setModal(True)

        # ......................................................................
        # main dialog box, container
        vbxMainContainer = QVBoxLayout(dlgMain)

        # main dialog box, current layer name
        lblLayerName = QLabel("{0} <b><i>{1}</i></b>".format(i18n("Processing layer"), self.__sourceLayer.name()))
        lblLayerName.setFixedHeight(30)
        vbxMainContainer.addWidget(lblLayerName)

        # main dialog box, groupbox for layers options
        gbxLayersMgt = QGroupBox("Layers management")
        vbxMainContainer.addWidget(gbxLayersMgt)

        # main dialog box, groupbox for output options
        gbxOutputResults = QGroupBox("Output results")
        vbxMainContainer.addWidget(gbxOutputResults)

        vbxMainContainer.addStretch()

        # main dialog box, OK/Cancel buttons
        dbbxOkCancel = QDialogButtonBox(dlgMain)
        dbbxOkCancel.setOrientation(QtCore.Qt.Horizontal)
        dbbxOkCancel.setStandardButtons(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        dbbxOkCancel.accepted.connect(dlgMain.accept)
        dbbxOkCancel.rejected.connect(dlgMain.reject)
        vbxMainContainer.addWidget(dbbxOkCancel)

        # ......................................................................
        # create layout for groupbox "Layers management"
        flLayersMgt = QFormLayout()
        gbxLayersMgt.setLayout(flLayersMgt)

        ledLayerGroupName = QLineEdit()
        ledLayerGroupName.setText(self.__outputOptions['layerGroupName'])
        ledLayerGroupName.textChanged.connect(ledLayerGroupName_Changed)
        flLayersMgt.addRow(i18nc('The name for a new group layer; the generated layers will be placed in this group.', 'New layer group name'), ledLayerGroupName)

        ledLayerColorName = QLineEdit()
        ledLayerColorName.setText(self.__outputOptions['layerColorName'])
        ledLayerColorName.textChanged.connect(ledLayerColorName_Changed)
        flLayersMgt.addRow(i18nc('Defines how the name for each layer created from the channel is generated.', 'New layers color name'), ledLayerColorName)

        cmbOriginalLayerAction = QComboBox()
        cmbOriginalLayerAction.addItems([
                ORIGINAL_LAYER_KEEPUNCHANGED,
                ORIGINAL_LAYER_KEEPVISIBLE,
                ORIGINAL_LAYER_KEEPHIDDEN,
                ORIGINAL_LAYER_REMOVE
            ])
        cmbOriginalLayerAction.setCurrentText(self.__outputOptions['originalLayerAction'])
        cmbOriginalLayerAction.currentTextChanged.connect(cmbOriginalLayerAction_Changed)
        flLayersMgt.addRow(i18n("Original layer"), cmbOriginalLayerAction)

        # ......................................................................
        # create layout for groupbox "Output results"
        flOutputResults = QFormLayout()
        gbxOutputResults.setLayout(flOutputResults)

        cmbOutputMode = QComboBox()
        cmbOutputMode.addItems([
                OUTPUT_MODE_RGB,
                OUTPUT_MODE_CMY,
                OUTPUT_MODE_CMYK,
                OUTPUT_MODE_LRGB,
                OUTPUT_MODE_LCMY,
                OUTPUT_MODE_LCMYK
            ])
        cmbOutputMode.setCurrentText(self.__outputOptions['outputMode'])
        cmbOutputMode.currentTextChanged.connect(cmbOutputMode_Changed)
        flOutputResults.addRow(i18n("Mode"), cmbOutputMode)

        vbxPreviewLblContainer = QHBoxLayout()
        flOutputResults.addRow('', vbxPreviewLblContainer)
        vbxPreviewContainer = QHBoxLayout()
        flOutputResults.addRow('', vbxPreviewContainer)

        # add preview progressbar
        pbProgress = QProgressBar()
        pbProgress.setFixedHeight(8)
        pbProgress.setTextVisible(False)
        pbProgress.setVisible(False)
        pbProgress.setRange(0, 107)
        flOutputResults.addRow('', pbProgress)


        imageRatio = self.__sourceDocument.width() / self.__sourceDocument.height()
        rect = QRect(0, 0, OUTPUT_PREVIEW_MAXSIZE, OUTPUT_PREVIEW_MAXSIZE)

        # always ensure that final preview width and/or height is lower or equal than OUTPUT_PREVIEW_MAXSIZE
        if imageRatio < 1:
            # width < height
            rect.setWidth(int(imageRatio * OUTPUT_PREVIEW_MAXSIZE))
        else:
            # width >= height
            rect.setHeight(int(OUTPUT_PREVIEW_MAXSIZE / imageRatio))

        imgThumbSrc = self.toQImage(self.__sourceLayer, rect)

        previewBaSrc.resize(imgThumbSrc.byteCount())
        ptr = imgThumbSrc.bits()
        ptr.setsize(imgThumbSrc.byteCount())
        previewBaSrc = QByteArray(ptr.asstring())


        lblPreviewSrc = QLabel()
        lblPreviewSrc.setPixmap(QPixmap.fromImage(imgThumbSrc))
        lblPreviewSrc.setFixedHeight(imgThumbSrc.height() + 4)
        lblPreviewSrc.setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewContainer.addWidget(lblPreviewSrc)

        lblPreviewLblSrc = QLabel(i18nc("the original layer", "Original"))
        lblPreviewLblSrc.setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewLblContainer.addWidget(lblPreviewLblSrc)


        vbxPreviewLblContainer.addWidget(QLabel(" "))
        vbxPreviewContainer.addWidget(QLabel(">"))

        lblPreview[3].setPixmap(QPixmap.fromImage(imgThumbSrc))
        lblPreview[3].setFixedHeight(imgThumbSrc.height() + 4)
        lblPreview[3].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewContainer.addWidget(lblPreview[3])

        lblPreviewLbl[3] = QLabel(i18n("<i>Cyan</i>"))
        lblPreviewLbl[3].setIndent(10)
        lblPreviewLbl[3].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewLblContainer.addWidget(lblPreviewLbl[3])


        lblPreview[2] = QLabel()
        lblPreview[2].setPixmap(QPixmap.fromImage(imgThumbSrc))
        lblPreview[2].setFixedHeight(imgThumbSrc.height() + 4)
        lblPreview[2].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewContainer.addWidget(lblPreview[2])

        lblPreviewLbl[2] = QLabel(i18n("<i>Magenta</i>"))
        lblPreviewLbl[2].setIndent(10)
        lblPreviewLbl[2].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewLblContainer.addWidget(lblPreviewLbl[2])


        lblPreview[1] = QLabel()
        lblPreview[1].setPixmap(QPixmap.fromImage(imgThumbSrc))
        lblPreview[1].setFixedHeight(imgThumbSrc.height() + 4)
        lblPreview[1].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewContainer.addWidget(lblPreview[1])

        lblPreviewLbl[1] = QLabel(i18n("<i>Yellow</i>"))
        lblPreviewLbl[1].setIndent(10)
        lblPreviewLbl[1].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewLblContainer.addWidget(lblPreviewLbl[1])


        lblPreview[0] = QLabel()
        lblPreview[0].setPixmap(QPixmap.fromImage(imgThumbSrc))
        lblPreview[0].setFixedHeight(imgThumbSrc.height() + 4)
        lblPreview[0].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewContainer.addWidget(lblPreview[0])

        lblPreviewLbl[0] = QLabel(i18n("<i>Black</i>"))
        lblPreviewLbl[0].setIndent(10)
        lblPreviewLbl[0].setFixedWidth(imgThumbSrc.width() + 4)
        vbxPreviewLblContainer.addWidget(lblPreviewLbl[0])


        vbxPreviewLblContainer.addStretch()
        vbxPreviewContainer.addStretch()

        buildPreview()

        returned = dlgMain.exec_()

        return returned


    def progressNext(self, pProgress):
        """Update progress bar"""
        if pProgress is not None:
            stepCurrent=pProgress.value()+1
            pProgress.setValue(stepCurrent)
            QApplication.instance().processEvents()


    def run(self):
        """Run process for current layer"""

        pdlgProgress = QProgressDialog(self.__outputOptions['outputMode'], None, 0, 100, Application.activeWindow().qwindow())
        pdlgProgress.setWindowTitle(PLUGIN_DIALOG_TITLE)
        pdlgProgress.setMinimumSize(640, 200)
        pdlgProgress.setModal(True)
        pdlgProgress.show()

        self.process(self.__sourceDocument, self.__sourceLayer, pdlgProgress)

        pdlgProgress.close()




    def process(self, pDocument, pOriginalLayer, pProgress):
        """Process given layer with current options"""

        self.layerNum = 0
        document = pDocument
        originalLayer = pOriginalLayer
        parentGroupLayer = None
        currentProcessedLayer = None
        originalLayerIsVisible = originalLayer.visible()


        def getLayerByName(parent, value):
            """search and return a layer by name, within given parent group"""
            if parent == None:
                return document.nodeByName(value)

            for layer in parent.childNodes():
                if layer.name() == value:
                    return layer

            return None


        def duplicateLayer(currentProcessedLayer, value):
            """Duplicate layer from given name
               New layer become active layer
            """

            newLayer = None
            srcLayer = None
            srcName = re.match("^@(.*)", value)

            if not srcName is None:
                # reference to a specific layer
                if srcName[1] == 'original':
                    # original layer currently processed
                    srcLayer = originalLayer
                else:
                    # a color layer previously built (and finished)
                    srcLayer = getLayerByName(parentGroupLayer, parseLayerName(self.__outputOptions['layerColorName'], srcName[1]))
            else:
                # a layer with a fixed name
                srcLayer = document.nodeByName(parseLayerName(value, ''))


            if not srcLayer is None:
                newLayer = srcLayer.duplicate()

                self.layerNum+=1
                newLayer.setName("c2l-w{0}".format(self.layerNum))

                parentGroupLayer.addChildNode(newLayer, currentProcessedLayer)
                return newLayer
            else:
                return None


        def newLayer(currentProcessedLayer, value):
            """Create a new layer of given type
               New layer become active layer
            """

            newLayer = None

            if value is None or not value['type'] in ['filllayer']:
                # given type for new layer is not valid
                # currently only one layer type is implemented
                return None

            if value['type'] == 'filllayer':
                infoObject = InfoObject();
                infoObject.setProperty("color", value['color'])
                selection = Selection();
                selection.select(0, 0, document.width(), document.height(), 255)

                newLayer = document.createFillLayer(value['color'].name(), "color", infoObject, selection)


            if newLayer:
                self.layerNum+=1
                newLayer.setName("c2l-w{0}".format(self.layerNum))

                parentGroupLayer.addChildNode(newLayer, currentProcessedLayer)

                # Need to force generator otherwise, information provided when creating layer seems to not be taken in
                # account
                newLayer.setGenerator("color", infoObject)

                return newLayer
            else:
                return None


        def mergeDown(currentProcessedLayer, value):
            """Merge current layer with layer below"""
            if currentProcessedLayer is None:
                return None

            newLayer = currentProcessedLayer.mergeDown()
            # note:
            #   when layer is merged down:
            #   - a new layer seems to be created (reference to 'down' layer does not match anymore layer in group)
            #   - retrieved 'newLayer' reference does not match to new layer resulting from merge
            #   - activeNode() in document doesn't match to new layer resulting from merge
            #   maybe it's norpmal, maybe not...
            #   but the only solution to be able to work on merged layer (with current script) is to consider that from
            #   parent node, last child match to last added layer and then, to our merged layer
            currentProcessedLayer = parentGroupLayer.childNodes()[-1]
            # for an unknown reason, merged layer bounds are not corrects... :'-(
            currentProcessedLayer.cropNode(0, 0, document.width(), document.height())
            return currentProcessedLayer


        def applyBlendingMode(currentProcessedLayer, value):
            """Set blending mode for current layer"""
            if currentProcessedLayer is None or value is None or value == '':
                return False

            currentProcessedLayer.setBlendingMode(value)
            return True


        def applyFilter(currentProcessedLayer, value):
            """Apply filter to layer"""
            if currentProcessedLayer is None or value is None or value == '':
                return None

            filterName = re.match("name=([^;]+)", value)

            if filterName is None:
                return None

            filter = Application.filter(filterName.group(1))
            filterConfiguration = filter.configuration()

            for parameter in value.split(';'):
                parameterName = re.match("^([^=]+)=(.*)", parameter)

                if not parameterName is None and parameterName != 'name':
                    filterConfiguration.setProperty(parameterName.group(1), parameterName.group(2))

            filter.setConfiguration(filterConfiguration)
            filter.apply(currentProcessedLayer, 0, 0, document.width(), document.height())

            return currentProcessedLayer


        def parseLayerName(value, color):
            """Parse layer name"""

            returned = value

            returned = returned.replace("{source:name}", originalLayer.name())
            returned = returned.replace("{mode}", OUTPUT_MODE_NFO[self.__outputOptions['outputMode']]['groupLayerName'])
            returned = returned.replace("{color:short}", color)
            if color == "C":
                returned = returned.replace("{color:long}", i18n("Cyan"))
            elif color == "M":
                returned = returned.replace("{color:long}", i18n("Magenta"))
            elif color == "Y":
                returned = returned.replace("{color:long}", i18n("Yellow"))
            elif color == "K":
                returned = returned.replace("{color:long}", i18n("Black"))
            elif color == "R":
                returned = returned.replace("{color:long}", i18n("Red"))
            elif color == "G":
                returned = returned.replace("{color:long}", i18n("Green"))
            elif color == "B":
                returned = returned.replace("{color:long}", i18n("Blue"))
            else:
                returned = returned.replace("{color:long}", "")

            return returned


        if document is None or originalLayer is None:
            # should not occurs, but...
            return None

        if not pProgress is None:
            stepTotal = 4
            for layer in OUTPUT_MODE_NFO[self.__outputOptions['outputMode']]['layers']:
                stepTotal+=len(layer['process'])

            pProgress.setRange(0, stepTotal)


        if originalLayerIsVisible == False:
            originalLayer.setVisible(True)

        # ----------------------------------------------------------------------
        # Create new group layer
        parentGroupLayer = document.createGroupLayer(parseLayerName(self.__outputOptions['layerGroupName'], ''))

        self.progressNext(pProgress)

        currentProcessedLayer = None

        for layer in OUTPUT_MODE_NFO[self.__outputOptions['outputMode']]['layers']:
            for process in layer['process']:
                if process['action'] == 'duplicate':
                    currentProcessedLayer = duplicateLayer(currentProcessedLayer, process['value'])
                elif process['action'] == 'new':
                    currentProcessedLayer = newLayer(currentProcessedLayer, process['value'])
                elif process['action'] == 'merge down':
                    currentProcessedLayer = mergeDown(currentProcessedLayer, process['value'])
                    pass
                elif process['action'] == 'blending mode':
                    applyBlendingMode(currentProcessedLayer, process['value'])
                elif process['action'] == 'filter':
                    applyFilter(currentProcessedLayer, process['value'])

                self.progressNext(pProgress)

            if not currentProcessedLayer is None:
                # rename currentProcessedLayer
                currentProcessedLayer.setName(parseLayerName(self.__outputOptions['layerColorName'], layer['color']))

        document.rootNode().addChildNode(parentGroupLayer, originalLayer)
        self.progressNext(pProgress)

        if self.__outputOptions['originalLayerAction'] == ORIGINAL_LAYER_KEEPVISIBLE:
            originalLayer.setVisible(True)
        elif self.__outputOptions['originalLayerAction'] == ORIGINAL_LAYER_KEEPHIDDEN:
            originalLayer.setVisible(False)
        elif self.__outputOptions['originalLayerAction'] == ORIGINAL_LAYER_REMOVE:
            originalLayer.remove()
        else:
            # ORIGINAL_LAYER_KEEPUNCHANGED
            originalLayer.setVisible(originalLayerIsVisible)


        self.progressNext(pProgress)

        document.refreshProjection()
        self.progressNext(pProgress)

        document.setActiveNode(parentGroupLayer)

        return parentGroupLayer


#ChannelsToLayers(Krita.instance()).process(Application.activeDocument(), Application.activeDocument().activeNode(), None)
#ChannelsToLayers(Krita.instance()).action_triggered()
