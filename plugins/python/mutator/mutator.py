'''
Licensed under the MIT License.

Copyright (c) 2018 Eoin O'Neill <eoinoneill1991@gmail.com>
Copyright (c) 2018 Emmet O'Neill <emmetoneill.pdx@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
'''


import sys, math, random, colorsys
from PyQt5.QtWidgets import QWidget, QAction, QVBoxLayout, QSizePolicy, QFormLayout, QSlider, QPushButton, QLabel
from krita import Qt, Extension, DockWidget, DockWidgetFactory, SliderSpinBox


# Global mutation settings... 
# (Typically normalized within 0.0-1.0 range.
# Controlled via sliders within MutatorDocker GUI.)
nSizeMut = 0.5
nRotationMut = 1.0
nOpacityMut = 0.1
nFlowMut = 0.1
nHueMut = 0.2
nSaturationMut = 0.2
nValueMut = 0.1


# Usability-tuned maximum mutation values "constants"...
# (Think of these as the *largest possible mutation* for each parameter 
# when the slider is set to 100%. Can be modified to user taste!)
def sizeMutMax():
    #(lowThreshold, highThreshold, scale)
    return (10, 400, 0.25)
rotationMutMax = 180
opacityMutMax = 0.3
flowMutMax = 0.3
hueMutMax = 0.125
saturationMutMax = 0.3
valueMutMax = 0.25


class Mutator(Extension):
    ''' Mutator Class - Krita Extension 
    The Mutator Krita extension script randomly mutates some of the artist's 
    key brush and color settings by some configurable amount.
    (When the extension is active settings can be configured in Krita's GUI using sliders in the MutatorDocker.)
    '''
    def __init__(self,parent):
        super().__init__(parent)
    

    def setup(self):
        pass


    def createActions(self, window):
        '''
        Adds an "action" to the Krita menus, which connects to the mutate function.
        '''
        action = window.createAction("mutate", "Mutate", "tools/scripting")
        action.triggered.connect(self.mutate)
    
    
    def mutate(self):
        '''
        Mutates current brush/color/etc. settings by some user-configurable amount.
        Configurable settings are some percentage of a hard maximum amount for usability tuning.
        Mutation is triggered *manually* by the artist via action, hotkey, or button,
        whenever some randomness or brush/color variation is desired.
        '''
        window = Krita.instance().activeWindow()
        if window == None:
            return
        view = window.activeView()
        if view == None:
            return 
        if view.document() == None:
            return
        
        #Brush mutations...
        newSize = view.brushSize() + calculate_mutation(clamp(sizeMutMax()[0], sizeMutMax()[1], view.brushSize()) * sizeMutMax()[2], nSizeMut)
        view.setBrushSize(clamp(1, 1000, newSize))

        newRotation = view.brushRotation() + calculate_mutation(rotationMutMax, nRotationMut)
        view.setBrushRotation(newRotation)
        
        newOpacity = view.paintingOpacity() + calculate_mutation(opacityMutMax, nOpacityMut)
        view.setPaintingOpacity(clamp(0.01, 1, newOpacity))
        
        newFlow = view.paintingFlow() + calculate_mutation(flowMutMax, nFlowMut)
        view.setPaintingFlow(clamp(0.01, 1, newFlow))
        
        #Color mutations...
        color_fg = view.foregroundColor()
        preRGBA = color_fg.components()
        
        HSV = list(colorsys.rgb_to_hsv(preRGBA[0], preRGBA[1], preRGBA[2]))
        
        HSV[0] = (HSV[0] + calculate_mutation(hueMutMax, nHueMut))
        HSV[1] = clamp(0.01, 1, HSV[1] + calculate_mutation(saturationMutMax, nSaturationMut))
        HSV[2] = clamp(0, 1, HSV[2] + calculate_mutation(valueMutMax, nValueMut))

        postRGBA = list(colorsys.hsv_to_rgb(HSV[0], HSV[1], HSV[2]))
        postRGBA.append(preRGBA[3])
        
        color_fg.setComponents(postRGBA)
        view.setForeGroundColor(color_fg)


def calculate_mutation(mutationMax, nScale):
    '''
    mutationMax <- maximum possible mutation value. 
    nScale <- normalized (0.0..1.0) percentage (float). 
    Returns a randomized mutation value within range from -mutationMax..mutationMax, scaled by nScale.
    '''
    # return random.uniform(0, math.pi * 2) * mutationMax * nScale # Linear distribution (Evenly random.)
    return math.sin(random.uniform(0, math.pi * 2)) * mutationMax * nScale # Sine distribution (Randomness biased towards more extreme mutations.)


def clamp(minimum, maximum, input):
    '''
    Clamp input to some value between the minimum and maximum values. 
    Used to keep values within expected ranges.
    '''
    return min(maximum, max(input, minimum))


