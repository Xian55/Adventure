#pragma once

// Inline GLSL. Post-process fragment shader applied on the low-res image (chunky dither).
// GL330 desktop path for now; a GLES100 path (craft_raylib's macro dual-path) can be added
// alongside when a web/mobile target is wanted.
namespace adventure::shaders
{
	inline const char* kPostFS = R"GLSL(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec2 uLowRes;   // low-res target dimensions (for chunky per-texel dither)
uniform float uLevels;  // palette quantization levels per channel

// 4x4 ordered Bayer matrix, normalized 0..1.
const float bayer[16] = float[16](
	 0.0/16.0,  8.0/16.0,  2.0/16.0, 10.0/16.0,
	12.0/16.0,  4.0/16.0, 14.0/16.0,  6.0/16.0,
	 3.0/16.0, 11.0/16.0,  1.0/16.0,  9.0/16.0,
	15.0/16.0,  7.0/16.0, 13.0/16.0,  5.0/16.0);

void main()
{
	vec4 c = texture(texture0, fragTexCoord) * colDiffuse * fragColor;

	// Index the dither by low-res texel so the pattern stays coarse after upscale.
	ivec2 p = ivec2(floor(fragTexCoord * uLowRes));
	int ix = p.x - (p.x / 4) * 4;
	int iy = p.y - (p.y / 4) * 4;
	float d = (bayer[iy * 4 + ix] - 0.5) / uLevels;

	vec3 col = floor((c.rgb + d) * uLevels + 0.5) / uLevels;
	finalColor = vec4(clamp(col, 0.0, 1.0), c.a);
}
)GLSL";

	// World geometry: textured, baked vertex light, exponential distance fog.
	inline const char* kWorldVS = R"GLSL(
#version 330
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
uniform mat4 mvp;
uniform mat4 matModel;
uniform vec3 uCameraPos;
out vec2 fragTexCoord;
out vec4 fragColor;
out float fragDist;
void main()
{
	fragTexCoord = vertexTexCoord;
	fragColor = vertexColor;
	vec3 world = (matModel * vec4(vertexPosition, 1.0)).xyz;
	fragDist = length(world - uCameraPos);
	gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)GLSL";

	inline const char* kWorldFS = R"GLSL(
#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
in float fragDist;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform vec3 uFogColor;
uniform float uFogDensity;
out vec4 finalColor;
void main()
{
	vec4 c = texture(texture0, fragTexCoord) * fragColor * colDiffuse;
	if (c.a < 0.5)
		discard;
	float f = clamp(1.0 - exp2(-uFogDensity * fragDist), 0.0, 1.0);
	finalColor = vec4(mix(c.rgb, uFogColor, f), 1.0);
}
)GLSL";
} // namespace adventure::shaders
