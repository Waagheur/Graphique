#version 130
precision mediump float;

attribute vec3 vPosition; //Depending who compiles, these variables are not "attribute" but "in". In this version (130) both are accepted. in should be used later
attribute vec3 vNormal;
attribute vec2 vUV;

uniform mat4 uMVP;
uniform mat4 uModel;
uniform mat3 uInvModel3x3;

varying vec2 varyUV;
varying vec3 varyNormal;
varying vec4 varyWorldPosition;

//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

void main()
{
	varyUV = vUV;
	
	gl_Position = uMVP * vec4(vPosition, 1.0);
	varyNormal = transpose(uInvModel3x3) * vNormal;
	
	varyWorldPosition = uModel * vec4(vPosition, 1.0);
	varyWorldPosition = varyWorldPosition / varyWorldPosition.w; // Normalisation
}
