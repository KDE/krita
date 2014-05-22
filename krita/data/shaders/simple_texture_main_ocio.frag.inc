uniform sampler3D texture1;

void main() {
    vec4 col = texture2D(texture0, v_textureCoordinate.st);
    fragColor = OCIODisplay(col, texture1);
}
