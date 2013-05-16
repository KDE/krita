uniform sampler2D texture0;

varying vec4 v_textureCoordinate;

void main() {
    gl_FragColor = texture2D(texture0, v_textureCoordinate.st);
}