#GUI    
class MutatorDocker(DockWidget):
    ''' MutatorDocker - Krita DockWidget 
    This class handles the GUI elements that assign mutation values.
    Can be found inside Krita's Settings>Dockers menu.
    '''
    def __init__(self):
        super().__init__()
        
        self.setWindowTitle(i18n("Mutator"))
        
        # Create body, set widget and setup layout...
        body = QWidget(self)
        self.setWidget(body)
        body.setLayout(QVBoxLayout())
        body.setSizePolicy(QSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred))
        
        # Create mutation amount sliders...
        mutationSettings = QWidget()
        body.layout().addWidget(mutationSettings)

        mutationSettings.setLayout(QVBoxLayout())

        sizeMutSlider = SliderSpinBox().widget() # Size
        sizeMutSlider.setRange(0,100)
        sizeMutSlider.setPrefix(i18n("Size Mutation: "))
        sizeMutSlider.setSuffix("%")
        sizeMutSlider.valueChanged.connect(self.update_size_mut)
        sizeMutSlider.setValue(int(nSizeMut * 100))
        mutationSettings.layout().addWidget(sizeMutSlider)

        rotationMutSlider = SliderSpinBox().widget() # Rotation
        rotationMutSlider.setRange(0, 100)
        rotationMutSlider.setPrefix(i18n("Rotation Mutation: "))
        rotationMutSlider.setSuffix("%")
        rotationMutSlider.valueChanged.connect(self.update_rotation_mut)
        rotationMutSlider.setValue(int(nRotationMut * 100))
        mutationSettings.layout().addWidget(rotationMutSlider)
        
        opacityMutSlider = SliderSpinBox().widget() # Opacity
        opacityMutSlider.setRange(0, 100)
        opacityMutSlider.setPrefix(i18n("Opacity Mutation: "))
        opacityMutSlider.setSuffix("%")
        opacityMutSlider.valueChanged.connect(self.update_opacity_mut)
        opacityMutSlider.setValue(int(nOpacityMut * 100))
        mutationSettings.layout().addWidget(opacityMutSlider)
        
        flowMutSlider = SliderSpinBox().widget() # Flow
        flowMutSlider.setRange(0, 100)
        flowMutSlider.setPrefix(i18n("Flow Mutation: "))
        flowMutSlider.setSuffix("%")
        flowMutSlider.valueChanged.connect(self.update_flow_mut)
        flowMutSlider.setValue(int(nFlowMut * 100))
        mutationSettings.layout().addWidget(flowMutSlider)
        
        hueMutSlider = SliderSpinBox().widget() # FGC Hue
        hueMutSlider.setRange(0, 100)
        hueMutSlider.setPrefix(i18n("Hue Mutation: "))
        hueMutSlider.setSuffix("%")
        hueMutSlider.valueChanged.connect(self.update_fgc_hue_mut)
        hueMutSlider.setValue(int(nHueMut * 100))
        mutationSettings.layout().addWidget(hueMutSlider)
        
        saturationMutSlider = SliderSpinBox().widget() # FGC Saturation
        saturationMutSlider.setRange(0, 100)
        saturationMutSlider.setPrefix(i18n("Saturation Mutation: "))
        saturationMutSlider.setSuffix("%")
        saturationMutSlider.valueChanged.connect(self.update_fgc_saturation_mut)
        saturationMutSlider.setValue(int(nSaturationMut * 100))
        mutationSettings.layout().addWidget(saturationMutSlider)
        
        valueMutSlider = SliderSpinBox().widget() # FGC Value
        valueMutSlider.setRange(0, 100)
        valueMutSlider.setPrefix(i18n("Value Mutation: "))
        valueMutSlider.setSuffix("%")
        valueMutSlider.valueChanged.connect(self.update_fgc_value_mut)
        valueMutSlider.setValue(int(nValueMut * 100))
        mutationSettings.layout().addWidget(valueMutSlider)

        # Create mutate button...
        mutateButton = QPushButton(i18n("Mutate"))
        mutateButton.clicked.connect(self.trigger_mutate)
        body.layout().addWidget(mutateButton)
    
         
    # Slider event handlers...
    # Note: Sliders range from 0-100%, but global mutation state is normalized from 0.0-1.0.
    def update_size_mut(self, value):
        global nSizeMut 
        nSizeMut = value / 100


    def update_rotation_mut(self, value):
        global nRotationMut 
        nRotationMut = value / 100
    

    def update_opacity_mut(self, value):
        global nOpacityMut
        nOpacityMut = value / 100
    

    def update_flow_mut(self, value):
        global nFlowMut
        nFlowMut = value / 100


    def update_fgc_hue_mut(self, value):
        global nHueMut
        nHueMut = value / 100


    def update_fgc_saturation_mut(self, value):
        global nSaturationMut
        nSaturationMut = value / 100


    def update_fgc_value_mut(self, value):
        global nValueMut
        nValueMut = value / 100


    def trigger_mutate(self):
        Krita.instance().action("mutate").activate(QAction.Trigger)


    def canvasChanged(self, canvas): # Unused
        pass


# Krita boilerplate.
Krita.instance().addExtension(Mutator(Krita.instance()))
Krita.instance().addDockWidgetFactory(DockWidgetFactory("mutatorDocker", DockWidgetFactory.DockRight, MutatorDocker))