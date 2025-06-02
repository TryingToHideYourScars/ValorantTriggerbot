#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include "helper.h"

LARGE_INTEGER frequency, start, end;
int reaction_count = 0;
double total_reaction = 0.0;

typedef struct {
	char key_name[256];
	int key;
} SCAN_CODES;

void disable_quickedit() {
	HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prevMode;

    GetConsoleMode(hInput, &prevMode);
    SetConsoleMode(hInput, prevMode & ~ENABLE_QUICK_EDIT_MODE);
}

void free_config(CONFIG *pCfg) {
    free(pCfg->hold_key);
    free(pCfg->target_color);
}

bool get_config(CONFIG *pCfg) {
    pCfg->target_color = get_str("target_color");
    pCfg->hold_key     = get_str("hold_key");
    pCfg->scan_area_x  = get_int("scan_area_x");
    pCfg->scan_area_y  = get_int("scan_area_y");
    pCfg->color_sens   = get_int("color_sens");
    pCfg->tap_time     = get_float("tap_time");
    pCfg->hold_mode     = get_int("hold_mode");

    if (pCfg->scan_area_x == -1 ||
    	pCfg->scan_area_y == -1 ||
        pCfg->color_sens == -1  ||
        pCfg->tap_time == -1 	||
        pCfg->hold_key == 0 	||
        pCfg->hold_mode == -1   ||
        pCfg->target_color == 0)
    {
        return false;
    }

    return true;
}

unsigned int *get_screenshot(const char *save_name, int crop_width, int crop_height) {
    unsigned int *pixels = 0;
    HDC mem_dc = 0;
    FILE *file = 0;

    HDC screen_dc = GetDC(0);
    HBITMAP bitmap = 0;
    BITMAP bmp = {0};
    BITMAPFILEHEADER bmp_file_header = {0};

    if (screen_dc == 0) {
        printf("ERROR: GetDC() failed!");
        return 0;
    }

    int crop_x = 0;
    int crop_y = 0;

    int screen_width = GetDeviceCaps(screen_dc, DESKTOPHORZRES);
    int screen_height = GetDeviceCaps(screen_dc, DESKTOPVERTRES);


    crop_x = (screen_width - crop_width) / 2;
    crop_y = (screen_height - crop_height) / 2;


    mem_dc = CreateCompatibleDC(screen_dc);

    if (mem_dc == 0) {
        printf("ERROR: CreateCompatibleDC() failed!");
        ReleaseDC(0, screen_dc);
        return 0;
    }

    bitmap = CreateCompatibleBitmap(screen_dc, crop_width, crop_height);

    if (bitmap == 0) {
        printf("ERROR: CreateCompatibleBitmap() failed.");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        return 0;
    }

    SelectObject(mem_dc, bitmap);


    if (BitBlt(mem_dc, 0, 0, crop_width, crop_height, screen_dc, crop_x, crop_y, SRCCOPY) == 0) {
        printf("ERROR: BitBlt() failed!");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        DeleteObject(bitmap);
        return 0;
    }


    GetObject(bitmap, sizeof(BITMAP), &bmp);


    bmp_file_header.bfType = 0x4D42;
    bmp_file_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                             bmp.bmWidthBytes * crop_height;
    bmp_file_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);


    BITMAPINFOHEADER bmp_info_header;
    memset(&bmp_info_header, 0, sizeof(BITMAPINFOHEADER));
    bmp_info_header.biSize = sizeof(BITMAPINFOHEADER);
    bmp_info_header.biWidth = bmp.bmWidth;
    bmp_info_header.biHeight = bmp.bmHeight;
    bmp_info_header.biPlanes = 1;
    bmp_info_header.biBitCount = 32;
    bmp_info_header.biCompression = BI_RGB;
    bmp_info_header.biSizeImage = bmp.bmWidthBytes * bmp.bmHeight;

    if (save_name) {
        file = fopen(save_name, "wb");

        if (!file) {
            printf("ERROR: fopen() failed!");
            ReleaseDC(0, screen_dc);
            DeleteDC(mem_dc);
            DeleteObject(bitmap);
            return 0;
        }

        fwrite(&bmp_file_header, sizeof(BITMAPFILEHEADER), 1, file);
        fwrite(&bmp_info_header, sizeof(BITMAPINFOHEADER), 1, file);
    }

    /* Allocate buffer for bitmap data */
    pixels = (unsigned int *)malloc(bmp.bmWidthBytes * crop_height);

    if (!pixels) {
        printf("ERROR: malloc() failed!");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        DeleteObject(bitmap);
        return 0;
    }

    /* Get bitmap data */
    if (GetDIBits(mem_dc, bitmap, 0, crop_height, pixels, (BITMAPINFO *)&bmp_info_header, DIB_RGB_COLORS) == 0) {
        printf("ERROR: GetDIBits() failed!");
        ReleaseDC(0, screen_dc);
        DeleteDC(mem_dc);
        DeleteObject(bitmap);
        free(pixels);
        return 0;
    }

    if (save_name) {
        fwrite(pixels, bmp.bmWidthBytes * crop_height, 1, file);
        fclose(file);
    }

    ReleaseDC(0, screen_dc);
    DeleteDC(mem_dc);
    DeleteObject(bitmap);

    return pixels;
}

