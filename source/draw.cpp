/**
 * \file
 * \brief Source file for drawing mandelbrot set
*/

#include <SFML/Graphics.hpp>
#include <assert.h>
#include <immintrin.h>
#include "configs.hpp"
#include "draw.hpp"


typedef enum {
    OK                  = 0,        ///< OK
    INVALID_ARG         = 1,        ///< Invalid argument passed to the function
    ALLOC_FAIL          = 2,        ///< Allocation failed
    FILE_NOT_FOUND      = 3,        ///< File not found
    INVALID_FORMAT      = 4,        ///< Color table file has invalid format
} EXIT_CODES;


typedef union {
    __m256 float_vec;
    __m256i int_vec;
    float float_arr[8];
    int int_arr[8];
} VecToArr;


typedef struct {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
} IterColor;


#define ASSERT(condition, exit_code, ...)       \
do {                                            \
    if (!(condition)) {                         \
        printf(__VA_ARGS__);                    \
        return exit_code;                       \
    }                                           \
} while (0)                                     \


/**
 * \brief Set pixels colors in buffer accroding to Mandelbrot formula
 * \param [in]  color_table Containts rgb color for each iteration number
 * \param [out] buffer      Buffer to store pixels colors
*/
void set_pixels(IterColor *color_table, uint8_t *buffer);


/**
 * \brief Set pixel color in buffer based on iterations number and position
 * \param [out] buffer  Buffer to store pixel color
 * \param [in]  N       Number of iterations
 * \param [in]  x       Pixel x coordinate
 * \param [in]  y       Pixel y coordinate
*/
void set_pixel_color(IterColor *color_table, uint8_t *buffer, int N, float x, float y);


/**
 * \brief Loads color table from file into allocated buffer
 * \param [in]  filename    Path to color table source file
 * \param [out] buffer      Buffer to allocate and fill with colors
 * \return Non zero value means error 
*/
int load_color_table(const char *filename, IterColor **buffer);


/**
 * \brief Free color table buffer
 * \param [out] buffer  Buffer to free
 * \return Non zero value means error
*/
int free_color_table(IterColor **buffer);




int draw_mandelbrot(void) {
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Mandelbrot3000");


    sf::Font font;
    ASSERT(font.loadFromFile(FONT_FILE), FILE_NOT_FOUND, "Can't open %s!\n", FONT_FILE);

    sf::Text status(sf::String("FPS: 0"), font, FONT_SIZE);


    sf::Clock clock;
    sf::Time prev_time = clock.getElapsedTime();
    sf::Time curr_time = sf::seconds(0);
    char fps_text[30] = "";


    uint8_t *pixels = (uint8_t *) calloc(SCREEN_W * SCREEN_H * 4, sizeof(uint8_t));
    ASSERT(pixels, ALLOC_FAIL, "Can't allocate buffer for pixels colors!\n");


    IterColor *color_table = nullptr;
    int exitcode = load_color_table("assets/ColorTable.txt", &color_table);
    if (exitcode) return exitcode;


    sf::Image image;
    sf::Texture texture;


    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }


        for (size_t i = 0; i < 100; i++) set_pixels(color_table, pixels);

        image.create(SCREEN_W, SCREEN_H, pixels);
        texture.loadFromImage(image);
        sf::Sprite sprite(texture);


        curr_time = clock.getElapsedTime();
        int fps = (int)(100.0f / (curr_time.asSeconds() - prev_time.asSeconds()));
        sprintf(fps_text, "FPS: %i", fps);
        status.setString(fps_text);
        prev_time = curr_time;

        window.clear();
        window.draw(sprite);
        window.draw(status);
        window.display();
    }

    free(pixels);
    return free_color_table(&color_table);
}


void set_pixels(IterColor *color_table, uint8_t *buffer) {
    assert(color_table && "Color table is null!\n");
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
                set_pixel_color(color_table, buffer, (tmpN.int_arr)[i], (tmpX.float_arr)[8 - i - 1], y0);
                buffer += 4;
            }

            x0 = _mm256_add_ps(x0, _mm256_set1_ps(8.0f * delta_x));
        }

        y0 += delta_y;
    }
}


void set_pixel_color(IterColor *color_table, uint8_t *buffer, int N, float x, float y) {
    assert(buffer && "Can't set pixel color with null buffer!\n");

    buffer[0] = color_table[N].red;
    buffer[1] = color_table[N].green;
    buffer[2] = color_table[N].blue;
    buffer[3] = 255;
}


int load_color_table(const char *filename, IterColor **buffer) {
    ASSERT(filename, INVALID_ARG, "Can't load without filename!\n");
    ASSERT(buffer, INVALID_ARG, "Can't load in null buffer!\n");

    FILE *file = fopen(filename, "r");
    ASSERT(file, FILE_NOT_FOUND, "Color table source file not found!\n");

    *buffer = (IterColor *) calloc(NMAX + 1, sizeof(IterColor));
    ASSERT(*buffer, ALLOC_FAIL, "Can't allocate color table buffer!\n");

    for (int i = 0; i < NMAX + 1; i++) {
        if (fscanf(file, "%hhu %hhu %hhu", &((*buffer + i) -> red), &((*buffer + i) -> green), &((*buffer + i) -> blue)) != 3) {
            free(*buffer);
            *buffer = nullptr;

            printf("Color table source file not found!\n");
            return INVALID_FORMAT;
        }
    }

    fclose(file);

    return OK;
}


int free_color_table(IterColor **buffer) {
    ASSERT(buffer, INVALID_ARG, "Pointer to buffer is null!\n");
    ASSERT(*buffer, INVALID_ARG, "Can't free null buffer!\n");

    free(*buffer);
    *buffer = nullptr;

    return OK;
}
