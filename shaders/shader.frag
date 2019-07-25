#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) out vec4 outColor;

layout (location = 0) in vec3 fragColor;
/*
0 : 4-bit CLUT
1 : 8-bit CLUT
2 : 15-bit direct render
3 : color
*/
layout (location = 1) flat in uint renderMode;
layout (location = 2) flat in uvec2 textLoc;
layout (location = 3) in vec2 textPos;
layout (location = 4) flat in uvec2 clutLoc;
layout (binding = 0) uniform sampler2D frameContent;

void main(){
	if(renderMode == 0){
		// 4-bit clut
		outColor = vec4(fragColor,1.0);
	} else if(renderMode == 1){
		// 8-bit clut
		outColor = vec4(fragColor,1.0);
	} else if(renderMode == 2){
		// direct 15-bit
		outColor = texture(frameContent, textPos);
	} else {
		// render color
		outColor = vec4(fragColor,1.0);
	}
}