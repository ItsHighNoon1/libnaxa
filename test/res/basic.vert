#version 460 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex;
layout (location = 2) in vec3 a_norm;
layout (location = 3) in ivec4 a_bone_ids;
layout (location = 4) in vec4 a_bone_weights;

out vec2 v_tex;
out vec3 v_norm;

out float v_debug;

const int MAX_BONES = 200;
const int MAX_BONE_WEIGHTS = 4;

uniform mat4 u_model;
uniform mat4 u_mvp;
uniform mat4 u_skeleton[MAX_BONES];

void main() {
    vec4 total_position = vec4(0.0);
    vec3 total_normal = vec3(0.0);
    v_debug = 0;
    for (int i = 0; i < MAX_BONE_WEIGHTS; i++) {
        if (a_bone_ids[i] < 0) {
            break;
        }
        if (a_bone_ids[i] >= MAX_BONES) {
            v_debug = a_bone_ids[i] / (2147483647.0);
            total_position = vec4(a_pos, 1.0);
            break;
            
        }
        vec4 local_position = u_skeleton[a_bone_ids[i]] * vec4(a_pos, 1.0);
        total_position += local_position * a_bone_weights[i];
        vec3 local_normal = mat3(u_skeleton[a_bone_ids[i]]) * a_norm;
        total_normal += local_normal;

        
    }
    total_position.w = 1.0;
    gl_Position = u_mvp * total_position;
    v_tex = a_tex;
    v_norm = mat3(u_model) * total_normal;
}
