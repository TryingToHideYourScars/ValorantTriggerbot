#include <windows.h>
#include "helper.h"
#include <time.h>
#include <stdlib.h>

static inline int min_val(int a, int b) {
    return (a < b) ? a : b;
}

static inline int anti_debug(void) {
    __asm__ __volatile__("int $0x03");
    return 0;
}

bool find_target_position(DWORD* pixels, int width, int height, int red, int green, int blue, int sens, int* out_x, int* out_y) {

    return false;
}

int main(int argc, char* argv[]) {
    printf("Program starting...\n");
    

    int red = 0, blue = 0, green = 0;
    CONFIG cfg = {0};
    
    printf("Initializing config...\n");
    SetConsoleTitleA("Spotify");
    
    if (!get_config(&cfg)) {
        printf("Config error!\n");
        printf("Press enter to exit: ");
        getchar();
        return 1;
    }

    printf("Config loaded successfully\n");
    if (!get_valorant_colors(cfg.target_color, &red, &green ,&blue)) {
        printf("Error: Unknown color.\n");
        printf("Press enter to exit: ");
        getchar();
        return 1;
    }

    init_performance_counters();
    int pixel_count = cfg.scan_area_x * cfg.scan_area_y;
    get_screenshot(0, cfg.scan_area_x, cfg.scan_area_y);
    print_logo();

    while (true) {
        if (cfg.hold_mode == 0 || is_key_pressed(get_key_code(cfg.hold_key))) {
            unsigned int* pPixels = get_screenshot(0, cfg.scan_area_x, cfg.scan_area_y);
            if (!pPixels) continue;
            
            if (is_color_found((DWORD*)pPixels, pixel_count, red, green, blue, cfg.color_sens)) {
                left_click();
                Sleep(cfg.tap_time);
            }
            free(pPixels);
        }
        Sleep(1);
    }

    free_config(&cfg);
    return 0;
}
