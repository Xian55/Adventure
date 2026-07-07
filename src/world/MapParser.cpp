#include "world/MapParser.h"

#include <cstdio>
#include <cstdlib>

namespace adventure::world
{
	std::string Entity::str(const std::string& k, const std::string& def) const
	{
		auto it = keys.find(k);
		return it != keys.end() ? it->second : def;
	}

	float Entity::number(const std::string& k, float def) const
	{
		auto it = keys.find(k);
		return it != keys.end() ? (float)std::atof(it->second.c_str()) : def;
	}

	Vector3 Entity::vec3(const std::string& k, Vector3 def) const
	{
		auto it = keys.find(k);
		if (it == keys.end())
			return def;
		Vector3 v = def;
		std::sscanf(it->second.c_str(), "%f %f %f", &v.x, &v.y, &v.z);
		return v;
	}

	const Entity* MapData::first(const std::string& classname) const
	{
		for (const auto& e : entities)
			if (e.classname == classname)
				return &e;
		return nullptr;
	}

	namespace
	{
		enum class Tok
		{
			LBrace,
			RBrace,
			LParen,
			RParen,
			LBracket,
			RBracket,
			String,
			Word,
			End
		};

		struct Token
		{
			Tok kind;
			std::string text;
		};

		// Split into tokens: braces/parens/brackets are single chars, "..." is a string, comments (//)
		// run to end of line, everything else is a whitespace-delimited word (number or texture name).
		std::vector<Token> tokenize(const std::string& s)
		{
			std::vector<Token> out;
			std::size_t i = 0, n = s.size();
			while (i < n)
			{
				char c = s[i];
				if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
				{
					++i;
					continue;
				}
				if (c == '/' && i + 1 < n && s[i + 1] == '/')
				{
					while (i < n && s[i] != '\n')
						++i;
					continue;
				}
				switch (c)
				{
				case '{':
					out.push_back({Tok::LBrace, "{"});
					++i;
					continue;
				case '}':
					out.push_back({Tok::RBrace, "}"});
					++i;
					continue;
				case '(':
					out.push_back({Tok::LParen, "("});
					++i;
					continue;
				case ')':
					out.push_back({Tok::RParen, ")"});
					++i;
					continue;
				case '[':
					out.push_back({Tok::LBracket, "["});
					++i;
					continue;
				case ']':
					out.push_back({Tok::RBracket, "]"});
					++i;
					continue;
				}
				if (c == '"')
				{
					++i;
					std::string str;
					while (i < n && s[i] != '"')
						str.push_back(s[i++]);
					if (i < n)
						++i; // closing quote
					out.push_back({Tok::String, str});
					continue;
				}
				std::string word;
				while (i < n)
				{
					char w = s[i];
					if (w == ' ' || w == '\t' || w == '\r' || w == '\n' || w == '{' || w == '}' ||
					    w == '(' || w == ')' || w == '[' || w == ']')
						break;
					word.push_back(w);
					++i;
				}
				out.push_back({Tok::Word, word});
			}
			out.push_back({Tok::End, ""});
			return out;
		}

		// Standard Quake faces give only `offx offy rot sx sy`; synthesize axis-aligned texture axes from
		// the face normal using Quake's baseaxis table (so Valve-220 and Standard maps share one path).
		void standardAxes(Face& f, float offx, float offy)
		{
			// clang-format off
			static const float base[18][3] = {
			    {0, 0, 1},  {1, 0, 0}, {0, -1, 0},   // floor
			    {0, 0, -1}, {1, 0, 0}, {0, -1, 0},   // ceiling
			    {1, 0, 0},  {0, 1, 0}, {0, 0, -1},   // west
			    {-1, 0, 0}, {0, 1, 0}, {0, 0, -1},   // east
			    {0, 1, 0},  {1, 0, 0}, {0, 0, -1},   // south
			    {0, -1, 0}, {1, 0, 0}, {0, 0, -1},   // north
			};
			// clang-format on
			const float ax = f.p[1].x - f.p[0].x, ay = f.p[1].y - f.p[0].y, az = f.p[1].z - f.p[0].z;
			const float bx = f.p[2].x - f.p[0].x, by = f.p[2].y - f.p[0].y, bz = f.p[2].z - f.p[0].z;
			const float nx = ay * bz - az * by, ny = az * bx - ax * bz, nz = ax * by - ay * bx;

			int best = 0;
			float bestDot = -1e30f;
			for (int i = 0; i < 6; ++i)
			{
				const float d = nx * base[i * 3][0] + ny * base[i * 3][1] + nz * base[i * 3][2];
				if (d > bestDot)
				{
					bestDot = d;
					best = i;
				}
			}
			const float* u = base[best * 3 + 1];
			const float* v = base[best * 3 + 2];
			f.uAxis = Vector4{u[0], u[1], u[2], offx};
			f.vAxis = Vector4{v[0], v[1], v[2], offy};
		}

