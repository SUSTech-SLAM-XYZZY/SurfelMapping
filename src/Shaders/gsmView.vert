
#version 330 core
// point position
layout (location = 0) in vec2 texcoord;

out vec4 vPosition;
out vec4 vColor;
out vec4 vNormRad;

uniform isampler2D indexSampler;
uniform sampler2D vertConfSampler;
uniform sampler2D colorTimeSampler;
uniform sampler2D normRadSampler;

uniform mat4 pose;

void main() {
    vec3 posLocal_o;
    vec4 normRadLocal_o;

    float sub_x = texcoord.x;
    float sub_y = texcoord.y;
    int currentID = int(textureLod(indexSampler, vec2(sub_x, sub_y), 0.0));

    if(currentID > 0) // if it has a projection here
    {
        // get vertex
        vec4 vertConf = textureLod(vertConfSampler, vec2(sub_x, sub_y), 0.0);
        // get semantic & color
        vec4 colorTime = textureLod(colorTimeSampler, vec2(sub_x, sub_y), 0.0);

        vec4 normRad = textureLod(normRadSampler, vec2(sub_x, sub_y), 0.0); // normal vector
        posLocal_o = vertConf.xyz;
        normRadLocal_o = normRad;

        // connect to geom
        vPosition = pose * vec4(posLocal_o, 1.0);
        vPosition.w = vertConf.w;

        vColor = colorTime;

        vNormRad.xyz = mat3(pose) * normRadLocal_o.xyz;
        vNormRad.xyz = normalize(vNormRad.xyz);
        vNormRad.w = normRadLocal_o.w;
    }
    else  //============ Other vertex ============//
    {
        vPosition = vec4(0.0);
        vNormRad = vec4(0.0);
        vColor.y = -10.f;
    }
}
