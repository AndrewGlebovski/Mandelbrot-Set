/**
 * \file
 * \brief Source file for drawing mandelbrot set
*/

#include <SFML/Graphics.hpp>
#include <assert.h>
#include <immintrin.h>
#include "configs.hpp"
#include "draw.hpp"


const size_t FPS_BUFFER_SIZE = 100;
const size_t FPS_TEXT_SIZE = 30;
const size_t TEST_NUMBER = 1;


/// Possible functions exit codes
typedef enum {
    OK                  = 0,        ///< OK
    INVALID_ARG         = 1,        ///< Invalid argument passed to the function
    ALLOC_FAIL          = 2,        ///< Allocation failed
    FILE_NOT_FOUND      = 3,        ///< File not found
    INVALID_FORMAT      = 4,        ///< Color table file has invalid format
} EXIT_CODES;


/// Contains information about Mandelbrot set offset and scale
typedef struct {
    float center_x = CENTER_X;      ///< Offset x
    float center_y = CENTER_Y;      ///< Offset y
    float set_w = SET_W;            ///< Scale x
    float set_h = SET_H;            ///< Scale y
} Transform;


typedef union {
    __m256 float_vec;
    __m256i int_vec;
    float float_arr[8];
    int int_arr[8];
} VecToArr;


/// Contains information about color in RGB format
typedef struct {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
} IterColor;


typedef struct {
    sf::Window  *window = nullptr;      ///< Application window
    Transform   *transform = nullptr;   ///< Mandelbrot set transformation
} EventArgs;


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
 * \param [in]  transform   Mandelbrot set offset and scale
*/
void set_pixels(const IterColor *color_table, uint8_t *buffer, const Transform *transform);


/**
 * \brief Set pixel color in buffer based on iterations number and position
 * \param [out] buffer  Buffer to store pixel color
 * \param [in]  N       Number of iterations
 * \param [in]  x       Pixel x coordinate
 * \param [in]  y       Pixel y coordinate
*/
void set_pixel_color(const IterColor *color_table, uint8_t *buffer, int N, float x, float y);


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


/**
 * \brief Change Mandelbrot Transform according to the user input
 * \brief [in]  event       To handle input
 * \param [out] transform   Transform to change
*/
void transform_input(sf::Event event, Transform *transform);


/**
 * \brief Prints fps and store FPS into FPS buffer
 * \param [out] status      Text class to fill with FPS
 * \param [in]  clock       Clock class to get time passed
 * \param [out] prev_time   Required to calculate FPS
 * \param [out] fps_buffer  FPS buffer to store FPS value
*/
void print_fps(sf::Text *status, sf::Clock *clock, sf::Time *prev_time, int *fps_buffer);


/**
 * \brief Prints what FPS buffer contains
 * \param [in] fps_buffer   FPS buffer
*/
void show_fps_buffer(int *fps_buffer);


/**
 * \brief Handles all types of events
 * \param [in,out] args Contains all necessary arguments
*/
void event_parser(EventArgs *args);




int draw_mandelbrot(void) {
    sf::RenderWindow window(sf::VideoMode(SCREEN_W, SCREEN_H), "Mandelbrot3000");

    sf::Font font;
    ASSERT(font.loadFromFile(FONT_FILE), FILE_NOT_FOUND, "Can't open %s!\n", FONT_FILE);

    sf::Text status(sf::String("FPS: 0"), font, FONT_SIZE);

    sf::Clock clock;
    sf::Time prev_time = clock.getElapsedTime();

    uint8_t *pixels = (uint8_t *) calloc(SCREEN_W * SCREEN_H * 4, sizeof(uint8_t));
    ASSERT(pixels, ALLOC_FAIL, "Can't allocate buffer for pixels colors!\n");

    IterColor *color_table = nullptr;
    if (load_color_table("assets/ColorTable.txt", &color_table)) return 1;

    sf::Image image;
    sf::Texture texture;

    Transform transform = {};

    int *fps_buffer = (int *) calloc(FPS_BUFFER_SIZE, sizeof(int));
    ASSERT(fps_buffer, ALLOC_FAIL, "Failed to allocate FPS Buffer!\n");

    EventArgs event_args = {&window, &transform};

    while (window.isOpen()) {
        event_parser(&event_args);

        for (size_t i = 0; i < TEST_NUMBER; i++) set_pixels(color_table, pixels, &transform);

        image.create(SCREEN_W, SCREEN_H, pixels);
        texture.loadFromImage(image);
        sf::Sprite sprite(texture);

        print_fps(&status, &clock, &prev_time, fps_buffer);

        window.clear();
        window.draw(sprite);
        window.draw(status);
        window.display();
    }

    show_fps_buffer(fps_buffer);
    free(fps_buffer);

    free(pixels);
    return free_color_table(&color_table);
}


