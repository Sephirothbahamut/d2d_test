#pragma once

#include <string>
#include <sstream>
#include <unordered_map>

#include <SFML/Graphics.hpp>
#include "mysf.h"

class Symbols_db
	{
	public:
		Symbols_db(const std::string texture_fname, std::vector<std::pair<std::string, sf::IntRect>> rects) : texture{texture_fname} 
			{
			for (const auto& element : rects) { map[element.first] = {texture, element.second}; }
			}

		auto find(const std::string& string) const { return map.find(string); }
		auto end() const { return map.end(); }

	private:
		mysf::Texture texture;
		std::unordered_map<std::string, sf::Sprite> map;
	};

class Text_ext : public sf::Drawable
	{
	using symbol_at_t = std::pair<std::string, size_t>;
	public:
		Text_ext(const sf::Font& font, const Symbols_db& symbols_db, const std::string& string = "", unsigned size = 30u) : text(string, font, size), symbols_db{&symbols_db}, source_string{string}
			{ update(); }
			
		void set_size(sf::Vector2f new_size) { size = new_size; update(); }
		void set_size(float x, float y) { size = {x, y}; update(); }
		void set_string(const std::string string) { source_string = string; update(); }
		void set_position(sf::Vector2f new_position) { text.setPosition(new_position); update(); }
		void set_position(float x, float y) { size = {x, y}; update(); }

	private:
		sf::Text text;
		std::string source_string{};
		sf::Vector2f size{};
		std::vector<sf::Sprite> sprites;

		const Symbols_db* symbols_db;

		float update()
			{
			auto ret = prepare_string_for_symbols();

			text.setString(ret.first);

			size_t last_symbol_index = 0;
			for (size_t i = 0; i < text.getString().getSize(); i++)
				{
				if (text.findCharacterPos(i).x > size.x)
					{
					auto str = text.getString();
					str.insert(i, '\n');
					text.setString(str);

					while (ret.second[last_symbol_index].second < i) { last_symbol_index++; }
					for (size_t i = last_symbol_index; i < ret.second.size(); i++) { ret.second[last_symbol_index].second++; }
					}
				}
			sprites.clear();
			sprites.reserve(ret.second.size());
			for (const auto& element : ret.second)
				{
				auto it = symbols_db->find(element.first);
				if (it == symbols_db->end()) { throw std::runtime_error{"Couldn't find symbol " + element.first}; }
				sf::Sprite s = it->second;
				s.setPosition(text.findCharacterPos(element.second));
				sprites.push_back(s);
				}

			return text.getGlobalBounds().height;
			}

		std::pair<std::string, std::vector<symbol_at_t>> prepare_string_for_symbols()
			{
			std::vector<symbol_at_t> symbols;
			std::stringstream result;

			size_t i = 0, r = 0;
			while (i < source_string.size())
				{
				if (source_string[i] != '\\') { result << source_string[i]; i++, r++; }
				else
					{
					std::string token;
					for (i++; i < source_string.size() && source_string[i] != ' '; i++) { token += source_string[i]; }
					symbols.emplace_back(token, r);
					result << "#";
					r++;
					}
				}
			return {result.str(), symbols};
			}

		virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
			{
			target.draw(text, states);
			for (const auto& s : sprites) { target.draw(s, states); }
			}
	};