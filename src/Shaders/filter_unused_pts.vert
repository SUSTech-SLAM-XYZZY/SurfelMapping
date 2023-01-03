#version 330 core

layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vColorTime;
layout (location = 2) in vec4 vNormRad;

flat out vec4 positionConf;
flat out vec4 colorTime;
flat out vec4 normRadii;
flat out int emit;

void main()
{
    emit = 1;

    int mark = int(round(vColorTime.y));

    // Filter out -10 (unused pts)
    if(mark < -5.f)
    {
        emit = 0;
    }

    positionConf = vPosition;
    colorTime = vColorTime;
    normRadii = vNormRad;
}

