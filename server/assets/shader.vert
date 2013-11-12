#version 110

varying vec3 normal;
varying vec4 worldPosition;
varying vec4 worldN;
varying vec3 viewDir;
varying vec3 position;
	
uniform vec4 camLoc;

void main()
{
	normal = gl_NormalMatrix * gl_Normal;
	vec4 v = vec4( gl_Vertex );
	vec3 position = v.xyz;

	gl_Position = ftransform();
	worldPosition = gl_ModelViewMatrix * gl_Vertex;
	worldN = normalize( vec4( gl_NormalMatrix * gl_Normal, 0 ) );
	vec4 viewDir4 = normalize( camLoc - worldPosition );
	viewDir = vec3( viewDir4.xyz );
	gl_TexCoord[0] = gl_MultiTexCoord0;
}
