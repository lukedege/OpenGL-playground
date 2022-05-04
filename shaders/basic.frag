/*

00_basic.frag : (VERY) basic Fragment shader

*/

#version 410 core

// output variable for the fragment shader. Usually, it is the final color of the fragment
out vec4 color;

void main()
{
    color = vec4(1.0,0.0,0.0,1.0);
}
