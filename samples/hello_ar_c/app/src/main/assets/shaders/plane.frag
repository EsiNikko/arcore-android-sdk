precision highp float;
precision highp int;

uniform sampler2D u_Texture;
varying vec2 v_TextureCoords;
varying float v_Alpha;
varying vec3 v_ViewPosition;
varying vec3 v_ScreenSpacePosition;

void main() {
  float r = texture2D(u_Texture, v_TextureCoords).r;
  gl_FragColor = vec4(r * v_Alpha);
}
