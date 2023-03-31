/**
 * \file
 * \brief Source file for drawing mandelbrot set
*/

#include <SFML/Graphics.hpp>
#include <assert.h>
#include "configs.hpp"
#include "draw.hpp"




/**
 * \brief Set pixels colors in buffer accroding to Mandelbrot formula
 * \param [out] buffer Buffer to store pixels colors
*/
void set_pixels(uint8_t *buffer);


/**
 * \brief Set pixel color in buffer based on iterations number and position
 * \param [out] buffer  Buffer to store pixel color
 * \param [in]  N       Number of iterations
 * \param [in]  x       Pixel x coordinate
 * \param [in]  y       Pixel y coordinate
*/
void set_pixel_color(uint8_t *buffer, uint8_t N, float x, float y);




int draw_mandelbrot(void) {
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Mandelbrot3000");


    sf::Font font;
    if (!font.loadFromFile(FONT_FILE)) {
        printf("Can't open %s!\n", FONT_FILE);
        return 1;
    }

    sf::Text status(sf::String("FSP: 0"), font, FONT_SIZE);
    status.setFillColor(sf::Color().Black);


    sf::Clock clock;
    sf::Time prev_time = clock.getElapsedTime();
    sf::Time curr_time = sf::seconds(0);
    char fps_text[30] = "";


    uint8_t *pixels = (uint8_t *) calloc(SCREEN_W * SCREEN_H * 4, sizeof(uint8_t));
    
    if (!pixels) {
        printf("Can't allocate buffer for pixels colors!\n");
        return 1;
    }

    sf::Image image;
    sf::Texture texture;


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }


        set_pixels(pixels);
        image.create(SCREEN_W, SCREEN_H, pixels);
        texture.loadFromImage(image);
        sf::Sprite sprite(texture);


        curr_time = clock.getElapsedTime();
        int fps = (int)(1.0f / (curr_time.asSeconds() - prev_time.asSeconds()));
        sprintf(fps_text, "FPS: %i", fps);
        status.setString(fps_text);
        prev_time = curr_time;


        window.clear();
        window.draw(sprite);
        window.draw(status);
        window.display();
    }

    free(pixels);

    return 0;
}


void set_pixels(uint8_t *buffer) {
    assert(buffer && "Can't set pixels with null buffer!\n");

    const float delta_x = SET_W / (float)SCREEN_W;
    const float delta_y = SET_H / (float)SCREEN_H;

    float y0 = CENTER_Y - 0.5f * SET_H;

    for (uint32_t y = 0; y < SCREEN_H; y++) {
        float x0 = CENTER_X - 0.5f * SET_W;

        for (uint32_t x = 0; x < SCREEN_W; x++) {
            float x_i = x0;
            float y_i = y0;

            uint8_t N = 0;
            for (; N < NMAX; N++) {
                float x2 = x_i * x_i;
                float y2 = y_i * y_i;
                float xy = x_i * y_i;

                if ((x2 + y2) > RMAX * RMAX) break;

                x_i = x2 - y2 + x0, y_i = 2 * xy + y0;
            }

            set_pixel_color(buffer, N, x0, y0);
            buffer += 4;

            x0 += delta_x;
        }

        y0 += delta_y;
    }
}


void set_pixel_color(uint8_t *buffer, uint8_t N, float x, float y) {
    assert(buffer && "Can't set pixel color with null buffer!\n");

    buffer[0] = NMAX - N;
    buffer[1] = NMAX - N;
    buffer[2] = NMAX - N;
    buffer[3] = 255;
}
