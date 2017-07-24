;high-pass.scm
; by Rob Antonishen
; http://ffaat.pointclark.net

; Version 1.2 (20120427)

; Description
;
; This implements a highpass filter using the blur and invert method
; parameters are blur radius, a preserve colour toggle, and whether to keep the original layer
;
; Changes:
; v1.1 fixes "bug" created by bug fix for gimp 2.6.9 in gimp-histogram
; v1.2 changed to a different layer blend mode (thanks Blacklemon67) that
;      has less histogram banding, by eliminating the need for the contrast boost.
;

; License:
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version. 
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; The GNU Public License is available at
; http://www.gnu.org/copyleft/gpl.html

(define (high-pass img inLayer inRadius inMode inKeepOrig)
  (let*
    (
	  (blur-layer 0)
	  (blur-layer2 0)
	  (working-layer 0)
	  (colours-layer 0)
	  (colours-layer2 0)
	  (orig-name (car (gimp-drawable-get-name inLayer)))
    )
    ;  it begins here
    (gimp-context-push)
    (gimp-image-undo-group-start img)
	
	;if keep original selected, make a working copy
	(if (= inKeepOrig TRUE)
      (begin
        (set! working-layer (car (gimp-layer-copy inLayer FALSE)))
	    (gimp-image-add-layer img working-layer -1)
		(gimp-drawable-set-name working-layer "working")
	  )
	  (set! working-layer inLayer)  ;else
	)   

	(gimp-image-set-active-layer img working-layer)

	;if keeping colours
	(if (or (= inMode 1) (= inMode 3))
      (begin
        (set! colours-layer (car (gimp-layer-copy inLayer FALSE)))
	    (gimp-image-add-layer img colours-layer -1)
	    (gimp-image-lower-layer img colours-layer)	
		(gimp-drawable-set-name colours-layer "colours")
		(gimp-image-set-active-layer img working-layer)
	  )
	)   
	
    ;if Greyscale, desaturate
	(if (or (= inMode 2) (= inMode 3))
	  (gimp-desaturate working-layer) 	
	)
	
	;Duplicate on top and blur
    (set! blur-layer (car (gimp-layer-copy working-layer FALSE)))
    (gimp-image-add-layer img blur-layer -1)
	(gimp-drawable-set-name blur-layer "blur")

	;blur
    (plug-in-gauss 1 img blur-layer inRadius inRadius 1) 	
	
	(if (<= inMode 3)
	  (begin
        ;v1.2 using grain extract...
        (gimp-layer-set-mode blur-layer GRAIN-EXTRACT-MODE)
	    (set! working-layer (car (gimp-image-merge-down img blur-layer 0)))
		
	    ; if preserve chroma, change set the mode to value and merge down with the layer we kept earlier.
	    (if (= inMode 3)
          (begin
            (gimp-layer-set-mode working-layer VALUE-MODE)
		    (set! working-layer (car (gimp-image-merge-down img working-layer 0)))
	      )
	    )   

	    ; if preserve DC, change set the mode to overlay and merge down with the average colour of the layer we kept earlier.
	    (if (= inMode 1)
          (begin	
	        (gimp-context-set-foreground (list 
                (car (gimp-histogram colours-layer HISTOGRAM-RED 0 255)) 
                (car (gimp-histogram colours-layer HISTOGRAM-GREEN 0 255)) 
                (car (gimp-histogram colours-layer HISTOGRAM-BLUE 0 255)))
            )
		    (gimp-drawable-fill colours-layer FOREGROUND-FILL)
            (gimp-layer-set-mode working-layer OVERLAY-MODE)
		    (set! working-layer (car (gimp-image-merge-down img working-layer 0)))
	      )
	    )   
	  )
	  (begin  ;else 4=redrobes method
	  
	    (gimp-image-set-active-layer img blur-layer)  ;top layer
		
		;get the average colour of the input layer
	    (set! colours-layer (car (gimp-layer-copy inLayer FALSE)))
	    (gimp-image-add-layer img colours-layer -1)
		(gimp-drawable-set-name colours-layer "colours")

	    (gimp-context-set-foreground (list 
            (car (gimp-histogram colours-layer HISTOGRAM-RED 0 255)) 
            (car (gimp-histogram colours-layer HISTOGRAM-GREEN 0 255)) 
            (car (gimp-histogram colours-layer HISTOGRAM-BLUE 0 255)))
        )
		(gimp-drawable-fill colours-layer FOREGROUND-FILL)
	    (gimp-image-set-active-layer img colours-layer)

        ;copy the solid colour layer
		(set! colours-layer2 (car (gimp-layer-copy colours-layer FALSE)))
        (gimp-image-add-layer img colours-layer2 -1)		
        (gimp-layer-set-mode colours-layer SUBTRACT-MODE)
	    (gimp-image-set-active-layer img colours-layer2)

		;copy the blurred layer
		(set! blur-layer2 (car (gimp-layer-copy blur-layer FALSE)))
        (gimp-image-add-layer img blur-layer2 -1)		
        (gimp-layer-set-mode blur-layer2 SUBTRACT-MODE)
		
		(set! blur-layer (car (gimp-image-merge-down img colours-layer 0)))
		(set! blur-layer2 (car (gimp-image-merge-down img blur-layer2 0)))
		
        (gimp-layer-set-mode blur-layer SUBTRACT-MODE)
        (gimp-layer-set-mode blur-layer2 ADDITION-MODE)
		
	    (set! working-layer (car (gimp-image-merge-down img blur-layer 0)))
	    (set! working-layer (car (gimp-image-merge-down img blur-layer2 0)))
	
	  )
	)
	
	(if (= inKeepOrig TRUE)
      (gimp-drawable-set-name working-layer (string-append orig-name " high pass"))
      (gimp-drawable-set-name working-layer orig-name)
	)   
	
	;done
	(gimp-progress-end)
	(gimp-image-undo-group-end img)
	(gimp-displays-flush)
	(gimp-context-pop)
  )
)

(script-fu-register "high-pass"
        		    "<Image>/Filters/Generic/_High Pass Filter"
                    "Basic High Pass Filter."
                    "Rob Antonishen"
                    "Rob Antonishen"
                    "April 2012"
                    "RGB* GRAY*"
                    SF-IMAGE      "image"      0
                    SF-DRAWABLE   "drawable"   0
                    SF-ADJUSTMENT "Filter Radius" '(10 2 200 1 10 0 0)
                    SF-OPTION     "Mode" '("Colour" "Preserve DC" "Greyscale" "Greyscale, Apply Chroma" "Redrobes")
				    SF-TOGGLE     "Keep Original Layer?" TRUE
)