/**
 * \file
 * \brief Source file for drawing mandelbrot set
*/

#include <SFML/Graphics.hpp>
#include "configs.hpp"
#include "draw.hpp"




int draw_mandelbrot(void) {
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Mandelbrot3000");


    sf::Font font;
    if (!font.loadFromFile(FONT_FILE)) {
        printf("Can't open %s!\n", FONT_FILE);
        return 1;
    }

    sf::Text status(sf::String("FSP: 0"), font, FONT_SIZE);


    sf::Clock clock;
    sf::Time prev_time = clock.getElapsedTime();
    sf::Time curr_time = sf::seconds(0);
    char fps_text[30] = "";


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }


        curr_time = clock.getElapsedTime();
        int fps = (int)(1.0f / (curr_time.asSeconds() - prev_time.asSeconds()));
        
        sprintf(fps_text, "FPS: %i", fps);
        status.setString(fps_text);

        prev_time = curr_time;


        window.clear();
        window.draw(status);
        window.display();
    }

    return 0;
}