		struct Parser
		{
			const std::vector<Token>& t;
			std::size_t i = 0;
			std::string error;

			explicit Parser(const std::vector<Token>& toks)
			    : t(toks) {}

			const Token& peek() const { return t[i]; }
			const Token& next() { return t[i++]; }
			bool accept(Tok k)
			{
				if (t[i].kind == k)
				{
					++i;
					return true;
				}
				return false;
			}
			bool fail(const std::string& msg)
			{
				if (error.empty())
					error = msg;
				return false;
			}

			bool number(float& out)
			{
				if (peek().kind != Tok::Word)
					return fail("expected number");
				out = (float)std::atof(next().text.c_str());
				return true;
			}

			bool vec3In(Vector3& v)
			{
				if (!accept(Tok::LParen))
					return fail("expected '('");
				if (!number(v.x) || !number(v.y) || !number(v.z))
					return false;
				if (!accept(Tok::RParen))
					return fail("expected ')'");
				return true;
			}

			bool vec4In(Vector4& v)
			{
				if (!accept(Tok::LBracket))
					return fail("expected '['");
				if (!number(v.x) || !number(v.y) || !number(v.z) || !number(v.w))
					return false;
				if (!accept(Tok::RBracket))
					return fail("expected ']'");
				return true;
			}

			bool face(Face& f)
			{
				if (!vec3In(f.p[0]) || !vec3In(f.p[1]) || !vec3In(f.p[2]))
					return false;
				if (peek().kind != Tok::Word)
					return fail("expected texture name");
				f.texture = next().text;

				if (peek().kind == Tok::LBracket) // Valve 220: [ ux uy uz off ] [ vx vy vz off ] rot sx sy
				{
					if (!vec4In(f.uAxis) || !vec4In(f.vAxis))
						return false;
					if (!number(f.rotation) || !number(f.scaleX) || !number(f.scaleY))
						return false;
				}
				else // Standard Quake: offx offy rot sx sy
				{
					float offx = 0.0f, offy = 0.0f;
					if (!number(offx) || !number(offy) || !number(f.rotation) || !number(f.scaleX) ||
					    !number(f.scaleY))
						return false;
					standardAxes(f, offx, offy);
				}
				return true;
			}

			bool brush(Brush& b)
			{
				// caller consumed the opening '{'
				while (peek().kind != Tok::RBrace && peek().kind != Tok::End)
				{
					Face f;
					if (!face(f))
						return false;
					b.faces.push_back(std::move(f));
				}
				if (!accept(Tok::RBrace))
					return fail("unterminated brush");
				if (b.faces.size() < 4)
					return fail("brush has fewer than 4 faces");
				return true;
			}

			bool entity(Entity& e)
			{
				// caller consumed the opening '{'
				while (peek().kind != Tok::RBrace && peek().kind != Tok::End)
				{
					if (peek().kind == Tok::String)
					{
						std::string key = next().text;
						if (peek().kind != Tok::String)
							return fail("key without value");
						std::string val = next().text;
						if (key == "classname")
							e.classname = val;
						e.keys[key] = val;
					}
					else if (accept(Tok::LBrace))
					{
						Brush b;
						if (!brush(b))
							return false;
						e.brushes.push_back(std::move(b));
					}
					else
					{
						return fail("unexpected token in entity");
					}
				}
				if (!accept(Tok::RBrace))
					return fail("unterminated entity");
				return true;
			}

			bool parse(MapData& map)
			{
				while (peek().kind != Tok::End)
				{
					if (!accept(Tok::LBrace))
						return fail("expected entity '{'");
					Entity e;
					if (!entity(e))
						return false;
					map.entities.push_back(std::move(e));
				}
				return true;
			}
		};
	} // namespace

	MapParseResult parseMap(const std::string& text)
	{
		MapParseResult r;
		std::vector<Token> toks = tokenize(text); // must outlive the Parser (it holds a reference)
		Parser p(toks);
		r.ok = p.parse(r.data);
		if (!r.ok)
		{
			r.error = p.error.empty() ? "parse failed" : p.error;
			r.data.entities.clear();
		}
		return r;
	}
} // namespace adventure::world
