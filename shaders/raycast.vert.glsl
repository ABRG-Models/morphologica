#version 420 core

layout (location = 0) in vec3 aPos; // Attribute data: vertex(s) X, Y, Z position received via VBO on the CPU side
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 vertex_normal;
out vec2 texture_coordinates;
out vec3 vert_pos_transformed; // Transformed model vertex position passed to fragment shader for lighting

out float aPos_dist_from_ray_intersect;

uniform mat4 sphere_trans_mat;
uniform vec3 ray_intersect_point; // Where the ray hits the sphere

uniform mat4 view;
uniform mat4 projection;

void main()
{
    texture_coordinates = aTexCoord;
    vert_pos_transformed = vec3(sphere_trans_mat * vec4(aPos, 1.0));

    mat3 normal_matrix = transpose(inverse(mat3(sphere_trans_mat)));
    vertex_normal = normal_matrix * aNormal;

    if (length(vertex_normal) > 0) {
        vertex_normal = normalize(vertex_normal);
    }

    aPos_dist_from_ray_intersect = abs(length(vert_pos_transformed - ray_intersect_point));

    gl_Position = projection * view * sphere_trans_mat * vec4(aPos, 1.0);
}