void print_fps(sf::Text *status, sf::Clock *clock, sf::Time *prev_time, int *fps_buffer) {
    assert(prev_time && "Can't print fps without prev time pointer!\n");
    assert(fps_buffer && "Can't print fps without fps buffer!\n");

    static size_t fps_index = 0;

    sf::Time curr_time = clock -> getElapsedTime();

    int fps = (int)((float) TEST_NUMBER * 1.0f / (curr_time.asSeconds() - prev_time -> asSeconds()));

    char fps_text[FPS_TEXT_SIZE] = "";
    sprintf(fps_text, "FPS: %i", fps);
    status -> setString(fps_text);

    *prev_time = curr_time;

    fps_buffer[fps_index] = fps;
    fps_index = (fps_index + 1) % FPS_BUFFER_SIZE;
}


void show_fps_buffer(int *fps_buffer) {
    assert(fps_buffer && "Can't show null fps buffer!\n");

    for (size_t i = 0; i < FPS_BUFFER_SIZE; i++) printf("%d, ", fps_buffer[i]);
    putchar('\n');
}


void event_parser(EventArgs *args) {
    assert(args && "Event parser can't work with null args!\n");
    assert(args -> window && "Event parser can't work with null window!\n");
    assert(args -> transform && "Event parser can't work with null transform!\n");

    sf::Event event;
    while (args -> window -> pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            args -> window -> close();
            return;
        }
        
        transform_input(event, args -> transform);
    }
}


void transform_input(sf::Event event, Transform *transform) {
    assert(transform && "Transformation pointer in null!\n");

    switch (event.type) {
        case sf::Event::KeyPressed: {
            switch (event.key.code) {
                case sf::Keyboard::Up:
                    transform -> center_y -= MOVE_FACTOR * transform -> set_h; return;

                case sf::Keyboard::Down:
                    transform -> center_y += MOVE_FACTOR * transform -> set_h; return;

                case sf::Keyboard::Left:
                    transform -> center_x -= MOVE_FACTOR * transform -> set_w; return;

                case sf::Keyboard::Right:
                    transform -> center_x += MOVE_FACTOR * transform -> set_w; return;
                
                default: return;
            }
        }
        case sf::Event::MouseWheelScrolled: {
            if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                if (event.mouseWheelScroll.delta > 0) {
                    transform -> set_w *= ZOOM_FACTOR;
                    transform -> set_h *= ZOOM_FACTOR;
                }
                else {
                    transform -> set_w /= ZOOM_FACTOR;
                    transform -> set_h /= ZOOM_FACTOR;
                }
            }

            break;
        }

        default: return;
    }
}


void set_pixels(const IterColor *color_table, uint8_t *buffer, const Transform *transform) {
    assert(color_table && "Color table is null!\n");
    assert(buffer && "Can't set pixels with null buffer!\n");

    const float delta_x = transform -> set_w / (float)SCREEN_W;
    const float delta_y = transform -> set_h / (float)SCREEN_H;

    float y0 = transform -> center_y - 0.5f * transform -> set_h;

    for (uint32_t y = 0; y < SCREEN_H; y++) {

        __m256 x0 = _mm256_add_ps(
            _mm256_set1_ps(transform -> center_x - 0.5f * transform -> set_w),
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


void set_pixel_color(const IterColor *color_table, uint8_t *buffer, int N, float x, float y) {
    assert(buffer && "Can't set pixel color with null buffer!\n");

    if (0 < N && N < NMAX) {
        buffer[0] = color_table[N % POSSIBLE_COLORS].red;
        buffer[1] = color_table[N % POSSIBLE_COLORS].green;
        buffer[2] = color_table[N % POSSIBLE_COLORS].blue;
        buffer[3] = 255;
    }
    else {
        buffer[0] = 0;
        buffer[1] = 0;
        buffer[2] = 0;
        buffer[3] = 255;
    }
}


int load_color_table(const char *filename, IterColor **buffer) {
    ASSERT(filename, INVALID_ARG, "Can't load without filename!\n");
    ASSERT(buffer, INVALID_ARG, "Can't load in null buffer!\n");

    FILE *file = fopen(filename, "r");
    ASSERT(file, FILE_NOT_FOUND, "Color table source file not found!\n");

    *buffer = (IterColor *) calloc(POSSIBLE_COLORS, sizeof(IterColor));
    ASSERT(*buffer, ALLOC_FAIL, "Can't allocate color table buffer!\n");

    for (int i = 0; i < POSSIBLE_COLORS; i++) {
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
