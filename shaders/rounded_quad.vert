#version 410 core
layout (location = 0) in vec2 a_position;

out vec2 v_world_position;

uniform mat4 projection;
uniform mat4 model;

void main() {
    vec4 world_position = model * vec4(a_position, 0.0, 1.0);
    v_world_position = world_position.xy;
    gl_Position = projection * world_position;
}