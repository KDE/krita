/*
KDE JavaScript invert script.

The JavaScript invert script inverts all pixels at the activate layer in
the current image.

Copyright (c) 2006 Sebastian Sauer <mail@dipe.org>
Published under the GNU GPL v2 or higher
*/

// fetch the image.
var image = Krita.image();

// we like to manipulate the active painting layer.
var layer = image.activePaintLayer();

// get the height and the width the layer has.
var width = layer.width();
var height = layer.height();

// we like to use the progressbar
var progress = Krita.progress();
progress.setProgressTotalSteps(width * height);

// tell Krita that painting starts. the whole painting session will be
// counted till layer.endPainting() was called as one undo/redo-step.
layer.beginPainting("invert");

// create an iterator to walk over all pixels the layer has.
var it = layer.createRectIterator(0, 0, width, height);

// iterate over all pixels and invert each pixel.
while( ! it.isDone() ) {
    // invert the color of the pixel.
    it.invertColor();

    // increment the progress to show, that work on this pixel is done.
    progress.incProgress();

    // go to the next pixel.
    it.next();
}

// painting is done now.
layer.endPainting();
