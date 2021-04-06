#version 330 core

flat in vec3 frag_color;
out vec4 color;

void main()
{
    color = vec4(frag_color, 1.0f);
	// color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
	// float r, g, b;
    // notice the 256's instead of 255
    // notice the 256's instead of 255
    // r = (gl_PrimitiveID % 256) / 255.0f;
    // g = ((gl_PrimitiveID / 256) % 256) / 255.0f;
    // b = ((gl_PrimitiveID / (256 * 256)) % 256) / 255.0f;
    // color = vec3(r, g, b);
}