const char* fragmentShader = R"~~(#version 330 core
in vec4 vPosition;
in vec4 vNormal;

#ifdef USE_TEXTURE
	in vec2 vTexCoord;
	uniform sampler2D texture;
#else
	uniform vec4 diffuseColor;
#endif

out vec4 color;

void main()
{
	vec3 N = normalize(vNormal.xyz);
	vec3 L = normalize(vec3(0.0, 0.2, 1.0) - vPosition.xyz);
	vec3 E = normalize(-vPosition.xyz); // eyePos is (0,0,0)
	vec3 R = normalize(-reflect(L, N));
	
	// Ambient term
	vec4 ambient = vec4(0,0,0,0);
	
	// Diffuse Term
#ifdef USE_TEXTURE
	vec4 diffuse = texture2D(texture, vTexCoord);
#else
	vec4 diffuse = diffuseColor;
#endif
//	diffuse = diffuse * max(dot(N,L), 0.0);	// 1 faced
	diffuse = diffuse * abs(dot(N,L));		// 2 faced
	diffuse = clamp(diffuse, 0.0, 1.0);

	// Specular Term
	vec4 specular = vec4(1, 1, 1, 0) * pow(max(dot(R,E),0.0), 128.0 );
	specular = clamp(specular, 0.0, 1.0);

	// Write final color
	color = ambient + diffuse + specular;
}
)~~";

const char* vertexShader = R"~~(#version 330 core
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
)~~";
