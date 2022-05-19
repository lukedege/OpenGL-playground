/*

06_procedural_base.vert: Vertex shader for the examples on procedural texturing. It is equal to 05_uv2color.vert

author: Davide Gadia

Real-Time Graphics Programming - a.a. 2021/2022
Master degree in Computer Science
Universita' degli Studi di Milano

*/

#version 410 core

// vertex position in world coordinates
layout (location = 0) in vec3 position;
// vertex normal
layout (location = 1) in vec3 normal;
// UV texture coordinates
layout (location = 2) in vec2 UV;
// the numbers used for the location in the layout qualifier are the positions of the vertex attribute
// as defined in the Mesh class

// model matrix
uniform mat4 modelMatrix;
// view matrix
uniform mat4 viewMatrix;
// Projection matrix
uniform mat4 projectionMatrix;
// Normal matrix
uniform mat3 normalMatrix;

//Lights
uniform vec3 pointLightPosition;

out vec3 lightDir; // we calculate per-vertex the direction of the light coming from the point light source
out vec3 vNormal;  		// we pass the vertex normal vector
out vec3 vViewPosition; // we pass the vector pointing to the camera, useful for specular component

// the output variable for UV coordinates
out vec2 interp_UV;

void main()
{
	// I assign the values to a variable with "out" qualifier so to use the per-fragment interpolated values in the Fragment shader
	interp_UV = UV;

	vec4 mvPosition = viewMatrix * modelMatrix * vec4(position, 1);
	vViewPosition = -mvPosition.xyz; // it would be camera pos - vertex pos in view coords, but camera in view coords is the origin thus is zero
	
	vec4 lightPos = viewMatrix * vec4(pointLightPosition, 1); // convert lightposition from world to view coordinates
	lightDir = lightPos.xyz - mvPosition.xyz; // vector from light to vertex position in view coords

	vNormal = normalize(normalMatrix * normal);

	// transformations are applied to each vertex
	gl_Position = projectionMatrix * mvPosition;
}
