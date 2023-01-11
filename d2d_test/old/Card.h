#pragma once
#include <memory>
#include <vector>
#include <regex>
/*
#include <SFML/Graphics.hpp>

#include "Border_generator.h"

struct Part
	{
	virtual sf::Vector2f draw(sf::RenderTarget& rt) = 0;
	};


struct PosSize
	{
	sf::Vector2f position;
	sf::Vector2f size;
	};

class Border : public Part
	{
	public:
		Border(Border_generator& generator, PosSize possize) : generator(&generator) {}

	private:
		Border_generator* generator;
	};

class Boxed : public Part
	{
	public:
		Boxed(Border_generator& generator, std::unique_ptr<Part> part) : generator{&generator}, part{std::move(part)} {}

		std::unique_ptr<Part> part;
		Border_generator* generator;

		virtual sf::Vector2f draw(sf::RenderTarget& rt) override 
			{
			auto inner_size = part->draw(rt);
			generator->generate(inner_size);
			}

	private:
	};

enum class Grow_direction { right, up, left, down };

template <Grow_direction grow>
class Textbox_grow_dir : public sf::Drawable
	{
	public:
		Textbox_grow_dir(const std::string& string, sf::Font& font, sf::Vector2f position, sf::Vector2f max_size, Symbols_db& symbols_db) : string{string}, font{&font}, position{position}, max_size{max_size}, symbols_db{&symbols_db} {}


		virtual sf::Vector2f draw(sf::RenderTarget& rt) override
			{
			sf::Text text;
			std::vector<sf::Sprite*> symbols;

			std::string s = string;
			std::regex regex{"\\[a-zA-Z_]+"};
			std::smatch match;
			while (std::regex_search(s, match, regex))
				{
				for (std::ssub_match element : match)
					{
					symbols.push_back(&symbols_db[element.str()]);
					}

				s = match.suffix().str();
				}

			//replace "\\[a-zA-Z_]+" with "  \  " to count coordinates for symbols
			s = std::regex_replace(string, regex, "  \\  ");
			}

	private:
		std::string string;
		sf::Font* font;
		sf::Vector2f position;
		sf::Vector2f max_size;
		Symbols_db* symbols_db;
	};

class Textbox_grow_font : public sf::Drawable
	{
	public:
		Textbox_grow_font(const std::string& string, sf::Font& font, sf::Vector2f position, sf::Vector2f size) {}
	private:
	};


class Card_generator
	{
	public:
		std::vector<std::unique_ptr<Part>> parts;

	private:
	};

*/