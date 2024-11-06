#version 410 core
 
in vec3 fNormal;
in vec4 fPosEye;
in vec3 fPos;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
 
out vec4 fColor;
 
//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
 
//positional lights data
uniform vec3 lantern;
uniform int LightEnabler;
 
//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
 
vec3 ambient;
float ambientStrength = 0.2f;
float sunAmbientStrength = 0.1f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;
float shininess = 32.0f;
 
vec3 ambientalLight;
vec3 diffuseLight;
vec3 specularLight;

void directionalLight() {
 
	//compute ambient light
	ambient = ambientStrength * lightColor;
 
	//transform normal
	vec3 normalEye = normalize(fNormal);
 
	//compute light direction
	vec3 lightDirN = normalize(lightDir);
 
	//compute view direction
	vec3 cameraPosEye = vec3(0.0f);	//in eye coordinates, the viewer is situated at the origin
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
 
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
 
	//compute specular light
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDirN, reflection), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
}

 
void pointLights(vec3 lightPos, vec3 lightColor, float ambientStrength){
	
	
	vec3 cameraPosEye = vec3 (0.0f, 0.0f, 0.0f);
	
	ambient = ambientStrength * lightColor;
 
	//compute diffuse light
	vec3 norm = normalize(fNormal);
	vec3 lightDir = normalize(lightPos - fPos);
	diffuse = max(dot(norm, lightDir), 0.0f) * lightColor;
 
	//compute specular light
	vec3 viewDir = normalize(cameraPosEye - fPos.xyz);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
	specular = specularStrength * spec * lightColor;
 
	// Attenuation
	float distance = length(fPos.xyz - lightPos);
	float attenuation = 10.0f / (0.05f + 0.5f * distance + 0.05f * (distance * distance));
 
	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;
}


void getPointLights(){

	pointLights(lantern, lightColor, 2.2f);
	vec3 lanternAmbientalLight = ambient * texture(diffuseTexture, fTexCoords).rgb;
	vec3 lanternDiffuseLight = diffuse * texture(diffuseTexture, fTexCoords).rgb;
	vec3 lanternSpecularLight = specular * texture(specularTexture, fTexCoords).rgb;


	ambientalLight = lanternAmbientalLight;
	diffuseLight = lanternDiffuseLight;
	specularLight = lanternSpecularLight;
}


float fogGenerator() {
	float fogDensity = 0.005f;
	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
	return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {
	vec3 sunAmbient, sunDiffuse, sunSpecular;
 
	if(LightEnabler == 1){
		directionalLight();
		sunAmbient = 0.9 * ambient * texture(diffuseTexture, fTexCoords).rgb;
		sunDiffuse = 0.8 * diffuse * texture(diffuseTexture, fTexCoords).rgb;
		sunSpecular = specular * texture(specularTexture, fTexCoords).rgb;
	}
	
	getPointLights();
	
	ambient = sunAmbient + ambientalLight;
	diffuse = sunDiffuse + diffuseLight;
	specular = sunSpecular + specularLight;
	
	float alpha = texture(diffuseTexture, fTexCoords).a;
	vec4 color = vec4(min(ambient + diffuse + specular, 1.0f), 1.0f);
	
	float fogFactor = fogGenerator();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
	
	fColor = mix(fogColor, color, fogFactor);
}