bool is_color_found(DWORD* pPixels, int pixel_count, int red, int green, int blue, int color_sens) {
    for (int i = 0; i < pixel_count; i++) {
        DWORD pixelColor = pPixels[i];
        int r = GetRed(pixelColor);
        int g = GetGreen(pixelColor);
        int b = GetBlue(pixelColor);
        
        if (abs(r - red) <= color_sens && 
            abs(g - green) <= color_sens && 
            abs(b - blue) <= color_sens) {
            return true;
        }
    }
    return false;
}
    

int get_key_code(char* input_key) {

	SCAN_CODES keys[] =
	{
        {"left_mouse_button", 0x01},
        {"right_mouse_button", 0x02},
        {"x1", 0x05},
        {"x2", 0x06},
        {"num_0", 0x30},
        {"num_1", 0x31},
        {"num_2", 0x32},
        {"num_3", 0x33},
        {"num_4", 0x34},
        {"num_5", 0x35},
        {"num_6", 0x36},
        {"num_7", 0x37},
        {"num_8", 0x38},
        {"num_9", 0x39},
        {"a", 0x41},
        {"b", 0x42},
        {"c", 0x43},
        {"d", 0x44},
        {"e", 0x45},
        {"f", 0x46},
        {"g", 0x47},
        {"h", 0x48},
        {"i", 0x49},
        {"j", 0x4A},
        {"k", 0x4B},
        {"l", 0x4C},
        {"m", 0x4D},
        {"n", 0x4E},
        {"o", 0x4F},
        {"p", 0x50},
        {"q", 0x51},
        {"r", 0x52},
        {"s", 0x53},
        {"t", 0x54},
        {"u", 0x55},
        {"v", 0x56},
        {"w", 0x57},
        {"x", 0x58},
        {"y", 0x59},
        {"z", 0x5A},
        {"backspace", 0x08},
        {"down_arrow", 0x28},
        {"enter", 0x0D},
        {"esc", 0x1B},
        {"home", 0x24},
        {"insert", 0x2D},
        {"left_alt", 0xA4},
        {"left_ctrl", 0xA2},
        {"left_shift", 0xA0},
        {"page_down", 0x22},
        {"page_up", 0x21},
        {"right_alt", 0xA5},
        {"right_ctrl", 0xA3},
        {"right_shift", 0xA1},
        {"space", 0x20},
        {"tab", 0x09},
        {"up_arrow", 0x26},
        {"f1", 0x70},
        {"f2", 0x71},
        {"f3", 0x72},
        {"f4", 0x73},
        {"f5", 0x74},
        {"f6", 0x75},
        {"f7", 0x76},
        {"f8", 0x77},
        {"f9", 0x78},
        {"f10", 0x79},
        {"f11", 0x7A},
        {"f12", 0x7B}
    };


    int num_keys = sizeof(keys) / sizeof(keys[0]);

    for(int i = 0; i < num_keys; i++) {
        if(strcmp(keys[i].key_name, input_key) == 0 ) {
            return keys[i].key;
        }
    }
    return -1;
}

bool get_valorant_colors(const char* pColor, int *pRed, int *pGreen, int *pBlue) {
    if (strcmp(pColor, "purple") == 0) {
        *pRed = 250;
        *pGreen = 100;
        *pBlue = 250;

        return true;
    }
    else if (strcmp(pColor, "yellow") == 0) {
        *pRed = 254;
        *pGreen = 254;
        *pBlue = 64;

        return true;
    }
    else {
        return false;
    }
}

