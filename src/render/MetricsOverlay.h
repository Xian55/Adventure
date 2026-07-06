#pragma once

namespace adventure
{
	class Metrics;

	// Draws the metrics HUD at native window resolution. Single responsibility: presentation only.
	// Reads a Metrics snapshot; never mutates it. Call inside BeginDrawing()/EndDrawing(), after
	// the upscaled scene blit, so the text stays crisp.
	void drawMetricsOverlay(const Metrics& metrics, bool visible);
} // namespace adventure
