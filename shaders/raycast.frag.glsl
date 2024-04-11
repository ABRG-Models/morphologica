#version 420 core

out vec4 fragment_colour;

// Must be the exact same name as declared in the vertex shader
// ------------------------------------------------------------
in vec3 vert_pos_transformed; // Transformed vertex position coordinates received as interpolated
in vec3 vertex_normal;
in vec2 texture_coordinates;
in float aPos_dist_from_ray_intersect;

uniform sampler2D image;
uniform vec3 camera_position; // camera_position is set in main() on the CPU side

void main()
{
    vec3 view_direction = normalize(camera_position - vert_pos_transformed);

    vec3 light_position = vec3(0.0, 20.0, 0.0); // Light position in 3D space
    vec3 light_direction = normalize(vec3(light_position - vert_pos_transformed));

    vec4 image_colour = texture(image, texture_coordinates);

    float ambient_factor = 0.95; // Intensity multiplier
    vec4 ambient_result = vec4(ambient_factor * image_colour.rgb, 1.0);

    float diffuse_factor = 0.75;
    float diffuse_angle = max(dot(light_direction, vertex_normal), -0.1); // [-1.0 to 0] Results in darker lighting past 90 degrees
    vec4 diffuse_result =  vec4(diffuse_factor * diffuse_angle * image_colour.rgb, 1.0);

    vec3 specular_colour = vec3(0.5, 0.5, 0.5);
    vec3 reflect_direction = normalize(reflect(-light_direction, vertex_normal)); // Notice the light direction is negated here
    float specular_strength = pow(max(dot(view_direction, reflect_direction), 0), 32);
    vec4 specular_result = vec4(specular_colour * specular_strength, 1.0);

    fragment_colour = ambient_result + diffuse_result + specular_result;

    if (aPos_dist_from_ray_intersect > 0) {
        fragment_colour.g += 0.1 / aPos_dist_from_ray_intersect;
        // fragment_colour.a -= 0.5 / aPos_dist_from_ray_intersect;
    }
}
