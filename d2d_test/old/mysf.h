#pragma once

#include <SFML/Graphics.hpp>

namespace mysf
    {
    struct Texture : sf::Texture
        {
        Texture(const std::string& fname) { if (!loadFromFile(fname)) { throw std::runtime_error{"Failed to load texture " + fname}; } }
        Texture(const sf::Texture& a) : sf::Texture{a}{}
        };


    struct Image : sf::Image
        {
        Image(const std::string& fname) { if (!loadFromFile(fname)) { throw std::runtime_error{"Failed to load image " + fname}; } }
        Image(const sf::Image& a) : sf::Image{a}{}
        };

    struct RepeatedTexture : sf::Texture
        {
        RepeatedTexture(const std::string& fname, sf::IntRect rect) 
            {
            mysf::Image source{fname};
            if(!loadFromImage(source, rect)) { throw std::runtime_error{"Failed to load texture " + fname}; }
            setRepeated(true);
            }
        };

    struct Font : sf::Font
        {
        Font(const std::string& fname) { if (!loadFromFile(fname)) { throw std::runtime_error{"Failed to load font " + fname}; } }
        Font(const sf::Font& a) : sf::Font{a}{};
        };

    struct Sprite : sf::Sprite
        {
        Sprite(sf::Texture tex, sf::IntRect rect, sf::Vector2f origin) : sf::Sprite{tex, rect} { setOrigin(origin); }
        Sprite(const sf::Sprite& a) : sf::Sprite{a}{}
        };

    struct RenderTexture : sf::RenderTexture
        {
        RenderTexture(sf::Vector2u size) { if (!create(size.x, size.y)) { throw std::runtime_error{"Failed to create render texture"}; } }
        RenderTexture(unsigned x, unsigned y) { if (!create(x, y)) { throw std::runtime_error{"Failed to create render texture"}; } }
        };

    struct Rectangle_origin
        {
        sf::IntRect rect;
        sf::Vector2f origin;
        };
    }