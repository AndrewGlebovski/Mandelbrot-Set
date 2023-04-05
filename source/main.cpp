#include <stdio.h>
#include "draw.hpp"


int main() {
    int result = draw_mandelbrot();

    printf("Mandelbrot set!\n");

    return result;
}
