// ui_button.frag
#version 410 core
out vec4 o_color;

in vec2 v_world_position;  // The position of the fragment

uniform vec4 color;
uniform vec2 resolution;      // The size of the button (width, height)
uniform float border_radius;  // The border radius
uniform vec2 center;          // The center position of the button

void main() {
    // Convert the fragment position to a coordinate system where (0,0) is at the center of the button
    vec2 pos = v_world_position - center;

    // Calculate half-size
    vec2 half_size = resolution * 0.5;

    // Calculate the distance from the edge, considering the border radius
    vec2 dist = abs(pos) - (half_size - vec2(border_radius));

    // If the distance is inside the border radius, render the pixel
    float dist_from_corner = length(max(dist, 0.0)) - border_radius;

    // Discard fragments outside the rounded rectangle
    if (dist_from_corner > 0.0) {
        discard;
    }

    o_color = color;
}