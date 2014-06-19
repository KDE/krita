uniform sampler3D texture1;

void main() {
    vec4 col = texture2D(texture0, v_textureCoordinate.st);
    gl_FragColor = OCIODisplay(col, texture1);
}
