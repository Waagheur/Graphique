#version 130
precision mediump float; //Medium precision for float. highp and smallp can also be used

uniform sampler2D uTexture;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float alpha;

uniform vec3 lightPos;
uniform vec3 lightColor;

uniform vec3 cameraPos;

varying vec2 varyUV;
varying vec3 varyNormal;
varying vec4 varyWorldPosition;

//We still use varying because OpenGLES 2.0 (OpenGL Embedded System, for example for smartphones) does not accept "in" and "out"

void main()
{
	vec4 color = texture2D(uTexture, varyUV);
	vec3 normal = normalize(varyNormal);
	vec3 lightDir = normalize(lightPos - varyWorldPosition.xyz);

	vec3 V = normalize(cameraPos - varyWorldPosition.xyz);
	vec3 R = reflect(-lightDir, normal);

	vec3 ambient = ka * color.rgb * lightColor;
	vec3 diffuse = kd * max(0.0, dot(normal, lightDir)) * color.rgb * lightColor;
	vec3 specular = ks * pow(max(0.0, dot(R, V)), alpha) * lightColor;

    gl_FragColor = vec4(ambient + diffuse + specular, 1.0);
}
