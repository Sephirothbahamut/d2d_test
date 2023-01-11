
#include <iostream>
#include <string>
#include <stdexcept>
#include <regex>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Border_generator.h"
#include "Text_ext.h"

class Interface
	{
	public:
		Interface() : window(sf::VideoMode{1400, 1200}, "Zeus Shrine Editor", 7U)
			{
			loop();
			}

	private:
		sf::RenderWindow window;

		void loop()
			{
			draw();
			while (window.isOpen()) { step(); }
			}

		void step()
			{
			event();
			draw();
			}

		void event()
			{
			sf::Event event;
			window.waitEvent(event);
			switch (event.type)
				{
				case sf::Event::Closed: window.close(); break;
				case sf::Event::MouseMoved: mouse_move(event.mouseMove); break;
				case sf::Event::MouseWheelScrolled: mouse_wheel(event.mouseWheelScroll); break;
				case sf::Event::KeyPressed: keyboard(event.key); break;
				}
			}

		void draw()
			{
			window.clear(sf::Color::Cyan);

			Border_generator thick_border{"./Data/Textures/RPG_GUI_v1.png",
				{.rect{986, 227, 12, 55}, .origin{12, 0}},//rr
				{.rect{978, 193, 20, 20}, .origin{20, 0}},//ru
				{.rect{893, 193, 73, 12}, .origin{0, 0}},//up
				{.rect{860, 193, 20, 20}, .origin{0, 0}},//lu
				{.rect{860, 227, 12, 55}, .origin{0, 0}},//ll
				{.rect{860, 294, 20, 20}, .origin{0, 20}},//ld
				{.rect{893, 302, 73, 12}, .origin{0, 12}},//dw
				{.rect{978, 294, 20, 20}, .origin{20, 20}},//rd
				};

			Symbols_db symbols_db{"./Data/Textures/IconSet1.png",
					{
							{"cut",       {384, 128, 32, 32}},
							{"crush",     {416, 128, 32, 32}},
							{"pierce",    {448, 128, 32, 32}},

							{"fire",      {  0, 128, 32, 32}},
							{"ice",       { 32, 128, 32, 32}},
							{"lightning", { 64, 128, 32, 32}},
							{"water",     { 96, 128, 32, 32}},
							{"earth",     {128, 128, 32, 32}},
							{"wind",      {160, 128, 32, 32}},
							{"light",     {192, 128, 32, 32}},
							{"darkness",  {224, 128, 32, 32}},
					}
				};

			auto ret = thick_border.generate({700, 1200});
			sf::Sprite tmp{ret};
			mysf::Font font{"./Data/Fonts/arial.ttf"};
			Text_ext text(font, symbols_db, "Target distance 1, radius 1\nDeals 5 \\crush \\fire damage.", 18u);
			text.set_size(600, 0);


			window.draw(tmp);
			window.draw(text);

			window.display();
			}

		void mouse_wheel(sf::Event::MouseWheelScrollEvent e)
			{
			auto view = window.getView();
			if (e.delta > 0) { view.zoom(.9); }
			else { view.zoom(1.1); }

			view.setCenter(view.getSize().x / 2, view.getSize().y / 2);
			window.setView(view);
			}


		void mouse_move(sf::Event::MouseMoveEvent e)
			{
			auto tmp = window.mapPixelToCoords({e.x, e.y}, window.getView());

			size_t x = tmp.x / 64;
			size_t y = tmp.y / 64;


			}

		void keyboard(sf::Event::KeyEvent e)
			{
			}
	};

#include <string>
#include <sstream>

int main()
	{




	try
		{
		Interface i;
		}
	catch (std::runtime_error e) { std::cout << e.what() << std::endl; }
	}