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

// New uniforms for cylindrical projection
//uniform float pointRadius = 0.001;    //# uiname=Point Radius (m); min=0.001; max=10
//uniform float trimRadius = 1000000; //# uiname=Trim Radius; min=1; max=1000000
uniform float hfov = 35;            //# uiname=Horizontal field of view; min=5; max=175; scaling=linear

// Parameters of our cylindrical screen
uniform float cyl_radius = 0.5;
uniform float cyl_height = 0.5;
// Camera position
uniform vec4 campos = vec4(0);

//uniform float minPointSize = 0;
//uniform float maxPointSize = 600.0;
// Point size multiplier to get from a width in projected coordinates to the
// number of pixels across as required for gl_PointSize
uniform float pointPixelScale = 0;
uniform vec3 cursorPos = vec3(0);
uniform int fileNumber = 0;
in float intensity;

in int returnNumber;
in int numberOfReturns;
in int pointSourceId;
in int classification;

flat out float modifiedPointRadius;
flat out float pointScreenSize;

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
    // Position of vertex WITHOUT a perspective transformation.
    vec4 pv = (v_matrix * m_matrix * position);

    // if perspective/orthographic:
    //gl_Position = (p_matrix * v_matrix * m_matrix * position);
    // else:
    vec4 ray = pv - campos;
    vec3 rho_phi_z; // polar coordinates of ray
    rho_phi_z[0] = sqrt (ray.x * ray.x + ray.y * ray.y);
    rho_phi_z[1] = atan (ray.y, ray.x); // care may be required. -pi to pi range.. atan (ray.y/ray.x) returns -pi/2 to pi/2.
    rho_phi_z[2] = ray.z;

    // Convert phi into a value between -1 and 1 as the x of our projected position.
    float x_screen = rho_phi_z[1] / 6.283185307; // 2 pi
    // theta is angle from xy plane to vertex
    float theta = asin (rho_phi_z[2]/rho_phi_z[0]);
    float y_screen = cyl_radius * tan (theta);

    gl_PointSize = 10;
    //gl_Position = vec4(x_screen, 0.5, -1.0, 1.0);
    gl_Position = (p_matrix * v_matrix * m_matrix * position);
    //gl_Position[0] = x_screen;
    //gl_Position[0] = theta; // seems to be unsensible

    //vertex.color = vec4(color, alpha);
    vertex.color = vec4(1.0, 0.0, 0.0, 1.0); // Hackety hack

    vertex.fragpos = vec3(m_matrix * position); // within-model position of fragment, used for lighting
    vertex.normal = normalin;

    /*
    vec4 p0 = (v_matrix * m_matrix * position);
    // Here we do our own cylindrical projection inside the vertex shader.
    // This is the right projection if you're standing at the centre of a
    // circularly curved screen.
    //
    // Notes:
    //
    // * Projected x should be an angle in the camera's xz plane
    //
    // * Projected y should be projected by dividing by the xz distance
    //
    // * Projected z is only necessary for the z-buffer, and we compute it the
    //   same way as usual for simplicity.
    //
    // OpenGL will automatically divide by gl_Position.w to do what it assumes
    // is a normal perspective projection.  For consistency with depth
    // calculations in the fragment shader, we'd like to avoid completely
    // subverting the w component, so we set it equal to xzlen (with sign(p0.z)
    // to ensure proper culling behind the camera).  This means we need to
    // factor it into the x and z components so OpenGL can remove it
    // again in the unwanted perspective divide.
    //
    float hfovScale = hfov * 0.008726646259971648;
    float vfov = hfovScale * p_matrix[0][0] / p_matrix[1][1];
    float xzlen = length(p0.xz);
    float theta = atan(-p0.x/p0.z);

    float projZ = (p_matrix[2][2] * p0.z + p_matrix[2][3]) / (p_matrix[3][2] * p0.z);

    vec4 p = vec4(
        xzlen * theta / hfovScale,
        p0.y / atan(vfov),
        xzlen * projZ,
        xzlen * -sign(p0.z)
    );

    gl_PointSize = 1;

    gl_Position = p;

    vertex.color = vec4(color, alpha);
    vertex.fragpos = vec3(p);
    vertex.normal = normalin;
    */
}
