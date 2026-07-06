#include "render/MetricsOverlay.h"
#include "core/Metrics.h"

#include "raylib.h"

namespace adventure
{
	namespace
	{
		float mib(std::size_t bytes)
		{
			return (float)bytes / (1024.0f * 1024.0f);
		}
	} // namespace

	void drawMetricsOverlay(const Metrics& m, bool visible)
	{
		if (!visible)
			return;

		const int x = 10, y = 10;
		const int line = 22, font = 20;
		const int rows = 3 + (int)m.sections().size();
		const int w = 360;
		const int h = line * rows + 12;

		DrawRectangle(x - 4, y - 4, w, h, Color{0, 0, 0, 160});
		DrawRectangleLines(x - 4, y - 4, w, h, Color{80, 90, 110, 200});

		int cy = y;
		const Color head = Color{120, 230, 140, 255};
		const Color val = RAYWHITE;
		const Color warn = Color{240, 180, 90, 255};

		DrawText(TextFormat("FPS %3.0f   %5.2f ms  (max %5.2f)", m.fps(), m.frameMs(), m.frameMsMax()),
		         x,
		         cy,
		         font,
		         m.frameMsMax() > 20.0f ? warn : head);
		cy += line;

		DrawText(TextFormat("CPU %4.1f%%   RSS %6.1f MB   Lua %5.1f KB",
		                    m.cpuPercent(),
		                    mib(m.rssBytes()),
		                    (float)m.luaBytes() / 1024.0f),
		         x,
		         cy,
		         font,
		         val);
		cy += line;

		DrawText("sections (ms):", x, cy, font, Color{150, 160, 180, 255});
		cy += line;

		for (const auto& s : m.sections())
		{
			DrawText(TextFormat("  %-10s %5.2f", s.name ? s.name : "?", s.ms),
			         x,
			         cy,
			         font,
			         s.ms > 8.0f ? warn : val);
			cy += line;
		}
	}
} // namespace adventure
