#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <SDL2/SDL.h>
#include <zlib.h>

#define PNG_COLORTYPE_GRAYSCALE 0
#define PNG_COLORTYPE_TRUECOLOR 2
#define PNG_COLORTYPE_INDEXED 3
#define PNG_COLORTYPE_GRAYSCALE_ALPHA 4
#define PNG_COLORTYPE_TRUECOLOR_ALPHA 6

#define PNG_FILTERTYPE_NONE 0
#define PNG_FILTERTYPE_SUB 1
#define PNG_FILTERTYPE_UP 2
#define PNG_FILTERTYPE_AVERAGE 3
#define PNG_FILTERTYPE_PAETH 4

struct PNG_IHDR {
        uint32_t length;
        uint8_t type[4];
        uint32_t width;
        uint32_t height;
        uint8_t bit_depth;
        uint8_t color_type;
        uint8_t compression_method;
        uint8_t filter_method;
        uint8_t interlace_method;
}__attribute__((packed));

struct PNG_CHUNK_BASIC {
        uint32_t length;
        uint8_t type[4];
}__attribute__((packed));

struct PNG_TRUECOLOR_PIXEL8 {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
}__attribute__((packed));

struct PNG_TRUECOLOR_PIXEL16 {
        uint16_t r;
        uint16_t g;
        uint16_t b;
        uint16_t a;
}__attribute__((packed));

int paeth(int a, int b, int c) {
        int p = a + b - c;
        int pa = abs(p - a);
        int pb = abs(p - b);
        int pc = abs(p - c);
        if (pa <= pb && pa <= pc) return a;
        else if (pb <= pc) return b;
        else return c;
}

