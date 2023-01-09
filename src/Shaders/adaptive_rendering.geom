
#version 330 core

layout(points) in;
layout(points, max_vertices = 1) out;

uniform vec4 cam;   //cx, cy, fx, fy
uniform float cols;
uniform float rows;
uniform mat4 t_inv;
uniform mat4 pose;
uniform float maxDepth;
uniform float threshold;

in vec4 vColor[];
in vec4 vPosition[];
in vec4 vNormRad[];

out vec4 vPosition0;
out vec4 vColor0;
out vec4 vNormRad0;

#include "color.glsl"

void main()
{
    vec4 vPosHome = t_inv * vec4(vPosition[0].xyz, 1.0);
    vPosHome.w = vPosition[0].w;

    vec4 vNormRadHome = vec4(normalize(mat3(t_inv) * vNormRad[0].xyz), vNormRad[0].w);

    uvec4 srgb = decodeColor(vColor[0].x);

    if(vPosHome.z >= maxDepth || vPosHome.z <= 1.f)
    {
        gl_Position = vec4(-10, -10, 1, 0);
    }
    else
    {
        if(vPosHome.z > 5)
        {
            vec3 tmpNorm = vec3(0, 0, 1);
            // face to axis
            vNormRad0 = vec4(normalize(mat3(pose) * tmpNorm), vNormRad[0].w);
        }
        else
        {
            vec3 eyeToVert = vPosHome.xyz;
            float cosAngle = dot(eyeToVert, vNormRadHome.xyz) / (length(eyeToVert) * length(vNormRadHome.xyz));

            float radius = vNormRadHome.w / (1.0 + 0.5 * abs(cosAngle));

            vNormRad0 = vec4(vNormRad[0].xyz, radius);
        }

        vPosition0 = vPosition[0];
        vColor0 = vColor[0];

        EmitVertex();

        EndPrimitive();
    }
}
