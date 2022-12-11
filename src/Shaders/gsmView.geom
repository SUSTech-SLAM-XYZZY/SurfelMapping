
#version 330 core

layout(points) in;
layout(points, max_vertices = 1) out;

in vec4 vPosition[];
in vec4 vColor[];
in vec4 vNormRad[];

out vec4 vPosition0;
out vec4 vColor0;
out vec4 vNormRad0;

void main() 
{
    //Emit a vertex if either we have an update, or a new unstable vertex
	float mark = vColor[0].y;
    if(mark >= 0.f)
    {
	    vPosition0 = vPosition[0];
	    vColor0 = vColor[0];
	    vNormRad0 = vNormRad[0];

	    EmitVertex();
		EndPrimitive();
    }
}