int main(int argc, char* argv[]) {
        if (argc != 2) {
                printf("wrong number of arguments\nusage: pngview image_path\n");
                return 1;
        }

        FILE *file = fopen(argv[1], "r");
        if (file == NULL) {
                printf("file %s could not be opened\n", argv[1]);
                return 2;
        }
        fseek(file, 0L, SEEK_END);
        long size = ftell(file);
        rewind(file);

        char *filecontents = malloc(size);
        int fread_result = fread(filecontents, 1, size, file);
        if (fread_result != size) {
                printf("read did not complete, read %d/%d bytes\n", fread_result, size);
                return 8;
        }

        char png_magic[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
        if (strncmp(filecontents, png_magic, 8) != 0) {
                printf("file %s is not a valid png file\n", argv[1]);
                return 3;
        }

        struct PNG_IHDR *ihdr = (struct PNG_IHDR*)(filecontents + 8);
        if (strncmp(ihdr->type, "IHDR", 4) != 0) {
                printf("invalid ihdr\n");
                return 4;
        }

        ihdr->length = ntohl(ihdr->length);
        ihdr->width = ntohl(ihdr->width);
        ihdr->height = ntohl(ihdr->height);
        if ((ihdr->color_type != PNG_COLORTYPE_TRUECOLOR && ihdr->color_type != PNG_COLORTYPE_TRUECOLOR_ALPHA) || ihdr->interlace_method != 0) {
                printf("pngview does not support this type of png, pngview only supports non-interlaced truecolor (2 or 6)\n%s is of type %d\n", argv[1], ihdr->color_type);
                return 5;
        }

        int offset = 8 + 4 + 4 + ihdr->length + 4;
        uint8_t found = 0;

        uint8_t *compressed_data = NULL;
        size_t compressed_data_size = 0;

        while (offset < size) {
                if (offset + sizeof(struct PNG_CHUNK_BASIC) > size) {
                        printf("unexpected eof while reading chunk\n");
                        return 7;
                }

                struct PNG_CHUNK_BASIC *next_chunk = (struct PNG_CHUNK_BASIC*)(filecontents + offset);
                next_chunk->length = ntohl(next_chunk->length);

                if (strncmp(next_chunk->type, "IDAT", 4) == 0) {
                        compressed_data = realloc(compressed_data, compressed_data_size + next_chunk->length);
                        memcpy(compressed_data + compressed_data_size, filecontents + offset + 8, next_chunk->length);
                        compressed_data_size += next_chunk->length;
                } else if (strncmp(next_chunk->type, "IEND", 4) == 0) {
                        break;
                }

                offset += 8 + next_chunk->length + 4;
        }

        uint8_t bpp;
        switch (ihdr->color_type) {
                case PNG_COLORTYPE_TRUECOLOR: {
                        bpp = 3 * (ihdr->bit_depth / 8);
                        break;
                }
                case PNG_COLORTYPE_TRUECOLOR_ALPHA: {
                        bpp = 4 * (ihdr->bit_depth / 8);
                        break;
                }
        }

        size_t row_bytes = ihdr->width * bpp;
        size_t expected_size = (row_bytes + 1) * ihdr->height;
        uint8_t *decompressed_data = malloc(expected_size);
        uLongf sizeulongf = expected_size;
        int uncompress_result = uncompress(decompressed_data, &sizeulongf, compressed_data, compressed_data_size);
        if (uncompress_result != Z_OK || sizeulongf != expected_size) {
                printf("failed to decompress image data or unexpected size\nexpected %d, got %d\n", expected_size, sizeulongf);
                return 6;
        }

        uint8_t *real_pixels = malloc(ihdr->width * ihdr->height * bpp);

        for (int y = 0; y < ihdr->height; y++) {
                uint8_t *src = decompressed_data + y * (row_bytes + 1);
                uint8_t filter_type = src[0];
                uint8_t *row_data = src + 1;

                uint8_t *dst = real_pixels + y * row_bytes;

                switch(filter_type) {
                        case PNG_FILTERTYPE_NONE: {
                                memcpy(dst, row_data, row_bytes);
                                break;
                        }
                        case PNG_FILTERTYPE_SUB: {
                                for (int x = 0; x < ihdr->width * bpp; x++) {
                                        if (x < bpp) {
                                                dst[x] = row_data[x];
                                        } else {
                                                dst[x] = row_data[x] + dst[x - bpp];
                                        }
                                }
                                break;
                        }
                        case PNG_FILTERTYPE_UP: {
                                for (int x = 0; x < ihdr->width * bpp; x++) {
                                        if (y == 0) {
                                                dst[x] = row_data[x];
                                        } else {
                                                dst[x] = row_data[x] + real_pixels[(y-1) * row_bytes + x];
                                        }
                                }
                                break;
                        }
                        case PNG_FILTERTYPE_AVERAGE: {
                                for (int x = 0; x < ihdr->width * bpp; x++) {
                                        int left = (x >= bpp) ? dst[x - bpp] : 0;
                                        int above = (y > 0) ? real_pixels[(y-1) * row_bytes + x] : 0;
                                        dst[x] = row_data[x] + (left + above) / 2;
                                }
                                break;
                        }
                        case PNG_FILTERTYPE_PAETH: {
                                for (int x = 0; x < ihdr->width * bpp; x++) {
                                        int left = (x >= bpp) ? dst[x - bpp] : 0;
                                        int above = (y > 0) ? real_pixels[(y-1) * row_bytes + x] : 0;
                                        int left_above = (x >= bpp && y > 0) ? real_pixels[(y-1) * row_bytes + (x - bpp)] : 0;
                                        dst[x] = row_data[x] + paeth(left, above, left_above);
                                }
                        }
                }
        }

        SDL_Init(SDL_INIT_VIDEO);
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_CreateWindowAndRenderer(ihdr->width, ihdr->height, 0, &window, &renderer);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        for (int i = 0; i < ihdr->width * ihdr->height * bpp; i += bpp) {
                switch(ihdr->bit_depth) {
                        case 8: {
                                struct PNG_TRUECOLOR_PIXEL8 *pixel = (struct PNG_TRUECOLOR_PIXEL8*)(real_pixels + i);
                                uint8_t alpha = pixel->a;
                                if (ihdr->color_type != PNG_COLORTYPE_TRUECOLOR_ALPHA) alpha = SDL_ALPHA_OPAQUE;
                                SDL_SetRenderDrawColor(renderer, pixel->r, pixel->g, pixel->b, alpha);

                                int pix_i = i / bpp;
                                SDL_RenderDrawPoint(renderer, pix_i % ihdr->width, pix_i / ihdr->width);

                                break;
                        }
                        case 16: {
                                struct PNG_TRUECOLOR_PIXEL16 *pixel = (struct PNG_TRUECOLOR_PIXEL16*)(real_pixels + i);
                                pixel->r = ntohs(pixel->r);
                                pixel->g = ntohs(pixel->g);
                                pixel->b = ntohs(pixel->b);
                                pixel->a = ntohs(pixel->a);
                                uint16_t alpha = pixel->a;
                                if (ihdr->color_type != PNG_COLORTYPE_TRUECOLOR_ALPHA) alpha = SDL_ALPHA_OPAQUE;
                                
                                SDL_SetRenderDrawColor(renderer, pixel->r, pixel->g, pixel->b, alpha);
                                int pix_i = i / bpp;
                                SDL_RenderDrawPoint(renderer, pix_i % ihdr->width, pix_i / ihdr->width);

                                break;
                        
                        }
                }
        }

        SDL_RenderPresent(renderer);

        uint8_t quit = 0;
        SDL_Event event;
        while(!quit) {
                while (SDL_PollEvent(&event)) {
                        if (event.type == SDL_QUIT) {
                                quit = 1;
                        }
                }

                SDL_Delay(10);
        }

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
}
