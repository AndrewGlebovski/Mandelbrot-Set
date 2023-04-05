/**
 * \file
 * \brief Source file for drawing mandelbrot set
*/

#include <SFML/Graphics.hpp>
#include <assert.h>
#include <immintrin.h>
#include "configs.hpp"
#include "draw.hpp"


typedef union {
    __m256 float_vec;
    __m256i int_vec;
    float float_arr[8];
    int int_arr[8];
} VecToArr;



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
void set_pixel_color(uint8_t *buffer, int N, float x, float y);




int draw_mandelbrot(void) {
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Mandelbrot3000");


    sf::Font font;
    if (!font.loadFromFile(FONT_FILE)) {
        printf("Can't open %s!\n", FONT_FILE);
        return 1;
    }

    sf::Text status(sf::String("FPS: 0"), font, FONT_SIZE);
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

        __m256 x0 = _mm256_add_ps(
            _mm256_set1_ps(CENTER_X - 0.5f * SET_W),
            _mm256_set_ps(0.0f, delta_x, 2.0f * delta_x, 3.0f * delta_x, 4.0f * delta_x, 5.0f * delta_x, 6.0f * delta_x, 7.0f * delta_x)
        );

        for (uint32_t x = 0; x < SCREEN_W; x += 8) {
            __m256 x_i = x0;
            __m256 y_i = _mm256_set1_ps(y0);

            __m256i N = _mm256_setzero_si256();
            for (;;) {
                __m256 x2 = _mm256_mul_ps(x_i, x_i);
                __m256 y2 = _mm256_mul_ps(y_i, y_i);
                __m256 xy = _mm256_mul_ps(x_i, y_i);


                __m256 res1 = _mm256_cmp_ps(_mm256_add_ps(x2, y2), _mm256_set1_ps(RMAX * RMAX), _CMP_LT_OS);
                if (_mm256_testz_si256(_mm256_castps_si256(res1), _mm256_set1_epi32(0xFFFFFFFF))) break;

                N = _mm256_add_epi32(N, _mm256_and_si256(_mm256_castps_si256(res1), _mm256_set1_epi32(1)));

                __m256i res2 = _mm256_cmpeq_epi32(N, _mm256_set1_epi32(NMAX));
                if (!_mm256_testz_si256(res2, _mm256_set1_epi32(0xFFFFFFFF))) break;


                x_i = _mm256_add_ps(_mm256_sub_ps(x2, y2), x0);
                y_i = _mm256_add_ps(_mm256_mul_ps(xy, _mm256_set1_ps(2.0f)), _mm256_set1_ps(y0));
            }

            VecToArr tmpN = {}, tmpX = {};
            tmpN.int_vec = N, tmpX.float_vec = x0;

            for (int i = 7; i >= 0; i--) {
                set_pixel_color(buffer, (tmpN.int_arr)[i], (tmpX.float_arr)[8 - i - 1], y0);
                buffer += 4;
            }

            x0 = _mm256_add_ps(x0, _mm256_set1_ps(8.0f * delta_x));
        }

        y0 += delta_y;
    }
}


void set_pixel_color(uint8_t *buffer, int N, float x, float y) {
    assert(buffer && "Can't set pixel color with null buffer!\n");

    buffer[0] = (uint8_t) (NMAX - N);
    buffer[1] = (uint8_t) (NMAX - N);
    buffer[2] = (uint8_t) (NMAX - N);
    buffer[3] = 255;
}