char* get_str(char* key_name) {
    FILE* file;
    char line[256];

    char* key;
    char* value;

    file = fopen("config.txt", "r");

    if (!file) {
        printf("The config.txt file was not found. Using default config.\n");

        file = fopen("config.txt", "w");
        if (file) {
            fprintf(file, "hold_mode=1\n");
            fprintf(file, "hold_key=left_alt\n");
            fprintf(file, "target_color=purple\n");
            fprintf(file, "color_sens=65\n");
            fprintf(file, "tap_time=100\n");
            fprintf(file, "scan_area_x=8\n");
            fprintf(file, "scan_area_y=8\n");
            fclose(file);
        }
        else {
            printf("Error: Could not create the .txt file.\n");
        }
        file = fopen("config.txt", "r");
    }



    while (fgets(line, sizeof(line), file)) {
        key = strtok(line, "=");
        value = strtok(0, "=");

        if (value != 0) {

            size_t len = strlen(value);
            if (len > 0 && value[len - 1] == '\n') {
                value[len - 1] = '\0';
            }
        }

        if (strcmp(key_name, key) == 0) {
            fclose(file);
            char* ret = (char*)malloc(strlen(value) + 1);
            strcpy(ret, value);
            return ret;
        }
    }

    fclose(file);
    printf("Error: The variable name could not be found in the txt file.\n");
    return 0;
}


int get_int(char* key_name) {
    char *str = get_str(key_name);
    if (str == 0) {
        return -1;
    }
    int ret = atoi(str);
	free(str);
    return ret;
}

double get_float(char* key_name) {
	double dnum = 0;
	char* str = get_str(key_name);
	if (str == 0) {
		return -1;
	}
	char local_value[256];
	strcpy(local_value, str);
	sscanf(local_value, "%lf", &dnum);
	free(str);
	return dnum;
}

double get_reaction_average(double total_reaction, int reaction_count) {
	return total_reaction / reaction_count;
} 

void init_performance_counters() {
    QueryPerformanceFrequency(&frequency);
}

void start_counter() {
    QueryPerformanceCounter(&start); // Start timer
}

void stop_counter() {
    double elapsed_time;
    QueryPerformanceCounter(&end); // End timer
    elapsed_time = (double)(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;
    reaction_count++;
    total_reaction += elapsed_time;
    printf("\rReaction time: %.2f ms ", get_reaction_average(total_reaction, reaction_count));
}

void left_click() {

    INPUT input_down = {0};
    input_down.type = INPUT_MOUSE;
    input_down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input_down, sizeof(INPUT));


    Sleep(3);


    INPUT input_up = {0};
    input_up.type = INPUT_MOUSE;
    input_up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input_up, sizeof(INPUT));
}

bool is_key_pressed(int hold_key) {
	return GetAsyncKeyState(hold_key) & 0x8000;
}

void print_logo_content() {
    printf("  ██████  ██▓███   ▒█████   ▄▄▄█████▓ ██▓  █████▒▓██   ██▓  \n");
    printf("▒██    ▒ ▓██░  ██▒▒██▒  ██▒ ▓  ██▒ ▓▒▓██▒▓██   ▒  ▒██  ██▒  \n");
    printf("░ ▓██▄   ▓██░ ██▓▒▒██░  ██▒ ▒ ▓██░ ▒░▒██▒▒████ ░   ▒██ ██░  \n");
    printf("  ▒   ██▒▒██▄█▓▒ ▒▒██   ██░ ░ ▓██▓ ░ ░██░░▓█▒  ░   ░ ▐██▓░  \n");
    printf("▒██████▒▒▒██▒ ░  ░░ ████▓▒░   ▒██▒ ░ ░██░░▒█░      ░ ██▒▓░  \n");
    printf("▒ ▒▓▒ ▒ ░▒▓▒░ ░  ░░ ▒░▒░▒░    ▒ ░░   ░▓   ▒ ░       ██▒▒▒  \n");
    printf("░ ░▒  ░ ░░▒ ░       ░ ▒ ▒░      ░     ▒ ░ ░       ▓██ ░▒░  \n");
    printf("░  ░  ░  ░░       ░ ░ ░ ▒     ░       ▒ ░ ░ ░     ▒ ▒ ░░   \n");
    printf("      ░               ░ ░             ░           ░ ░      \n");
    printf("                                                  ░ ░      \n");
}

void print_logo() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    system("cls");
    disable_quickedit();

    for (int i = 0; i < 3; ++i) {
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        print_logo_content();
        printf("Version 1.4\n");
        printf("Created by TryingToHideYourScars\n");
        printf("GitHub: https://github.com/TryingToHideYourScars\n");

        Sleep(300);
        system("cls");
        Sleep(150);
    }

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
    print_logo_content();
    printf("Version 1.4\n");
    printf("Created by TryingToHideYourScars\n");
    printf("GitHub: https://github.com/TryingToHideYourScars\n");

    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}
