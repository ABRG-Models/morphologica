// The coded-in shaders tell non-Mac platforms that they use OpenGL 4.5, but Mac limited to 4.1
#version 410

// ProjMatrix * RotnMatrix operation can be carried out on CPU with a single matrix
//uniform mat4 mvp_matrix;
// Or, and this is important for lighting effects and possibly text, too, matrices can be passed separately
//uniform mat4 vp_matrix; // sceneview-projection matrix
uniform mat4 m_matrix; // model matrix
uniform mat4 v_matrix; // scene view matrix
uniform mat4 p_matrix; // projection matrix
// alpha - to make a model see-through
uniform float alpha;
// Parameters of our cylindrical screen
uniform float cyl_radius = 0.005;
uniform float cyl_height = 0.02;
// Camera position
uniform vec4 cyl_cam_pos = vec4(0);

// My original inputs
layout(location = 0) in vec4 position; // Attrib location 0. vertex position
layout(location = 1) in vec4 normalin; // Attrib location 1. vertex normal
layout(location = 2) in vec3 color;    // Attrib location 2. vertex colour

out VERTEX
{
    vec4 normal;
    vec4 color;   // Could make vec4 and incorporate alpha
    vec3 fragpos; // fragment position
} vertex;

void main (void)
{
    const float pi = 3.1415927;
    const float two_pi = 6.283185307;
    const float heading_offset = 1.570796327; // pi/2 but maybe pass in?
    // Transform vertex position with scene view and model view matrices
    vec4 pv = (v_matrix * m_matrix * position);
    vec4 ray = pv - (v_matrix * cyl_cam_pos);
    vec3 rho_phi_z; // polar coordinates of ray
    rho_phi_z[0] = sqrt (ray.x * ray.x + ray.y * ray.y);
    rho_phi_z[1] = atan (ray.y, ray.x) - heading_offset; // glsl atan(y,x) like std::atan2(y,x)
    if (rho_phi_z[1] > pi) { rho_phi_z[1] = rho_phi_z[1] - two_pi; }
    if (rho_phi_z[1] < -pi) { rho_phi_z[1] = rho_phi_z[1] + two_pi; }
    rho_phi_z[2] = ray.z;

    // Convert phi into a value between -1 and 1 as the x of our projected position.
    float x_s = -rho_phi_z[1] / pi;
    float y_s = 0.0;
    if (x_s != 0.0) {
        // theta is angle from xy plane to vertex
        float theta = asin (rho_phi_z[2] / rho_phi_z[0]);
        y_s = (cyl_radius * tan (theta)) / cyl_height;
        gl_PointSize = 1;
        gl_Position = vec4(x_s, y_s, -1.0, 1.0);
        vertex.color = vec4(color, alpha);
        vertex.fragpos = vec3(m_matrix * position); // within-model position of fragment, used for lighting
        vertex.normal = normalin;
    } else {
        gl_Position = vec4(0.0, 0.0, -100.0, 1.0);
        vertex.color = vec4(color, 0.0);
        vertex.fragpos = vec3(m_matrix * position);
        vertex.normal = normalin;
    }
}
