#version 330 core

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
