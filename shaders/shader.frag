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

uint vecToUint(vec4 val){
	uint a = uint(val.a + 0.5);
	uint r = uint(val.r * 31 + 0.5);
	uint g = uint(val.g * 31 + 0.5);
	uint b = uint(val.b * 31 + 0.5);
	return (a << 15) | (b << 10) | (g << 5) | r;
}

void main(){
	if(renderMode == 0){
		// 4-bit clut
		uint texel = vecToUint(texture(frameContent, textPos));
		// get the right clut pos
		texel = (texel >> ((int(textPos.x * 4) & 3) * 4)) & 0xF;
		outColor = texelFetch(frameContent, ivec2(clutLoc.x + texel, clutLoc.y),0);
		// hopefully this inequality on floating values does not lead to any problem
		if(outColor == vec4(0.0,0.0,0.0,0.0)){
			// transparent
			discard;
		}
	} else if(renderMode == 1){
		// 8-bit clut
		uint texel = vecToUint(texture(frameContent, textPos));
		// get the right clut pos
		texel = (texel >> ((int(textPos.x * 2) & 1) * 8)) & 0xFF;
		outColor = texelFetch(frameContent, ivec2(clutLoc.x + texel, clutLoc.y),0);
		// hopefully this inequality on floating values does not lead to any problem
		if(outColor == vec4(0.0,0.0,0.0,0.0)){
			// transparent
			discard;
		}
	} else if(renderMode == 2){
		// direct 15-bit
		outColor = texture(frameContent, textPos);
		// hopefully this inequality on floating values does not lead to any problem
		if(outColor == vec4(0.0,0.0,0.0,0.0)){
			// transparent
			discard;
		}
	} else {
		// render color
		outColor = vec4(fragColor,0.0);
	}
}