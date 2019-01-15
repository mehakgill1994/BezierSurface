#version 410

//the first parameter specifies our abstract patch is a quad (other options: triangles, isolines)
//the second parameter specifies the tessellation method, but it only matters when the tessellation factors are not integers
//the third parameter specifies that the generated triangles should have counter-clockwise winding
layout(quads, equal_spacing, cw) in;
//you may need to change the triangle winding order depending on your implementation of the assignment

//control points received from the tessellation control shader
in vec3 tcPosition[];

//per-patch inputs received from the tessellation control shader

patch in vec3 patchColor;


//uniform variables

uniform mat4 matWorld;
uniform mat4 matView;
uniform mat4 matProjection;

out vec3 teViewNormal;
out vec3 teToEye;
out vec3 teColor;

/*
Implicitly defined:

in vec3 gl_TessCoord; //coordinates of the current point on the abstract patch (only x & y are used for quads and isolines)
in int gl_PatchVerticesIn; //number of control points in patch received from tessellation control shader
in int gl_PrimitiveID; //index of the current patch received from tessellation control shader
out vec4 gl_Position;
*/


void main()
{
	//TODO
	float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

	//16 control points
	vec3 p00 = tcPosition[0];
	vec3 p10 = tcPosition[1];
	vec3 p20 = tcPosition[2];
	vec3 p30 = tcPosition[3];

	vec3 p01 = tcPosition[4];
	vec3 p11 = tcPosition[5];
	vec3 p21 = tcPosition[6];
	vec3 p31 = tcPosition[7];
	
	vec3 p02 = tcPosition[8];
	vec3 p12 = tcPosition[9];
	vec3 p22 = tcPosition[10];
	vec3 p32 = tcPosition[11];
   
	vec3 p03 = tcPosition[12];
	vec3 p13 = tcPosition[13];
	vec3 p23 = tcPosition[14];
	vec3 p33 = tcPosition[15];

	float u0 = (1.-u)*(1.-u)*(1.-u);
	float u1 = 3.*u*(1.-u)*(1.-u);
	float u2 = 3.*u*u*(1.-u);
	float u3 = u*u*u;


	float v0 = (1.-v)*(1.-v)*(1.-v);
	float v1 = 3.*v*(1.-v)*(1.-v);
	float v2 = 3.*v*v*(1.-v);
	float v3 = v*v*v;

	float du0 = -3.*(1.-u)*(1.-u);
	float du1 = 3.*(1.-u) * (1.-(3.*u));
	float du2 = 3.*u*(2.-(3.*u));
	float du3 = 3.*u*u;
	
	float dv0 = -3.*(1.-v)*(1.-v);
	float dv1 = 3.*(1.-v) * (1.-(3.*v));
	float dv2 = 3.*v*(2.-(3.*v));
	float dv3 = 3.*v*v;

	//bezier surface formula
	vec3 pos = (u0*(v0*p00 + v1*p01 + v2*p02 + v3*p03)) + (u1*(v0*p10 + v1*p11 + v2*p12 + v3*p13)) + (u2*(v0*p20 + v1*p21 + v2*p22 + v3*p23)) + (u3*(v0*p30 + v1*p31 + v2*p32 + v3*p33)); 
	
	//float x = (u0*(v0*p00.x + v1*p01.x + v2*p02.x + v3*p03.x)) + (u1*(v0*p10.x + v1*p11.x + v2*p12.x + v3*p13.x)) + (u2*(v0*p20.x + v1*p21.x + v2*p22.x + v3*p23.x)) + (u3*(v0*p30.x + v1*p31.x + v2*p32.x + v3*p33.x)); 
	//float y = (u0*(v0*p00.y + v1*p01.y + v2*p02.y + v3*p03.y)) + (u1*(v0*p10.y + v1*p11.y + v2*p12.y + v3*p13.y)) + (u2*(v0*p20.y + v1*p21.y + v2*p22.y + v3*p23.y)) + (u3*(v0*p30.y + v1*p31.y + v2*p32.y + v3*p33.y));
	//float z = (u0*(v0*p00.z + v1*p01.z + v2*p02.z + v3*p03.z)) + (u1*(v0*p10.z + v1*p11.z + v2*p12.z + v3*p13.z)) + (u2*(v0*p20.z + v1*p21.z + v2*p22.z + v3*p23.z)) + (u3*(v0*p30.z + v1*p31.z + v2*p32.z + v3*p33.z));

	gl_Position =  matView * matWorld * vec4(pos, 1);

	//differentiation of bezier surface formula w.r.t. 'u' and 'v'
	vec3 dpdu = (du0*(v0*p00 + v1*p01 + v2*p02 + v3*p03)) + (du1*(v0*p10 + v1*p11 + v2*p12 + v3*p13)) + (du2*(v0*p20 + v1*p21 + v2*p22 + v3*p23)) + (du3*(v0*p30 + v1*p31 + v2*p32 + v3*p33)); 
	vec3 dpdv = (u0*(dv0*p00 + dv1*p01 + dv2*p02 + dv3*p03)) + (u1*(dv0*p10 + dv1*p11 + dv2*p12 + dv3*p13)) + (u2*(dv0*p20 + dv1*p21 + dv2*p22 + dv3*p23)) + (u3*(dv0*p30 + dv1*p31 + dv2*p32 + dv3*p33)); 
	
	//normal is in the direction perpendicular to the direction of both u and v i.e. given by the cross product between two
	teViewNormal = cross(-dpdu, dpdv);

	//mathematical way to compute cross product
	//teViewNormal = normalize(vec3(dpdu.y*dpdv.z - dpdu.z*dpdv.y, dpdu.z*dpdv.x - dpdu.x*dpdv.z, dpdu.x*dpdv.y - dpdu.y*dpdv.x));
		
	//transforming normal in the world and view space
	mat3 normalTransform = mat3(matView * matWorld);
	teViewNormal = normalize(normalTransform * teViewNormal);

	teColor = patchColor;
	teToEye = -normalize(gl_Position.xyz);
	

	gl_Position = matProjection * gl_Position;
}

