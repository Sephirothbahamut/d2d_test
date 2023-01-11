#pragma once

#include <string>
#include <stdexcept>
#include <iostream>

#include "mysf.h"

class Repeated : public sf::Sprite
	{
	public:
		mysf::RepeatedTexture texture;

		Repeated(const std::string& fname, sf::IntRect rect, sf::Vector2f origin) : texture{fname, rect}
			{
			setTexture(texture);
			setOrigin(origin);
			setTextureRect({{}, {rect.width, rect.height}});
			}
	};

class Border_generator
	{

	public:
		Border_generator(const std::string& fname, 
			mysf::Rectangle_origin rr, mysf::Rectangle_origin ru, mysf::Rectangle_origin up, mysf::Rectangle_origin lu,
			mysf::Rectangle_origin ll, mysf::Rectangle_origin ld, mysf::Rectangle_origin dw, mysf::Rectangle_origin rd) :
			texture{fname}, 
			rr{fname, rr.rect, rr.origin}, ru{texture, ru.rect, ru.origin}, up{fname, up.rect, up.origin}, lu{texture, lu.rect, lu.origin},
			ll{fname, ll.rect, ll.origin}, ld{texture, ld.rect, ld.origin}, dw{fname, dw.rect, dw.origin}, rd{texture, rd.rect, rd.origin}
			{
			this->ru.setTexture(texture);
			this->lu.setTexture(texture);
			this->ld.setTexture(texture);
			this->rd.setTexture(texture);
			}

		mysf::Texture generate(sf::Vector2f size)
			{
			if ((size.x < ru.getTextureRect().width + lu.getTextureRect().width) ||
				(size.y < ru.getTextureRect().height + rd.getTextureRect().height))
				{
				throw std::runtime_error{"Trying to create a border smaller than its minimum corners size"};
				}
			
			mysf::RenderTexture rt(static_cast<unsigned>(size.x), static_cast<unsigned>(size.y));
			rt.clear(sf::Color::Transparent);

			lu.setPosition(0, 0);
			ru.setPosition(size.x, 0);
			ld.setPosition(0, size.y);
			rd.setPosition(size.x, size.y);

			rt.draw(lu); rt.draw(ru); rt.draw(ld); rt.draw(rd);

			{//Top border
			up.setTextureRect({{0, 0}, {static_cast<int>(size.x - lu.getTextureRect().width - ru.getTextureRect().width), static_cast<int>(up.getTextureRect().height)}});
			up.setPosition(lu.getTextureRect().width, 0);
			
			rt.draw(up);
			}
			{//Bottom border
			dw.setTextureRect({{0, 0}, {static_cast<int>(size.x - ld.getTextureRect().width - rd.getTextureRect().width), static_cast<int>(dw.getTextureRect().height)}});
			dw.setPosition(lu.getTextureRect().width, size.y);

			rt.draw(dw);
			}
			{//Left border
			ll.setTextureRect({{0, 0}, {static_cast<int>(ll.getTextureRect().width), static_cast<int>(size.y - lu.getTextureRect().height - ld.getTextureRect().height)}});
			ll.setPosition(0, lu.getTextureRect().height);

			rt.draw(ll);
			}
			{//Right border
			rr.setTextureRect({{0, 0}, {static_cast<int>(rr.getTextureRect().width), static_cast<int>(size.y - ru.getTextureRect().height - rd.getTextureRect().height)}});
			rr.setPosition(size.x, ru.getTextureRect().height);

			rt.draw(rr);
			}

			rt.display();
			return rt.getTexture();
			}

	private:
		mysf::Texture texture;
		mysf::Sprite ru, lu, ld, rd;
		Repeated rr, up, ll, dw;
	};

