#version 410

//per-vertex attributes for the entire patch
//these come from the vertex shader
in vec3 vsPosition[]; //size of array = number of control points in input patch

//tessellation levels chosen by the user

uniform float customOuterTessFactor;
uniform float customInnerTessFactor;

//specifies that the output patch will have 16 control points
layout(vertices = 16) out;

//per-control point outputs

out vec3 tcPosition[];//size of array = number of control points in output patch

//per-patch outputs

patch out vec3 patchColor;

/*
Implicitly defined variables:

in int gl_PrimitiveID; //index of the current output patch
in int gl_InvocationID; //index of the current output control point in the current output patch
in int gl_PatchVerticesIn; //number of control points in input patch

patch out float gl_TessLevelOuter[4];
patch out float gl_TessLevelInner[2];
*/

void main()
{
	//TODO
	tcPosition[gl_InvocationID] = vsPosition[gl_InvocationID];

	gl_TessLevelOuter[0] = customOuterTessFactor;
	gl_TessLevelOuter[1] = customOuterTessFactor;
	gl_TessLevelOuter[2] = customOuterTessFactor;
	gl_TessLevelOuter[3] = customOuterTessFactor;

	gl_TessLevelInner[0] = customInnerTessFactor;
	gl_TessLevelInner[1] = customInnerTessFactor;

	if(gl_PrimitiveID%7==0)
		patchColor = vec3(0, 0, 10);
	else if(gl_PrimitiveID%7==1)
		patchColor = vec3(0, 10, 0);
	else if(gl_PrimitiveID%7==2)
		patchColor = vec3(10, 0, 0);
	else if(gl_PrimitiveID%7==3)
		patchColor = vec3(10, 0, 10);
	else if(gl_PrimitiveID%7==4)
		patchColor = vec3(0, 10, 10);
	else if(gl_PrimitiveID%7==5)
		patchColor = vec3(10, 10, 0);
	else if(gl_PrimitiveID%7==6)
		patchColor = vec3(10, 10, 10);
	
}