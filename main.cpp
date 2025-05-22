/*
░░░░░░░░░░░▄█░░░░░░░░░░░░█▄░░░░░░░░░░░
░░░░░░░░░▄██▀░░░░░░░░░░░░▀██░░░░░░░░░░
░░░░░░░░▄███░░░█░░░░░░█░░░███▄░░░░░░░░
░░░░░░░████░░▄█░░░░░░░░█▄░░████░░░░░░░
░░░░░░▄███░▄██▀░░░░░░░░▀██░░███▄░░░░░░
░░░█░░███▀░██░░░░░░░░░░░▀██░▀███░░█░░░
░░░██░░██░█▀▄█░░░░░░░░░░█▄▀█░██░░██░░░
▄░░▀██░▀█░░██░░░░░░░░░░░░██░░█▀░██▀░░▄
▀█▄░▀██▄▀░▄██░░░░░░░░░░░░██▄░▀▄██▀░▄█▀
░▀██▄░▀▀░░▀██▄░░░░░░░░░░▄██▀░░▀▀░▄██▀░
░▄▀▀███▄░░░████▄░░░░░░▄████░░░▄███▀▀▄░
░▀█▄▄░▀▀▀░░▀███▀░░░░░░▀███▀░░▀▀▀░▄▄█▀░
░░░▀▀██████░░▀█▄░░░░░░▄█▀░░██████▀▀░░░
░░░░▄▄▄▄▄██▀░░▄██▄░░▄██▄░▀▀██▄▄▄▄▄░░░░
░░░░░░░░░▄▄▀░▄██▀░░░░▀██▄░█▄▄░░░░░░░░░
░░░░░░░▀▀▀░░▀▀▀░░░░░░░░▀▀▀░░▀▀▀░░░░░░░
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstdint>

// window.h
#pragma pack(push, 1)
struct BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};

struct BITMAPINFOHEADER {
    uint32_t biSize;
    int32_t  biWidth;
    int32_t  biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t  biXPelsPerMeter;
    int32_t  biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};
#pragma pack(pop)

class BMPProcessor {
private:
    BITMAPFILEHEADER bmfh;
    BITMAPINFOHEADER bmih;
    std::vector<uint8_t> pixelData;
    int width;
    int height;
    bool is32Bit;

    void SetPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;

        int bytesPerPixel = is32Bit ? 4 : 3;
        int offset = (y * width + x) * bytesPerPixel;

        pixelData[offset] = b;
        pixelData[offset + 1] = g;
        pixelData[offset + 2] = r;
    }

public:
    BMPProcessor() : width(0), height(0), is32Bit(false) {}

    // чтение
    bool ReadBMP(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        file.read(reinterpret_cast<char*>(&bmfh), sizeof(bmfh));
        if (file.gcount() != sizeof(bmfh) || bmfh.bfType != 0x4D42) {
            file.close();
            return false;
        }

        file.read(reinterpret_cast<char*>(&bmih), sizeof(bmih));
        if (file.gcount() != sizeof(bmih) || bmih.biSize < sizeof(BITMAPINFOHEADER)) {
            file.close();
            return false;
        }

        // проверка формата
        if ((bmih.biBitCount != 24 && bmih.biBitCount != 32) || bmih.biCompression != 0) {
            file.close();
            return false;
        }

        is32Bit = (bmih.biBitCount == 32);
        width = bmih.biWidth;
        height = bmih.biHeight;

        // обработка отрицательной высоты
        if (height < 0) 
            height = -height;

        int bytesPerPixel = is32Bit ? 4 : 3; // выбираем вес пикселя 
        int rowSize = (width * bytesPerPixel + 3) & ~3;
        pixelData.resize(width * height * bytesPerPixel);

        file.seekg(bmfh.bfOffBits, std::ios::beg); // переход к началу данных пикселей

        // чтение строк
        for (int y = 0; y < height; ++y) {
            std::vector<uint8_t> buffer(rowSize);
            file.read(reinterpret_cast<char*>(buffer.data()), rowSize);
            if (file.gcount() != rowSize) {
                file.close();
                return false;
            }

            int targetY = height - 1 - y;
            uint8_t* targetRow = pixelData.data() + targetY * width * bytesPerPixel;
            std::copy(buffer.begin(), buffer.begin() + width * bytesPerPixel, targetRow);
        }

        file.close();
        return true;
    }

    // выводим 
    void DisplayToConsole() {
        int bytesPerPixel = is32Bit ? 4 : 3;
        for (int y = 0; y < height; ++y) {
            const uint8_t* row = pixelData.data() + y * width * bytesPerPixel;
            for (int x = 0; x < width; ++x) {
                const uint8_t* pixel = row + x * bytesPerPixel;
                if (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0) {
                    std::cout << '#';
                } else {
                    std::cout << ' ';
                }
            }
            std::cout << '\n';
        }
    }

    // алгоритм Брезенхема
    void DrawLine(int x1, int y1, int x2, int y2) {
        x1 = std::clamp(x1, 0, width - 1);
        y1 = std::clamp(y1, 0, height - 1);
        x2 = std::clamp(x2, 0, width - 1);
        y2 = std::clamp(y2, 0, height - 1);

        int dx = abs(x2 - x1);
        int dy = -abs(y2 - y1);
        int sx = x1 < x2 ? 1 : -1;
        int sy = y1 < y2 ? 1 : -1;
        int err = dx + dy;

        while (true) {
            SetPixel(x1, y1, 0, 0, 0);
            if (x1 == x2 && y1 == y2) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x1 += sx; }
            if (e2 <= dx) { err += dx; y1 += sy; }
        }
    }

    // крестик
    void DrawCross() {
        DrawLine(0, 0, width - 1, height - 1);
        DrawLine(0, height - 1, width - 1, 0);
    }

    bool SaveBMP(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        int bytesPerPixel = is32Bit ? 4 : 3;
        int rowSize = (width * bytesPerPixel + 3) & ~3;
        int dataSize = rowSize * height;

        bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dataSize;
        bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        bmih.biSizeImage = dataSize;

        file.write(reinterpret_cast<char*>(&bmfh), sizeof(bmfh));
        file.write(reinterpret_cast<char*>(&bmih), sizeof(bmih));

        for (int y = height - 1; y >= 0; --y) {
            const uint8_t* row = pixelData.data() + y * width * bytesPerPixel;
            file.write(reinterpret_cast<const char*>(row), width * bytesPerPixel);

            if (!is32Bit) {
                int padding = (4 - (width * 3) % 4) % 4;
                if (padding > 0) {
                    char pad[3] = {0};
                    file.write(pad, padding);
                }
            }
        }

        file.close();
        return true;
    }
};

int main() {
    BMPProcessor processor;

    std::string inputFilename;
    std::cout << "Enter input BMP file name: ";
    std::cin >> inputFilename;

    if (!processor.ReadBMP(inputFilename)) {
        std::cerr << "Error reading BMP file.\n";
        return 1;
    }

    processor.DisplayToConsole();

    processor.DrawCross();

    std::cout << "\nAfter drawing cross:\n";
    processor.DisplayToConsole();

    std::string outputFilename;
    std::cout << "Enter output BMP file name: ";
    std::cin >> outputFilename;

    if (!processor.SaveBMP(outputFilename)) {
        std::cerr << "Error saving BMP file.\n";
        return 1;
    }

    return 0;
}