uniform sampler3D texture1;

void main() {
    vec4 col;

    if (viewportScale < 0.5 - eps) {
        col = filterPureLinear8(texture0, v_textureCoordinate.st);
    } else {
        col = texture2D(texture0, v_textureCoordinate.st);
    }

    fragColor = OCIODisplay(col, texture1);
}
