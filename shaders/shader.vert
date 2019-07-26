#version 450

layout (location = 0) in ivec2 inPosition;
layout (location = 1) in uvec3 inColor;
layout (location = 2) in uvec2 textCoord;

/*
CLUT-ID
Specifies the location of the CLUT data. Data is 16bits.

F-6      Y coordinate 0-511
5-0      X coordinate X/16
*/
layout (location = 3) in uint clutId;

/*
----------------------------------------------------
|0f 0e 0d|0c|0b|0a  |09 |08 07|06 05|04|03 02 01 00|
| ?  ?  ?|me|md|dfe |dtd|tp   |abr  |ty|tx         |
----------------------------------------------------
                                                     
tx:        0      0        Texture page X = tx*64
           1     64
           2    128
           3    196
           4   ...
ty         0      0        Texture page Y
           1    256
abr      %00  0.5xB+0.5 xF Semi transparent state
         %01  1.0xB+1.0 xF
         %10  1.0xB-1.0 xF
         %11  1.0xB+0.25xF
tp       %00  4bit CLUT    Texture page color mode
         %01  8bit CLUT
         %10  15bit
dtd        0  Ditter off
           1  Ditter on
dfe        0  Draw to display area prohibited
           1  Draw to display area allowed
md         0  off
           1  on   Apply mask bit to drawn pixels.
me         0  off
           1  on   No drawing to pixels with set mask bit.
*/
layout (location = 4) in uint texturePage;

layout (location = 0) out vec3 fragColor;
layout (location = 1) out uint renderMode;
layout (location = 2) out uvec2 textLoc;
layout (location = 3) out vec2 textPos;
layout (location = 4) out uvec2 clutLoc;

void main(){
	float xpos = (float(inPosition.x) / 512) - 1.0;
	float ypos = (float(inPosition.y) / 256) - 1.0;
	gl_Position = vec4(xpos,ypos,0.0,1.0);
	fragColor = vec3(
		float(inColor.r) / 255,
		float(inColor.g) / 255,
		float(inColor.b) / 255);
	textLoc = uvec2(
		(texturePage & 0xF) << 6,
		(texturePage & 0x10) << 8
	);
	textPos = vec2(0.0,0.0);
	clutLoc = uvec2(
		(clutId & 0x3F) << 4,
		(clutId & 0xFFC0) >> 6
	);
	if(texturePage == (1 << 16)){
		// render color
		renderMode = 3;
	} else {
		renderMode = (texturePage >> 7) & 3;
		if(renderMode == 0){
			// 4-bit rendering -> 4 pixel per texel
			textPos = vec2(float(textLoc.x) + (float(textCoord.x) / 4),
				float(textLoc.y + textCoord.y));
		} else if(renderMode == 1){
			// 8-bit rendering -> 2 pixel per texel
			textPos = vec2(float(textLoc.x) + float(textCoord.x) / 2,
				float(textLoc.y + textCoord.y));
		} else if(renderMode == 2){
			// direct 15-bit
			textPos = vec2(float(textLoc.x + textCoord.x), float(textLoc.y + textCoord.y));
		}
	}
}