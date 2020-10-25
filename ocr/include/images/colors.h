#ifndef H_COLOR
#define H_COLOR

typedef enum type { COLOR_RGB, COLOR_RGBA, COLOR_GRAYSCALE, COLOR_BINARY } TYPE;

typedef struct rgb {
    unsigned char red, green, blue;
} RGB;

typedef struct rgba {
    unsigned char red, green, blue, alpha;
} RGBA;

typedef struct grayscale {
    float grayscale;
} GRAYSCALE;

typedef struct binary {
    unsigned char binary : 1;
} BINARY;

typedef union colors {
    RGB *rgb;
    RGBA *rgba;
    GRAYSCALE *grayscale;
    BINARY *binary;
} COLORS;

#endif
