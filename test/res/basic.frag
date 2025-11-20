#version 460 core

in vec2 v_tex;
in vec3 v_norm;

in float v_debug;

out vec4 o_frag_color;

uniform sampler2D u_texture;

void main() {
    o_frag_color = texture(u_texture, v_tex);
    o_frag_color = vec4(v_debug, v_debug, v_debug, 1);
}
