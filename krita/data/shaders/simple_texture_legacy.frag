/*
 *  shader for handling scaling
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

uniform sampler2D texture0;

varying mediump vec4 v_textureCoordinate;

void main() {
    vec4 col = texture2D(texture0, v_textureCoordinate.st);


#ifdef USE_OCIO
    gl_FragColor = OCIODisplay(col);
#else /* USE_OCIO */
    gl_FragColor = col;
#endif /* USE_OCIO */
}
