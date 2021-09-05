/*
 *  shader for handling scaling
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform sampler2D texture0;

varying mediump vec4 v_textureCoordinate;

#ifdef USE_OCIO
#ifndef USE_OCIO_V2
uniform sampler3D texture1;
#endif
#endif /* USE_OCIO */


void main() {
    vec4 col = texture2D(texture0, v_textureCoordinate.st);


#ifdef USE_OCIO
#ifdef USE_OCIO_V2
    gl_FragColor = OCIODisplay(col);
#else /* USE_OCIO_V2 */
    gl_FragColor = OCIODisplay(col, texture1);
#endif /* USE_OCIO_V2 */
#else /* USE_OCIO */
    gl_FragColor = col;
#endif /* USE_OCIO */
}
