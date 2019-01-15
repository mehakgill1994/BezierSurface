#version 410

//minimal geometry shader, just to add the mesh wireframe in the fragment shader

///we expect to receive triangles from the tessellation evaluation shader
layout(triangles) in;
//we're going to output one triangle per input triangle
layout(triangle_strip, max_vertices = 3) out;

//inputs for each triangle, received from the tessellation evaluation shader

in vec3 teViewNormal[3];
in vec3 teToEye[3];
in vec3 teColor[3];

/*
Implicitly defined:
in gl_PerVertex
{
	vec4 gl_Position;
} gl_in[3];
*/

//pass-though output variables for each vertex

//out vec4 gl_Position; //implicit
out vec3 gsViewNormal;
out vec3 gsToEye;
out vec3 gsColor;
out vec3 gsBaryCoords; //this will help us to change the color near triangle edges

//barycentric coordinates for the 3 triangle vertices

const vec3 barycentricCoords[3] = vec3[3](
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1)
);


void main()
{
	for (uint i = 0; i < 3; i++)
	{
		gl_Position = gl_in[i].gl_Position; 
		gsViewNormal = teViewNormal[i];
		gsToEye = teToEye[i];
		gsColor = teColor[i];

		gsBaryCoords = barycentricCoords[i];

		EmitVertex();//add the current vertex to the current triangle strip
	}

	EndPrimitive();//end the current triangle strip
}