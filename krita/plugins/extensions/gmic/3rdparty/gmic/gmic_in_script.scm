;;
;;   File        : gmic_in_script.scm
;;                 ( Scheme script for GIMP )
;;
;;   Description : Show how to call G'MIC commands from a GIMP script.
;;                 ( http://gmic.eu )
;;
;;   Copyright   : David Tschumperle
;;                 ( http://tschumperle.users.greyc.fr/ )
;;
;;   License     : CeCILL v2.0
;;                 ( http://www.cecill.info/licences/Licence_CeCILL_V2-en.html )
;;
;;   This software is governed by the CeCILL  license under French law and
;;   abiding by the rules of distribution of free software.  You can  use,
;;   modify and/ or redistribute the software under the terms of the CeCILL
;;   license as circulated by CEA, CNRS and INRIA at the following URL
;;   "http://www.cecill.info".
;;
;;   As a counterpart to the access to the source code and  rights to copy,
;;   modify and redistribute granted by the license, users are provided only
;;   with a limited warranty  and the software's author,  the holder of the
;;   economic rights,  and the successive licensors  have only  limited
;;   liability.
;;
;;   In this respect, the user's attention is drawn to the risks associated
;;   with loading,  using,  modifying and/or developing or reproducing the
;;   software by the user in light of its specific status of free software,
;;   that may mean  that it is complicated to manipulate,  and  that  also
;;   therefore means  that it is reserved for developers  and  experienced
;;   professionals having in-depth computer knowledge. Users are therefore
;;   encouraged to load and test the software's suitability as regards their
;;   requirements in conditions enabling the security of their systems and/or
;;   data to be ensured and,  more generally, to use and operate it in the
;;   same conditions as regards security.
;;
;;   The fact that you are presently reading this means that you have had
;;   knowledge of the CeCILL license and that you accept its terms.
;;

(define	(script-with-gmic img drawable x y z)

  ;; Start undo group.
  (gimp-image-undo-group-start img)

  (let* (
         (copy-layer (car (gimp-layer-copy drawable TRUE)))
         )

    ;; Add a copy of the layer to the image.
    (gimp-image-add-layer img copy-layer -1)

    ;; Render a 3D mapped cube from the active layer, using G'MIC.
    (plug-in-gmic 1 img drawable 1
                  (string-append
                   "-v - " ; To have a silent output. Remove it to display errors from the G'MIC interpreter on stderr.
                   "-gimp_imageobject3d 1,{w},{h},0.5,"
                   (number->string x) ","
                   (number->string y) ","
                   (number->string z) ",45,0,0,-100,0.5,0.7,4"
                   ))

    ;; Merge two layers together, using the G'MIC 'edges' mode (this layer mode does not exist by default in GIMP).
    (plug-in-gmic 1 img drawable 2 "-v - -compose_edges 1")

    )

  ;; Flush display.
  (gimp-displays-flush)

  ;; End undo group.
  (gimp-image-undo-group-end img)
  )

(script-fu-register "script-with-gmic"
                    _"<Image>/Filters/G'MIC Script test..."
                    "Show how to call G'MIC from a GIMP script"
                    "David Tschumperlé"
                    "David Tschumperlé"
                    "October 2009"
                    "*"
                    SF-IMAGE		"Image"		0
                    SF-DRAWABLE         "Drawable"      0
                    SF-ADJUSTMENT	_"X-Angle"	'(57  0 360  1 2 0 0)
                    SF-ADJUSTMENT	_"Y-Angle"	'(41  0 360  1 2 0 0)
                    SF-ADJUSTMENT	_"Z-Angle"	'(21  0 360  1 2 0 0)
                    )
