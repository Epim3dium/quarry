
uniform vec2 u_src_pos;
uniform float u_src_range;
uniform float u_src_intensity;
uniform sampler2D currentTexture;
uniform vec2 u_textureSize;

float map(float value, float min1, float max1, float min2, float max2) {
  return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}
void main(void) {
    vec2 coord = vec2( gl_FragCoord.x, gl_FragCoord.y);
    vec2 pos = coord - u_src_pos;
    float dist = sqrt(pos.x * pos.x + pos.y * pos.y);
    dist = map(clamp(u_src_range - dist, 0.0, u_src_range), 0.0, u_src_range, 0.0, 1.0);
    dist *= dist;

    vec4 color = texture2D(currentTexture, gl_TexCoord[0].xy);
    color.xyz *= dist * u_src_intensity;
    gl_FragColor = color; 
}
