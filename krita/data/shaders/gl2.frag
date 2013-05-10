/*
 * shader for handling scaling
 */
uniform sampler2D texture0;
uniform vec2 textureScale;

varying vec2 out_uv0;

void main() {
    gl_FragColor = texture2D(texture0, out_uv0 * textureScale);
}
