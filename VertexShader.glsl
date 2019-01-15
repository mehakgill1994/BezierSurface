#version 410

layout(location = 0) in vec3 position;

out vec3 vsPosition;

void main()
{
	vsPosition = position;//notice that we don't need gl_Position
	//this is because the final position is calculated later
}