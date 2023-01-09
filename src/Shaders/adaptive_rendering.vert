#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 color;
layout (location = 2) in vec4 normal;

uniform vec4 cam;   //cx, cy, fx, fy
uniform float cols;
uniform float rows;
uniform mat4 t_inv;
uniform mat4 pose;
uniform float maxDepth;
uniform float threshold;

out vec4 vColor;
out vec4 vPosition;
out vec4 vNormRad;

void main()
{
    vPosition = position;

    vColor = color;

    vNormRad = normal;

}
