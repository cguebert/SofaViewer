#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec4 vPosition;
out vec4 vNormal;

#ifdef USE_TEXTURE
	layout (location = 2) in vec2 texCoord;
	out vec2 vTexCoord;
#endif

uniform mat4 MV;
uniform mat4 MVP;

void main()
{
	gl_Position = MVP * vec4(position, 1.0f);
	vPosition	= MV * vec4(position, 1.0f);
	vNormal 	= MV * vec4(normal, 0.0f);
	
	#ifdef USE_TEXTURE
		vTexCoord = vec2(texCoord.x, 1.0f - mgl_TexCoords.y);
	#endif
}
