#include <iostream>
#include <iomanip>
#include <cmath>
#include <string>
#include <algorithm>
#include <vector>
#include <windows.h>
#include <limits>
#include <sstream>
#include <bitset>
#include <chrono>
#include <xmmintrin.h>

using namespace std;
using namespace std::chrono;

// ==================== UNION FOR FLOAT/INT CONVERSION ====================

union fu {
    float f;
    unsigned int u;
};

// ==================== BINARY CONVERSION METHODS ====================

// Простая функция для unsigned int -> binary (Пункт 0)
wstring decimalToBinary32(unsigned int n) {
    wstring binary;
    for (int i = 31; i >= 0; i--) {
        binary += (n & (1 << i)) ? L'1' : L'0';
        if (i % 8 == 0 && i != 0) binary += L" ";
    }
    return binary;
}

// Альтернативная простая версия как в задании
void printBinarySimple(unsigned int n) {
    for (int i = 31; i >= 0; i--) {
        wcout << ((n >> i) & 1);
        if (i % 8 == 0 && i != 0) wcout << L" ";
    }
    wcout << endl;
}

// ==================== FLOAT ANALYSIS FUNCTIONS ====================

wstring floatToBinaryIEEE754(float f) {
    fu converter;
    converter.f = f;
    return decimalToBinary32(converter.u);
}

struct FloatAnalysis {
    unsigned int sign;
    unsigned int exponent;
    unsigned int mantissa;
    wstring type;
    float actual_value;
    unsigned int raw_bits;
};

FloatAnalysis analyzeFloatStructure(float f) {
    fu converter;
    converter.f = f;
    unsigned int bits = converter.u;

    FloatAnalysis analysis;
    analysis.sign = (bits >> 31) & 1;
    analysis.exponent = (bits >> 23) & 0xFF;
    analysis.mantissa = bits & 0x7FFFFF;
    analysis.actual_value = f;
    analysis.raw_bits = bits;

    if (analysis.exponent == 0xFF) {
        analysis.type = (analysis.mantissa == 0) ? L"INFINITY" : L"NAN";
    } else if (analysis.exponent == 0) {
        analysis.type = (analysis.mantissa == 0) ? L"ZERO" : L"DENORMALIZED";
    } else {
        analysis.type = L"NORMALIZED";
    }

    return analysis;
}

// ==================== DENORMAL NUMBERS FUNCTIONS ====================

float generateDenormal() {
    float x = 1.0f;
    // Генерируем денормализованное число путем многократного деления
    for (int i = 0; i < 150; i++) { // Достаточно итераций для underflow
        x /= 2.0f;
        if (x == 0.0f) break;
    }
    return x;
}

double measureOperationTime(bool use_denormal, int iterations = 100000) {
    vector<float> numbers(1000);

    if (use_denormal) {
        float denormal = generateDenormal();
        for (int i = 0; i < 1000; i++) {
            numbers[i] = denormal * (i + 1);
        }
    } else {
        for (int i = 0; i < 1000; i++) {
            numbers[i] = 1.0f + i * 0.001f;
        }
    }

    auto start = high_resolution_clock::now();

    volatile float sum = 0.0f; // volatile чтобы компилятор не оптимизировал
    for (int i = 0; i < iterations; i++) {
        int idx1 = i % numbers.size();
        int idx2 = (i + 1) % numbers.size();
        sum += numbers[idx1] * numbers[idx2];
        sum += numbers[idx1] + numbers[idx2];
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);

    return duration.count() / 1000.0;
}

// ==================== MANTISSA OVERFLOW DEMONSTRATION ====================

void demonstrateMantissaOverflow() {
    wcout << L"\n=== MANTISSA OVERFLOW DEMONSTRATION (Point 2) ===\n\n";
    wcout << L"Showing powers of 10 and their binary representation:\n\n";

    wcout << fixed;
    wcout.precision(2);

    wcout << setw(15) << L"Decimal" << setw(40) << L"Binary" << setw(15) << L"Exponent" << setw(15) << L"Mantissa" << endl;
    wcout << wstring(85, L'-') << endl;

    float power = 1.0f;
    for (int i = 0; i <= 15; i++) {
        FloatAnalysis analysis = analyzeFloatStructure(power);

        wcout << setw(15) << power
             << setw(40) << floatToBinaryIEEE754(power)
             << setw(15) << (int)analysis.exponent - 127
             << setw(15) << analysis.mantissa << endl;

        power *= 10.0f;

        if (isinf(power)) {
            wcout << L"\n*** REACHED INFINITY at 10^" << i+1 << L" ***\n";
            break;
        }
    }
}

void demonstratePrecisionLoss() {
    wcout << L"\n=== PRECISION LOSS DEMONSTRATION ===\n\n";
    wcout << L"Showing how large numbers lose precision:\n\n";

    wcout << fixed;
    wcout.precision(0);

    wcout << setw(20) << L"Number" << setw(20) << L"Stored As" << setw(20) << L"Error" << endl;
    wcout << wstring(60, L'-') << endl;

    vector<unsigned int> test_numbers = {
        16777215, 16777216, 16777217, 16777218,
        100000000, 1000000000, 2000000000
    };

    for (unsigned int num : test_numbers) {
        float f = static_cast<float>(num);
        unsigned int recovered = static_cast<unsigned int>(f);
        int error = static_cast<int>(num) - static_cast<int>(recovered);

        wcout << setw(20) << num
             << setw(20) << recovered
             << setw(20) << error << endl;
    }
}

void demonstrateDiscreteNature() {
    wcout << L"\n=== DISCRETE NATURE OF FLOAT NUMBERS ===\n\n";
    wcout << L"Float can't represent all real numbers - only discrete set:\n\n";

    wcout << fixed;
    wcout.precision(10);

    vector<float> problematic_numbers = {0.1f, 0.2f, 0.3f, 0.4f, 0.6f, 0.7f, 0.8f, 0.9f};

    wcout << setw(15) << L"Decimal" << setw(25) << L"Actual stored value" << setw(20) << L"Error" << endl;
    wcout << wstring(60, L'-') << endl;

    for (float num : problematic_numbers) {
        double ideal = static_cast<double>(num);
        double actual = static_cast<double>(num);
        double error = actual - ideal;

        wcout << setw(15) << ideal
             << setw(25) << actual
             << setw(20) << scientific << error << fixed << endl;
    }
}

void demonstrateInfiniteLoop() {
    wcout << L"\n=== INFINITE LOOP DEMONSTRATION (Point 3) ===\n\n";
    wcout << L"This demonstrates how float discreteness can create infinite loops:\n\n";

    wcout << L"Example 1: Counting with large floats (will eventually stop due to precision loss)\n";
    wcout << L"Starting from 1e30, adding 1.0 each iteration:\n\n";

    float large_number = 1.0e30f;
    int iterations = 0;

    while (iterations < 100) {
        float next = large_number + 1.0f;
        if (next == large_number) {
            wcout << L"*** LOOP BROKEN: " << large_number << L" + 1 = " << next << L" (no change!)\n";
            wcout << L"*** Distance between floats at this magnitude is greater than 1\n";
            break;
        }
        large_number = next;
        iterations++;

        if (iterations % 20 == 0) {
            wcout << L"Iteration " << iterations << L": " << large_number << endl;
        }
    }

    wcout << L"\nExample 2: The classic infinite loop scenario\n";
    wcout << L"Starting from 16777216.0f (2^24), adding 1.0:\n\n";

    float x = 16777216.0f;
    wcout << L"Initial value: " << fixed << setprecision(1) << x << endl;
    wcout << L"x + 1 = " << (x + 1.0f) << endl;
    wcout << L"Notice: " << x << L" + 1 = " << (x + 1.0f) << L" (they are equal!)\n";
    wcout << L"This would cause an infinite loop: while (x < target) { x += 1; }\n";
}

void demonstrateClassicInfiniteLoop() {
    wcout << L"\n=== CLASSIC INFINITE LOOP DEMO ===\n\n";

    wcout << L"Classic infinite loop scenario:\n";
    wcout << L"for (float x = 16777216.0f; x < 16777218.0f; x += 1.0f)\n\n";

    wcout << L"Let's test step by step:\n";
    float x = 16777216.0f;
    wcout << L"x = " << fixed << x << endl;
    wcout << L"x + 1 = " << (x + 1.0f) << endl;
    wcout << L"Are they equal? " << ((x + 1.0f) == x ? L"YES - INFINITE LOOP!" : L"No") << endl;
}

// ==================== NUMERICAL INTEGRATION (Point 4) ====================

double function(double x) {
    return x * x;
}

double derivative(double x) {
    return 2 * x;
}

double antiderivative(double x) {
    return (x * x * x) / 3.0;
}

double analytical_integral() {
    return 1.0 / 3.0;
}

double integrate_rectangle_left(double a, double b, int n, bool use_double = true) {
    double sum = 0.0;
    double dx = (b - a) / n;

    if (use_double) {
        for (int i = 0; i < n; i++) {
            double x = a + i * dx;
            sum += function(x) * dx;
        }
    } else {
        float sum_f = 0.0f;
        float dx_f = (float)(b - a) / n;
        for (int i = 0; i < n; i++) {
            float x = (float)a + i * dx_f;
            sum_f += (float)function(x) * dx_f;
        }
        sum = sum_f;
    }
    return sum;
}

double integrate_rectangle_midpoint(double a, double b, int n, bool use_double = true) {
    double sum = 0.0;
    double dx = (b - a) / n;

    if (use_double) {
        for (int i = 0; i < n; i++) {
            double x = a + (i + 0.5) * dx;
            sum += function(x) * dx;
        }
    } else {
        float sum_f = 0.0f;
        float dx_f = (float)(b - a) / n;
        for (int i = 0; i < n; i++) {
            float x = (float)a + (i + 0.5f) * dx_f;
            sum_f += (float)function(x) * dx_f;
        }
        sum = sum_f;
    }
    return sum;
}

// ==================== SIMPLE UNION DEMO (Point 1) ====================

void demonstrateSimpleUnion() {
    wcout << L"\n=== SIMPLE UNION DEMO (Point 1) ===\n\n";

    fu converter;
    converter.f = 3.14f;

    wcout << L"Float value: " << converter.f << endl;
    wcout << L"As unsigned int: " << converter.u << endl;
    wcout << L"Binary: " << decimalToBinary32(converter.u) << endl;

    // Проверка на других значениях
    float test_values[] = {0.0f, 1.0f, -1.0f, 0.5f, 0.1f};
    for (float f : test_values) {
        converter.f = f;
        wcout << L"\n" << f << L" -> " << decimalToBinary32(converter.u);
    }
}

// ==================== UNSIGNED INT TO BINARY (Point 0) ====================

void demonstrateUnsignedIntToBinary() {
    wcout << L"\n=== UNSIGNED INT TO BINARY (Point 0) ===\n\n";

    vector<unsigned int> test_values = {0, 1, 255, 256, 65535, 16777215};

    wcout << L"Using simple bit operations:\n";
    for (unsigned int val : test_values) {
        wcout << L"Decimal: " << val << L" -> Binary: ";
        printBinarySimple(val);
    }

    wcout << L"\nUsing formatted function:\n";
    for (unsigned int val : test_values) {
        wcout << L"Decimal: " << val << L" -> Binary: " << decimalToBinary32(val) << endl;
    }
}

// ==================== GRAPHICAL WINDOW CLASS ====================

class FloatingPointLabWindow {
private:
    HWND hwnd;
    HDC hdc;
    int width, height;
    int margin;
    HFONT font_title, font_normal, font_small, font_large;
    int current_demo;
    float demo_float;
    unsigned int demo_int;
    wstring input_buffer;
    bool input_mode;
    bool show_help;
    int integration_steps;
    bool daz_ftz_enabled;

    void drawPixel(int x, int y, COLORREF color) {
        SetPixel(hdc, x, y, color);
    }

    void drawLine(int x1, int y1, int x2, int y2, COLORREF color) {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    void drawRect(int x, int y, int w, int h, COLORREF color) {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, x, y, x + w, y + h);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    void drawFilledRect(int x, int y, int w, int h, COLORREF color) {
        HBRUSH brush = CreateSolidBrush(color);
        RECT rect = {x, y, x + w, y + h};
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void drawText(int x, int y, const wstring& text, COLORREF color = RGB(255, 255, 255),
                  HFONT font = NULL, bool center = false) {
        SetTextColor(hdc, color);
        SetBkMode(hdc, TRANSPARENT);
        HFONT oldFont = (HFONT)SelectObject(hdc, font ? font : font_normal);

        if (center) {
            SIZE size;
            GetTextExtentPoint32W(hdc, text.c_str(), (int)text.length(), &size);
            x = (width - size.cx) / 2;
        }

        TextOutW(hdc, x, y, text.c_str(), (int)text.length());
        SelectObject(hdc, oldFont);
    }

    void drawPanel(int x, int y, int w, int h, const wstring& title,
                   COLORREF bgColor = RGB(45, 45, 50), COLORREF borderColor = RGB(80, 80, 90)) {
        // Рисуем фон панели
        drawFilledRect(x, y, w, h, bgColor);

        // Рамка
        drawRect(x, y, w, h, borderColor);

        // Заголовок
        if (!title.empty()) {
            drawText(x + 15, y + 12, title, RGB(220, 220, 255), font_normal);

            // Разделительная линия под заголовком
            drawLine(x + 10, y + 35, x + w - 10, y + 35, borderColor);
        }
    }

    void drawButton(int x, int y, int w, int h, const wstring& text, bool active = false) {
        COLORREF bgColor = active ? RGB(70, 100, 150) : RGB(60, 60, 70);
        COLORREF borderColor = active ? RGB(100, 150, 200) : RGB(90, 90, 100);

        drawFilledRect(x, y, w, h, bgColor);
        drawRect(x, y, w, h, borderColor);
        drawText(x + w/2, y + h/2 - 10, text, RGB(255, 255, 255), font_normal, true);
    }

    void drawBinaryVisualization(int x, int y, const wstring& binary, const wstring& title) {
        int panelWidth = min(width - 2 * margin, 1000);
        drawPanel(x, y, panelWidth, 140, title);

        wstring clean_binary = binary;
        clean_binary.erase(remove(clean_binary.begin(), clean_binary.end(), L' '), clean_binary.end());

        int bitSize = min(20, (panelWidth - 40) / 33);
        int totalWidth = (int)clean_binary.length() * bitSize;
        int startX = x + (panelWidth - totalWidth) / 2;

        // Подписи для частей числа
        drawText(startX, y + 40, L"Sign", RGB(255, 100, 100), font_small);
        drawText(startX + 32, y + 40, L"Exponent", RGB(100, 255, 100), font_small);
        drawText(startX + 32*9, y + 40, L"Mantissa", RGB(100, 100, 255), font_small);

        for (size_t i = 0; i < clean_binary.length(); i++) {
            COLORREF bitColor;
            if (i == 0) {
                bitColor = RGB(255, 100, 100); // Sign
            } else if (i >= 1 && i <= 8) {
                bitColor = RGB(100, 255, 100); // Exponent
            } else {
                bitColor = RGB(100, 100, 255); // Mantissa
            }

            if (clean_binary[i] == L'0') {
                bitColor = RGB(GetRValue(bitColor) / 2, GetGValue(bitColor) / 2, GetBValue(bitColor) / 2);
            }

            drawFilledRect(startX + (int)i * bitSize, y + 55, bitSize - 2, bitSize - 2, bitColor);

            wstring bitStr(1, clean_binary[i]);
            drawText(startX + (int)i * bitSize + (bitSize/2 - 4), y + 57, bitStr,
                    clean_binary[i] == L'1' ? RGB(255, 255, 255) : RGB(200, 200, 200), font_small);
        }

        // Отображаем биты с пробелами как в оригинальном формате
        drawText(x + 15, y + 85, L"Full binary: " + binary, RGB(200, 200, 255), font_small);
    }

    void drawCoordinateSystem(int x, int y, int w, int h, double xMin, double xMax, double yMin, double yMax, const wstring& title) {
        drawPanel(x, y, w, h, title);

        int graphX = x + 50;
        int graphY = y + 50;
        int graphWidth = w - 70;
        int graphHeight = h - 70;

        // Оси
        drawLine(graphX, graphY + graphHeight, graphX + graphWidth, graphY + graphHeight, RGB(200, 200, 200));
        drawLine(graphX, graphY, graphX, graphY + graphHeight, RGB(200, 200, 200));

        // Подписи осей
        drawText(graphX - 10, graphY + graphHeight + 10, L"0", RGB(200, 200, 200), font_small);
        drawText(graphX + graphWidth - 10, graphY + graphHeight + 10, L"1", RGB(200, 200, 200), font_small);
        drawText(graphX - 30, graphY, L"1", RGB(200, 200, 200), font_small);
    }

    void drawFunctionGraph(int x, int y, int w, int h, double xMin, double xMax, double yMin, double yMax,
                          double (*func)(double), COLORREF color, const wstring& label) {
        int graphX = x + 50;
        int graphY = y + 50;
        int graphWidth = w - 70;
        int graphHeight = h - 70;

        HPEN pen = CreatePen(PS_SOLID, 2, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        bool first = true;
        for (int px = 0; px < graphWidth; px++) {
            double xVal = xMin + (xMax - xMin) * px / graphWidth;
            double yVal = func(xVal);

            int py = graphY + graphHeight - (int)((yVal - yMin) / (yMax - yMin) * graphHeight);

            if (py < graphY) py = graphY;
            if (py > graphY + graphHeight) py = graphY + graphHeight;

            if (first) {
                MoveToEx(hdc, graphX + px, py, NULL);
                first = false;
            } else {
                LineTo(hdc, graphX + px, py);
            }
        }

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    void drawIntegrationVisualization(int x, int y, int w, int h, int steps) {
        drawPanel(x, y, w, h, L"Integration Visualization - Rectangle Method");

        int graphX = x + 50;
        int graphY = y + 50;
        int graphWidth = w - 70;
        int graphHeight = h - 70;

        double xMin = 0, xMax = 1;
        double yMin = 0, yMax = 1;

        // Функция
        HPEN funcPen = CreatePen(PS_SOLID, 3, RGB(0, 200, 255));
        HPEN oldPen = (HPEN)SelectObject(hdc, funcPen);

        bool first = true;
        for (int px = 0; px < graphWidth; px++) {
            double xVal = xMin + (xMax - xMin) * px / graphWidth;
            double yVal = function(xVal);
            int py = graphY + graphHeight - (int)((yVal - yMin) / (yMax - yMin) * graphHeight);

            if (first) {
                MoveToEx(hdc, graphX + px, py, NULL);
                first = false;
            } else {
                LineTo(hdc, graphX + px, py);
            }
        }
        SelectObject(hdc, oldPen);
        DeleteObject(funcPen);

        // Прямоугольники
        double dx = 1.0 / steps;
        HBRUSH rectBrush = CreateSolidBrush(RGB(255, 100, 100));
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, rectBrush);

        for (int i = 0; i < steps; i++) {
            double xLeft = i * dx;
            double yVal = function(xLeft);

            int rectX = graphX + (int)(xLeft * graphWidth);
            int rectWidth = (int)(dx * graphWidth);
            int rectY = graphY + graphHeight - (int)((yVal - yMin) / (yMax - yMin) * graphHeight);

            RECT rect = {rectX, rectY, rectX + rectWidth, graphY + graphHeight};
            FillRect(hdc, &rect, rectBrush);
        }

        SelectObject(hdc, oldBrush);
        DeleteObject(rectBrush);

        // Рамка графика
        drawRect(graphX, graphY, graphWidth, graphHeight, RGB(150, 150, 150));

        wstring info = L"Steps: " + to_wstring(steps) + L", Function: y = x², Interval: [0, 1]";
        drawText(x + 15, y + h - 25, info, RGB(200, 200, 255), font_small);
    }

public:
    FloatingPointLabWindow(int w = 1400, int h = 900) : margin(20),
                                                   current_demo(0), demo_float(0.0f), demo_int(0),
                                                   input_mode(false), show_help(false), integration_steps(10),
                                                   daz_ftz_enabled(false) {
        RECT workArea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

        width = workArea.right - workArea.left;
        height = workArea.bottom - workArea.top;

        WNDCLASSA wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "FloatLab";
        wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

        RegisterClassA(&wc);

        hwnd = CreateWindowA(
            "FloatLab",
            "Floating Point Laboratory - Interactive Visualization System",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE,
            workArea.left, workArea.top,
            width, height,
            NULL, NULL, GetModuleHandleA(NULL), this
        );

        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        UpdateWindow(hwnd);
        hdc = GetDC(hwnd);

        font_large = CreateFontA(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");

        font_title = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");

        font_normal = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");

        font_small = CreateFontA(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
    }

    ~FloatingPointLabWindow() {
        DeleteObject(font_large);
        DeleteObject(font_title);
        DeleteObject(font_normal);
        DeleteObject(font_small);
        ReleaseDC(hwnd, hdc);
    }

    void setDemo(int demo, float f_val = 0.0f, unsigned int i_val = 0) {
        current_demo = demo;
        if (demo == 1 || demo == 3 || demo == 4 || demo == 5 || demo == 6 || demo == 7) demo_float = f_val;
        if (demo == 2) demo_int = i_val;
        input_mode = false;
        input_buffer.clear();
        redraw();
    }

    void startInputMode() {
        input_mode = true;
        input_buffer.clear();
        redraw();
    }

    void processInput(wchar_t c) {
        if (c == L'\r') {
            if (!input_buffer.empty()) {
                try {
                    if (current_demo == 6) {
                        integration_steps = stoi(input_buffer);
                        if (integration_steps < 1) integration_steps = 1;
                        if (integration_steps > 1000) integration_steps = 1000;
                    } else if (current_demo == 1 || current_demo == 3 || current_demo == 4 || current_demo == 5 || current_demo == 7) {
                        demo_float = stof(input_buffer);
                    } else if (current_demo == 2) {
                        demo_int = stoul(input_buffer);
                    }
                } catch (...) {
                }
            }
            input_mode = false;
        } else if (c == L'\b') {
            if (!input_buffer.empty()) {
                input_buffer.pop_back();
            }
        } else if ((c >= L'0' && c <= L'9') || c == L'.' || c == L'-' || c == L'+') {
            if (input_buffer.length() < 20) {
                input_buffer += c;
            }
        }
        redraw();
    }

    void toggleHelp() {
        show_help = !show_help;
        redraw();
    }

    void toggleDAZFTZ() {
        daz_ftz_enabled = !daz_ftz_enabled;
        if (daz_ftz_enabled) {
            _mm_setcsr(_mm_getcsr() | 0x8040);
        } else {
            _mm_setcsr(_mm_getcsr() & ~0x8040);
        }
        redraw();
    }

private:
    void clear(COLORREF color = RGB(35, 35, 40)) {
        RECT rect = {0, 0, width, height};
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    void drawHeader(const wstring& title) {
        // Верхняя панель
        drawFilledRect(0, 0, width, 70, RGB(50, 50, 60));
        drawLine(0, 70, width, 70, RGB(80, 80, 100));

        drawText(0, 20, title, RGB(255, 255, 255), font_large, true);

        // Информация о текущем демо
        wstring demo_info = L"Demo " + to_wstring(current_demo) + L" - Press M for Main Menu";
        drawText(width - 300, 25, demo_info, RGB(200, 200, 255), font_small);
    }

    void drawInputPanel() {
        int panelY = 90;
        drawPanel(margin, panelY, width - 2 * margin, 70, L"Input Control");

        if (input_mode) {
            wstring input_text = L"Enter value: " + input_buffer + L"_";
            drawText(margin + 20, panelY + 45, input_text, RGB(255, 255, 100), font_normal);
            drawText(margin + 20, panelY + 75, L"Press ENTER to confirm, ESC to cancel", RGB(200, 200, 200), font_small);
        } else {
            wstring value_text;
            if (current_demo == 2) {
                value_text = L"Current value: " + to_wstring(demo_int);
            } else {
                value_text = L"Current value: " + to_wstring(demo_float);
            }
            drawText(margin + 20, panelY + 45, value_text, RGB(200, 255, 200), font_normal);
            drawText(margin + 400, panelY + 45, L"Press I to input new value", RGB(255, 255, 100), font_normal);
        }
    }

    void drawControlsPanel() {
        int controlsY = height - 80;
        drawPanel(margin, controlsY, width - 2 * margin, 60, L"Controls");

        vector<wstring> controls = {
            L"I - Input Value",
            L"M - Main Menu",
            L"H - Help",
            L"ESC - Exit"
        };

        if (current_demo == 7) {
            controls.push_back(L"D - Toggle DAZ/FTZ");
        }

        int controlWidth = (width - 2 * margin - 40) / controls.size();
        for (size_t i = 0; i < controls.size(); i++) {
            drawText(margin + 20 + (int)i * controlWidth, controlsY + 35,
                    controls[i], RGB(255, 255, 100), font_small);
        }
    }

    void drawFloatDemo() {
        clear();
        drawHeader(L"FLOAT TO BINARY - IEEE 754 ANALYSIS");
        drawInputPanel();

        int contentY = 180;

        // Бинарное представление
        wstring binary = floatToBinaryIEEE754(demo_float);
        drawBinaryVisualization(margin, contentY, binary, L"IEEE 754 Binary Representation");

        // Детальный анализ
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        int analysisY = contentY + 160;
        drawPanel(margin, analysisY, width - 2 * margin, 180, L"Detailed Analysis");

        int infoX = margin + 20;
        int columnWidth = (width - 2 * margin - 60) / 2;

        // Левая колонка
        drawText(infoX, analysisY + 45, L"VALUE: " + to_wstring(analysis.actual_value),
                RGB(255, 255, 255), font_normal);
        drawText(infoX, analysisY + 75, L"TYPE: " + analysis.type,
                analysis.type == L"NORMALIZED" ? RGB(100, 255, 100) :
                analysis.type == L"DENORMALIZED" ? RGB(255, 200, 100) : RGB(255, 100, 100), font_normal);
        drawText(infoX, analysisY + 105, L"SIGN: " + to_wstring(analysis.sign) + L" (" +
                (analysis.sign ? L"NEGATIVE" : L"POSITIVE") + L")",
                analysis.sign ? RGB(255, 100, 100) : RGB(100, 255, 100), font_normal);

        // Правая колонка
        drawText(infoX + columnWidth, analysisY + 45, L"EXPONENT: " + to_wstring(analysis.exponent) +
                L" (raw) = " + to_wstring((int)analysis.exponent - 127) + L" (actual)",
                RGB(100, 255, 100), font_normal);
        drawText(infoX + columnWidth, analysisY + 75, L"MANTISSA: " + to_wstring(analysis.mantissa),
                RGB(100, 100, 255), font_normal);
        drawText(infoX + columnWidth, analysisY + 105, L"RAW BITS: " + to_wstring(analysis.raw_bits),
                RGB(200, 200, 255), font_normal);

        // Примеры быстрого тестирования
        int examplesY = analysisY + 200;
        drawPanel(margin, examplesY, width - 2 * margin, 80, L"Quick Test Values");

        vector<wstring> examples = {L"0.0", L"1.0", L"-1.0", L"0.5", L"0.1", L"3.14", L"1e10", L"1e-10"};
        int exampleSpacing = min(120, (width - 2 * margin - 40) / 8);
        for (size_t i = 0; i < examples.size(); i++) {
            drawText(margin + 20 + (int)i * exampleSpacing, examplesY + 45,
                    examples[i], RGB(200, 255, 200), font_small);
        }

        drawControlsPanel();
    }

    void drawIntegerDemo() {
        clear();
        drawHeader(L"INTEGER TO BINARY CONVERSION");
        drawInputPanel();

        int contentY = 180;

        // Бинарное представление
        wstring binary = decimalToBinary32(demo_int);
        drawBinaryVisualization(margin, contentY, binary, L"32-bit Binary Representation");

        // Анализ
        int analysisY = contentY + 160;
        drawPanel(margin, analysisY, width - 2 * margin, 120, L"Analysis");

        drawText(margin + 20, analysisY + 45, L"DECIMAL: " + to_wstring(demo_int),
                RGB(255, 255, 255), font_normal);

        wstring hex_str;
        wstringstream hex_ss;
        hex_ss << hex << uppercase << demo_int;
        hex_ss >> hex_str;
        drawText(margin + 20, analysisY + 75, L"HEXADECIMAL: 0x" + hex_str,
                RGB(200, 200, 255), font_normal);

        // Примеры быстрого тестирования
        int examplesY = analysisY + 150;
        drawPanel(margin, examplesY, width - 2 * margin, 80, L"Quick Test Values");

        vector<wstring> examples = {L"0", L"1", L"255", L"1024", L"65535", L"16777215", L"2147483647"};
        int exampleSpacing = min(150, (width - 2 * margin - 40) / 7);
        for (size_t i = 0; i < examples.size(); i++) {
            drawText(margin + 20 + (int)i * exampleSpacing, examplesY + 45,
                    examples[i], RGB(200, 255, 200), font_small);
        }

        drawControlsPanel();
    }

    void drawPrecisionDemo() {
        clear();
        drawHeader(L"FLOATING POINT PRECISION ANALYSIS");
        drawInputPanel();

        int contentY = 180;

        // Анализ текущего значения
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        drawPanel(margin, contentY, width - 2 * margin, 100, L"Current Value Analysis");

        drawText(margin + 20, contentY + 45, L"Value: " + to_wstring(demo_float),
                RGB(255, 255, 255), font_normal);
        drawText(margin + 20, contentY + 75, L"Type: " + analysis.type,
                analysis.type == L"NORMALIZED" ? RGB(100, 255, 100) : RGB(255, 200, 100), font_normal);

        // Классические проблемы точности
        int problemsY = contentY + 120;
        drawPanel(margin, problemsY, width - 2 * margin, 150, L"Classic Precision Problems");

        // Проблема 0.1 + 0.2
        float f1 = 0.1f, f2 = 0.2f;
        float sum1 = f1 + f2;
        bool equal1 = (sum1 == 0.3f);

        drawText(margin + 20, problemsY + 45, L"Problem 1: 0.1 + 0.2", RGB(255, 255, 255), font_normal);
        wstring result1 = L"Result: " + to_wstring(sum1) + L" (expected: 0.3)";
        drawText(margin + 20, problemsY + 75, result1,
                equal1 ? RGB(100, 255, 100) : RGB(255, 100, 100), font_normal);

        wstring equal_str = equal1 ? L"✓ EQUAL [CORRECT]" : L"✗ NOT EQUAL [PRECISION ERROR]";
        drawText(margin + 400, problemsY + 45, equal_str,
                equal1 ? RGB(100, 255, 100) : RGB(255, 100, 100), font_normal);

        // Накопление ошибки
        drawText(margin + 20, problemsY + 105, L"Problem 2: Error Accumulation", RGB(255, 255, 255), font_normal);
        float accumulated = 0.0f;
        for (int i = 0; i < 100; i++) {
            accumulated += demo_float;
        }
        float expected = demo_float * 100.0f;
        float error = fabs(accumulated - expected);

        wstring accum_text = L"100 × " + to_wstring(demo_float) + L" = " + to_wstring(accumulated);
        drawText(margin + 20, problemsY + 135, accum_text, RGB(200, 200, 255), font_small);

        wstring error_text = L"Error: " + to_wstring(error);
        drawText(margin + 400, problemsY + 105, error_text,
                error < 0.001f ? RGB(100, 255, 100) : RGB(255, 100, 100), font_normal);

        // Решения и лучшие практики
        int solutionsY = problemsY + 180;
        drawPanel(margin, solutionsY, width - 2 * margin, 130, L"Solutions & Best Practices");

        vector<wstring> solutions = {
            L"• Use double instead of float for critical calculations",
            L"• Compare floats with tolerance: fabs(a-b) < 1e-6",
            L"• Avoid subtracting numbers of similar magnitude",
            L"• Use Kahan summation for accurate accumulation",
            L"• Be aware of catastrophic cancellation"
        };

        for (size_t i = 0; i < solutions.size(); i++) {
            drawText(margin + 20, solutionsY + 45 + (int)i * 20, solutions[i], RGB(200, 255, 200), font_small);
        }

        drawControlsPanel();
    }

    void drawMantissaOverflowDemo() {
        clear();
        drawHeader(L"MANTISSA OVERFLOW & DISCRETE NATURE OF FLOAT");
        drawInputPanel();

        int contentY = 180;

        // Анализ текущего значения
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        drawPanel(margin, contentY, width - 2 * margin, 100, L"Current Value Analysis");

        drawText(margin + 20, contentY + 45, L"Value: " + to_wstring(demo_float), RGB(255, 255, 255), font_normal);
        drawText(margin + 20, contentY + 75, L"Exponent: " + to_wstring((int)analysis.exponent - 127),
                RGB(100, 255, 100), font_normal);
        drawText(margin + 400, contentY + 45, L"Mantissa: " + to_wstring(analysis.mantissa),
                RGB(100, 100, 255), font_normal);

        // Бинарное представление
        wstring binary = floatToBinaryIEEE754(demo_float);
        drawBinaryVisualization(margin, contentY + 120, binary, L"Binary Representation");

        // Прогрессия степеней
        int progressionY = contentY + 280;
        drawPanel(margin, progressionY, width - 2 * margin, 200, L"Powers Progression - Mantissa Overflow");

        float power = demo_float;
        int tableY = progressionY + 45;

        // Заголовки таблицы
        drawText(margin + 20, tableY, L"Step", RGB(255, 255, 100), font_small);
        drawText(margin + 80, tableY, L"Value", RGB(255, 255, 100), font_small);
        drawText(margin + 250, tableY, L"Exponent", RGB(255, 255, 100), font_small);
        drawText(margin + 350, tableY, L"Mantissa", RGB(255, 255, 100), font_small);
        drawText(margin + 480, tableY, L"Status", RGB(255, 255, 100), font_small);

        tableY += 25;

        for (int i = 0; i < 8; i++) {
            if (isinf(power) || isnan(power)) break;

            FloatAnalysis power_analysis = analyzeFloatStructure(power);
            wstring status = L"Normal";
            COLORREF statusColor = RGB(100, 255, 100);

            if (power_analysis.exponent >= 0xFF) {
                status = L"INFINITY";
                statusColor = RGB(255, 100, 100);
            } else if (power_analysis.mantissa == 0x7FFFFF) {
                status = L"Mantissa MAX";
                statusColor = RGB(255, 200, 100);
            } else if (power_analysis.type == L"DENORMALIZED") {
                status = L"Denormal";
                statusColor = RGB(255, 150, 50);
            }

            wstring power_str = to_wstring(power);
            if (power_str.length() > 15) {
                power_str = power_str.substr(0, 15) + L"...";
            }

            drawText(margin + 20, tableY, to_wstring(i), RGB(200, 200, 255), font_small);
            drawText(margin + 80, tableY, power_str, RGB(255, 255, 255), font_small);
            drawText(margin + 250, tableY, to_wstring((int)power_analysis.exponent - 127),
                    RGB(100, 255, 100), font_small);
            drawText(margin + 350, tableY, to_wstring(power_analysis.mantissa),
                    RGB(100, 100, 255), font_small);
            drawText(margin + 480, tableY, status, statusColor, font_small);

            tableY += 20;
            power *= 10.0f;
        }

        // Ключевые инсайты
        int insightsY = progressionY + 220;
        drawPanel(margin, insightsY, width - 2 * margin, 120, L"Key Insights - Discrete Nature of Float");

        vector<wstring> insights = {
            L"• Float represents DISCRETE set of numbers, not continuous real numbers",
            L"• Many decimal numbers cannot be represented exactly in binary",
            L"• Binary tails are silently truncated - no warnings from compiler",
            L"• All floating-point calculations have some error - unavoidable in digital systems"
        };

        for (size_t i = 0; i < insights.size(); i++) {
            drawText(margin + 20, insightsY + 45 + (int)i * 20, insights[i], RGB(255, 200, 100), font_small);
        }

        drawControlsPanel();
    }

    void drawInfiniteLoopDemo() {
        clear();
        drawHeader(L"INFINITE LOOP DEMONSTRATION");
        drawInputPanel();

        int contentY = 180;

        // Анализ текущего значения
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        drawPanel(margin, contentY, width - 2 * margin, 100, L"Current Value Analysis");

        drawText(margin + 20, contentY + 45, L"Value: " + to_wstring(demo_float), RGB(255, 255, 255), font_normal);
        drawText(margin + 20, contentY + 75, L"Exponent: " + to_wstring((int)analysis.exponent - 127),
                RGB(100, 255, 100), font_normal);
        drawText(margin + 400, contentY + 45, L"Mantissa: " + to_wstring(analysis.mantissa),
                RGB(100, 100, 255), font_normal);

        // Симуляция бесконечного цикла
        int simulationY = contentY + 120;
        drawPanel(margin, simulationY, width - 2 * margin, 120, L"Infinite Loop Simulation");

        drawText(margin + 20, simulationY + 45, L"Testing: x = " + to_wstring(demo_float) + L" + 1.0",
                RGB(255, 255, 255), font_normal);

        float next_value = demo_float + 1.0f;
        bool will_loop = (next_value == demo_float);

        wstring result_text = L"Result: " + to_wstring(demo_float) + L" + 1 = " + to_wstring(next_value);
        drawText(margin + 20, simulationY + 75, result_text,
                will_loop ? RGB(255, 100, 100) : RGB(100, 255, 100), font_normal);

        if (will_loop) {
            drawText(margin + 20, simulationY + 105, L"*** INFINITE LOOP DETECTED! ***",
                    RGB(255, 50, 50), font_normal);
            drawText(margin + 400, simulationY + 45, L"LOOP CONDITION: x == x + 1",
                    RGB(255, 100, 100), font_normal);
        } else {
            drawText(margin + 20, simulationY + 105, L"Values are different - loop would work normally",
                    RGB(100, 255, 100), font_normal);
            drawText(margin + 400, simulationY + 45, L"LOOP CONDITION: x ≠ x + 1",
                    RGB(100, 255, 100), font_normal);
        }

        // Классические примеры бесконечных циклов
        int examplesY = simulationY + 150;
        drawPanel(margin, examplesY, width - 2 * margin, 150, L"Classic Infinite Loop Examples");

        vector<wstring> code_examples = {
            L"// DANGEROUS: Potential infinite loop",
            L"float x = 16777216.0f;  // 2^24",
            L"while (x < 16777218.0f) {",
            L"    x += 1.0f;  // Never changes beyond this point!",
            L"    // INFINITE LOOP!",
            L"}"
        };

        for (size_t i = 0; i < code_examples.size(); i++) {
            drawText(margin + 20, examplesY + 45 + (int)i * 18, code_examples[i],
                    i == 0 ? RGB(255, 100, 100) : RGB(200, 200, 255), font_small);
        }

        // Объяснение
        int explanationY = examplesY + 170;
        drawPanel(margin, explanationY, width - 2 * margin, 100, L"Why This Happens");

        vector<wstring> explanations = {
            L"• Float values become sparse at large magnitudes",
            L"• Distance between adjacent floats > 1 at 2^24 (16,777,216)",
            L"• Adding 1.0 doesn't change the value - no progression in loop",
            L"• Loop condition never becomes false"
        };

        for (size_t i = 0; i < explanations.size(); i++) {
            drawText(margin + 20, explanationY + 45 + (int)i * 20, explanations[i],
                    RGB(255, 200, 100), font_small);
        }

        drawControlsPanel();
    }

    void drawIntegrationDemo() {
        clear();
        drawHeader(L"NUMERICAL INTEGRATION DEMONSTRATION");

        // Специальная панель ввода для интеграции
        int inputY = 90;
        drawPanel(margin, inputY, width - 2 * margin, 70, L"Integration Parameters");
        if (input_mode) {
            wstring input_text = L"Enter number of steps (1-1000): " + input_buffer + L"_";
            drawText(margin + 20, inputY + 45, input_text, RGB(255, 255, 100), font_normal);
        } else {
            wstring display_value = L"Current steps: " + to_wstring(integration_steps) +
                                   L" | Function: y = x² | Interval: [0, 1] | Press I to change steps";
            drawText(margin + 20, inputY + 45, display_value, RGB(200, 255, 200), font_normal);
        }

        int contentY = 180;

        // Визуализация интегрирования
        int graphHeight = 300;
        drawIntegrationVisualization(margin, contentY, width - 2 * margin, graphHeight, integration_steps);

        // Результаты интегрирования
        int resultsY = contentY + graphHeight + 20;
        drawPanel(margin, resultsY, width - 2 * margin, 180, L"Integration Results");

        double analytical = analytical_integral();
        double rect_left_double = integrate_rectangle_left(0, 1, integration_steps, true);
        double rect_mid_double = integrate_rectangle_midpoint(0, 1, integration_steps, true);
        double rect_left_float = integrate_rectangle_left(0, 1, integration_steps, false);
        double rect_mid_float = integrate_rectangle_midpoint(0, 1, integration_steps, false);

        int textY = resultsY + 45;
        drawText(margin + 20, textY, L"Analytical result: ∫₀¹ x² dx = 1/3 = " + to_wstring(analytical),
                RGB(255, 255, 255), font_normal);

        // Сравнение double vs float
        int column1 = margin + 20;
        int column2 = margin + width/2;

        drawText(column1, textY + 35, L"Double precision:", RGB(100, 255, 100), font_normal);
        drawText(column1, textY + 60, L"  Left rectangles: " + to_wstring(rect_left_double) +
                L" (error: " + to_wstring(fabs(rect_left_double - analytical)) + L")",
                RGB(200, 255, 200), font_small);
        drawText(column1, textY + 85, L"  Midpoint rule:  " + to_wstring(rect_mid_double) +
                L" (error: " + to_wstring(fabs(rect_mid_double - analytical)) + L")",
                RGB(200, 255, 200), font_small);

        drawText(column2, textY + 35, L"Float precision:", RGB(255, 200, 100), font_normal);
        drawText(column2, textY + 60, L"  Left rectangles: " + to_wstring(rect_left_float) +
                L" (error: " + to_wstring(fabs(rect_left_float - analytical)) + L")",
                RGB(255, 255, 200), font_small);
        drawText(column2, textY + 85, L"  Midpoint rule:  " + to_wstring(rect_mid_float) +
                L" (error: " + to_wstring(fabs(rect_mid_float - analytical)) + L")",
                RGB(255, 255, 200), font_small);

        // Сравнение ошибок
        double float_error = fabs(rect_left_float - analytical);
        double double_error = fabs(rect_left_double - analytical);
        double error_ratio = (double_error > 0) ? float_error / double_error : 1.0;

        wstring comparison = L"Float error / Double error = " + to_wstring(error_ratio);
        drawText(margin + 20, textY + 115, comparison,
                 (error_ratio > 10) ? RGB(255, 100, 100) :
                 (error_ratio > 2) ? RGB(255, 200, 100) : RGB(100, 255, 100), font_small);

        // Графики функций
        int graphsY = resultsY + 200;
        int graphPanelHeight = 250;
        drawPanel(margin, graphsY, width - 2 * margin, graphPanelHeight, L"Function Analysis: y = x²");

        int graphWidth = (width - 2 * margin - 40) / 3;

        // Функция
        drawCoordinateSystem(margin + 20, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 1, L"Function: y = x²");
        drawFunctionGraph(margin + 20, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 2, function,
                         RGB(0, 200, 255), L"y = x²");
        drawText(margin + 20 + graphWidth/2, graphsY + 40, L"y = x²", RGB(0, 200, 255), font_normal, true);

        // Производная
        drawCoordinateSystem(margin + 30 + graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 2, L"Derivative: y' = 2x");
        drawFunctionGraph(margin + 30 + graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 2, derivative,
                         RGB(255, 100, 100), L"y' = 2x");
        drawText(margin + 30 + graphWidth + graphWidth/2, graphsY + 40, L"y' = 2x", RGB(255, 100, 100), font_normal, true);

        // Первообразная
        drawCoordinateSystem(margin + 40 + 2 * graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 0.4, L"Antiderivative: ∫x²dx = x³/3");
        drawFunctionGraph(margin + 40 + 2 * graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 0.4, antiderivative,
                         RGB(100, 255, 100), L"∫x²dx = x³/3");
        drawText(margin + 40 + 2 * graphWidth + graphWidth/2, graphsY + 40, L"∫x²dx = x³/3", RGB(100, 255, 100), font_normal, true);

        drawControlsPanel();
    }

    void drawUnderflowDemo() {
        clear();
        drawHeader(L"UNDERFLOW & DENORMAL NUMBERS DEMONSTRATION");
        drawInputPanel();

        int contentY = 180;

        // Статус DAZ/FTZ
        drawPanel(margin, contentY, width - 2 * margin, 70, L"DAZ/FTZ Control");
        wstring mode_status = daz_ftz_enabled ? L"ENABLED (DAZ/FTZ ON)" : L"DISABLED (DAZ/FTZ OFF)";
        drawText(margin + 20, contentY + 45, L"Current mode: " + mode_status,
                daz_ftz_enabled ? RGB(255, 100, 100) : RGB(100, 255, 100), font_normal);
        drawText(margin + 400, contentY + 45, L"Press D to toggle DAZ/FTZ mode",
                RGB(255, 255, 100), font_normal);

        // Генерация денормализованных чисел
        int denormalY = contentY + 90;
        drawPanel(margin, denormalY, width - 2 * margin, 180, L"Denormal Numbers Examples");

        vector<float> denormal_examples;
        vector<wstring> denormal_info;

        float current = demo_float;
        for (int i = 0; i < 4; i++) {
            float test_value = current;
            for (int j = 0; i < 3 && j < 140 + i * 5; j++) {
                test_value /= 2.0f;
                if (test_value == 0.0f) break;
            }

            if (test_value == 0.0f) {
                denormal_examples.push_back(0.0f);
                denormal_info.push_back(L"0.0 | Type: ZERO | Underflow to zero");
            } else {
                denormal_examples.push_back(test_value);
                FloatAnalysis analysis = analyzeFloatStructure(test_value);
                wstring info = to_wstring(test_value) + L" | Type: " + analysis.type +
                            L" | Exp: " + to_wstring(analysis.exponent) +
                            L" | Mantissa: " + to_wstring(analysis.mantissa);
                denormal_info.push_back(info);
            }
        }

        int exampleY = denormalY + 45;
        for (size_t i = 0; i < denormal_examples.size(); i++) {
            wstring example_text = L"Denormal " + to_wstring(i+1) + L": " + denormal_info[i];
            drawText(margin + 20, exampleY, example_text,
                    denormal_examples[i] == 0.0f ? RGB(255, 100, 100) : RGB(255, 200, 100), font_small);
            exampleY += 25;
        }

        // Бинарное представление первого денормала
        if (!denormal_examples.empty() && denormal_examples[0] != 0.0f) {
            wstring binary = floatToBinaryIEEE754(denormal_examples[0]);
            drawText(margin + 20, exampleY, L"Binary of Denormal 1: " + binary,
                    RGB(200, 200, 255), font_small);
        }

        // Сравнение производительности
        int performanceY = denormalY + 200;
        drawPanel(margin, performanceY, width - 2 * margin, 120, L"Performance Comparison");

        double normal_time = 1.0, denormal_time = 5.0; // Значения по умолчанию
        try {
            normal_time = measureOperationTime(false, 50000);
            denormal_time = measureOperationTime(true, 50000);
        } catch (...) {}

        drawText(margin + 20, performanceY + 45, L"Normal numbers time: " + to_wstring(normal_time) + L" ms",
                RGB(100, 255, 100), font_normal);
        drawText(margin + 20, performanceY + 75, L"Denormal numbers time: " + to_wstring(denormal_time) + L" ms",
                RGB(255, 100, 100), font_normal);

        double slowdown = 0.0;
        if (normal_time > 0) {
            slowdown = (denormal_time / normal_time - 1.0) * 100.0;
        }
        wstring slowdown_text = L"Performance slowdown: " + to_wstring(slowdown) + L"%";
        drawText(margin + 400, performanceY + 45, slowdown_text,
                slowdown > 100 ? RGB(255, 50, 50) :
                slowdown > 50 ? RGB(255, 150, 50) : RGB(255, 200, 100), font_normal);

        // Режимы DAZ/FTZ
        int modesY = performanceY + 140;
        drawPanel(margin, modesY, width - 2 * margin, 100, L"DAZ vs FTZ Modes");

        vector<wstring> explanations = {
            L"• DAZ (Denormals Are Zero): Denormal inputs treated as zero",
            L"• FTZ (Flush To Zero): Denormal results flushed to zero",
            L"• Both improve performance but reduce precision",
            L"• Used in graphics/games where speed > accuracy"
        };

        for (size_t i = 0; i < explanations.size(); i++) {
            drawText(margin + 20, modesY + 45 + (int)i * 20, explanations[i], RGB(200, 200, 255), font_small);
        }

        // Влияние на точность
        int precisionY = modesY + 120;
        drawPanel(margin, precisionY, width - 2 * margin, 100, L"Precision Impact with DAZ/FTZ");

        float small_sum = 0.0f;
        float increment = demo_float != 0.0f ? demo_float / 1e10f : 1e-38f;

        for (int i = 0; i < 100; i++) {
            small_sum += increment;
        }

        drawText(margin + 20, precisionY + 45, L"Sum of 100 × " + to_wstring(increment) + L": " + to_wstring(small_sum),
                RGB(255, 255, 255), font_normal);

        if (daz_ftz_enabled && small_sum == 0.0f) {
            drawText(margin + 20, precisionY + 75, L"*** PRECISION LOST: DAZ/FTZ flushed results to zero ***",
                    RGB(255, 100, 100), font_normal);
        } else if (small_sum > 0.0f) {
            drawText(margin + 20, precisionY + 75, L"✓ Precision preserved (small result accumulated)",
                    RGB(100, 255, 100), font_normal);
        } else {
            drawText(margin + 20, precisionY + 75, L"Underflow to zero (normal behavior)",
                    RGB(255, 200, 100), font_normal);
        }

        // Ключевые инсайты
        int insightsY = precisionY + 120;
        drawPanel(margin, insightsY, width - 2 * margin, 80, L"Key Insights");

        vector<wstring> insights = {
            L"• Denormals allow gradual underflow but are SLOW",
            L"• DAZ/FTZ improve performance but sacrifice precision",
            L"• Choose based on application: science (keep) vs games (disable)"
        };

        for (size_t i = 0; i < insights.size(); i++) {
            drawText(margin + 20, insightsY + 45 + (int)i * 20, insights[i], RGB(255, 200, 100), font_small);
        }

        drawControlsPanel();
    }

    void drawMainMenu() {
        clear();

        // Заголовок
        drawFilledRect(0, 0, width, 120, RGB(50, 50, 60));
        drawText(0, 30, L"FLOATING POINT LABORATORY", RGB(255, 255, 255), font_large, true);
        drawText(0, 75, L"Interactive Visualization System", RGB(200, 200, 255), font_title, true);

        int startY = 150;
        int panelWidth = (width - 3 * margin) / 2;
        int panelHeight = 100;

        COLORREF panelColors[] = {
            RGB(65, 80, 100),   // Float Analysis
            RGB(80, 100, 65),   // Integer Analysis
            RGB(100, 80, 65),   // Precision Issues
            RGB(80, 65, 100),   // Mantissa Overflow
            RGB(100, 65, 80),   // Infinite Loop
            RGB(65, 100, 80),   // Numerical Integration
            RGB(100, 80, 80)    // Underflow & Denormals
        };

        wstring titles[] = {
            L"1 - Float Analysis",
            L"2 - Integer Analysis",
            L"3 - Precision Issues",
            L"4 - Mantissa Overflow",
            L"5 - Infinite Loop",
            L"6 - Numerical Integration",
            L"7 - Underflow & Denormals"
        };

        wstring descriptions[] = {
            L"Float to Binary conversion\nIEEE 754 structure analysis",
            L"Integer to Binary conversion\n32-bit representation",
            L"Precision problems analysis\n0.1 + 0.2 demonstration",
            L"Mantissa overflow visualization\nPowers of 10 progression",
            L"Infinite loop demonstration\nFloat discreteness issues",
            L"Numerical integration methods\nRectangle method visualization",
            L"Underflow and denormal numbers\nDAZ/FTZ modes comparison"
        };

        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 2; col++) {
                int demoNum = row * 2 + col + 1;
                if (demoNum > 7) continue;

                int x = margin + col * (panelWidth + margin);
                int y = startY + row * (panelHeight + 15);

                drawPanel(x, y, panelWidth, panelHeight, titles[demoNum-1], panelColors[demoNum-1]);

                vector<wstring> lines;
                wstringstream ss(descriptions[demoNum-1]);
                wstring line;
                while (getline(ss, line, L'\n')) {
                    lines.push_back(line);
                }

                for (size_t i = 0; i < lines.size(); i++) {
                    drawText(x + 20, y + 45 + (int)i * 22, lines[i],
                            i == 0 ? RGB(255, 255, 255) : RGB(220, 220, 255),
                            i == 0 ? font_normal : font_small);
                }
            }
        }

        // Панель управления
        int controlsY = startY + 4 * (panelHeight + 15) + 20;
        drawPanel(margin, controlsY, width - 2 * margin, 100, L"Controls", RGB(70, 70, 80));

        vector<wstring> controlLines = {
            L"1-7 - Switch between demonstration modules",
            L"I - Input values (when in demonstration mode)",
            L"M - Return to main menu (from any demo)",
            L"ESC - Exit program"
        };

        for (size_t i = 0; i < controlLines.size(); i++) {
            drawText(margin + 20, controlsY + 35 + (int)i * 20, controlLines[i],
                    RGB(255, 255, 100), font_normal);
        }

        // Нижняя информационная панель
        drawText(margin, height - 30,
                L"Press number keys 1-7 to start exploring floating point arithmetic! | Use I key to input values in demonstrations",
                RGB(150, 150, 200), font_small, true);
    }

    void redraw() {
        switch (current_demo) {
            case 0: drawMainMenu(); break;
            case 1: drawFloatDemo(); break;
            case 2: drawIntegerDemo(); break;
            case 3: drawPrecisionDemo(); break;
            case 4: drawMantissaOverflowDemo(); break;
            case 5: drawInfiniteLoopDemo(); break;
            case 6: drawIntegrationDemo(); break;
            case 7: drawUnderflowDemo(); break;
        }
    }

public:
    void update() {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void run() {
        setDemo(0);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void handleKey(int key) {
        if (input_mode) {
            if (key == VK_RETURN) {
                processInput(L'\r');
            } else if (key == VK_BACK) {
                processInput(L'\b');
            } else if (key == VK_ESCAPE) {
                input_mode = false;
                redraw();
            }
        } else {
            switch (key) {
                case '1': setDemo(1, 3.14f); break;
                case '2': setDemo(2, 0, 255); break;
                case '3': setDemo(3, 0.1f); break;
                case '4': setDemo(4, 1.0f); break;
                case '5': setDemo(5, 16777216.0f); break;
                case '6': setDemo(6, 0.0f); break;
                case '7': setDemo(7, 1.0f); break;
                case 'I': case 'i': startInputMode(); break;
                case 'M': case 'm': setDemo(0); break;
                case 'D': case 'd':
                    if (current_demo == 7) toggleDAZFTZ();
                    break;
                case 'H': case 'h': toggleHelp(); break;
                case VK_ESCAPE: PostQuitMessage(0); break;
            }
        }
    }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        FloatingPointLabWindow* window = nullptr;

        if (uMsg == WM_NCCREATE) {
            CREATESTRUCT* create = reinterpret_cast<CREATESTRUCT*>(lParam);
            window = reinterpret_cast<FloatingPointLabWindow*>(create->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        } else {
            window = reinterpret_cast<FloatingPointLabWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        }

        if (window) {
            switch (uMsg) {
                case WM_PAINT: {
                    PAINTSTRUCT ps;
                    BeginPaint(hwnd, &ps);
                    window->redraw();
                    EndPaint(hwnd, &ps);
                    return 0;
                }
                case WM_CHAR:
                    if (window->input_mode) {
                        window->processInput((wchar_t)wParam);
                    }
                    return 0;
                case WM_KEYDOWN:
                    window->handleKey(static_cast<int>(wParam));
                    return 0;
                case WM_DESTROY:
                    PostQuitMessage(0);
                    return 0;
                case WM_SIZE: {
                    RECT rect;
                    GetClientRect(hwnd, &rect);
                    window->width = rect.right - rect.left;
                    window->height = rect.bottom - rect.top;
                    window->redraw();
                    return 0;
                }
            }
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};

// ==================== MAIN PROGRAM ====================

int main() {
    SetConsoleOutputCP(CP_UTF8);

    wcout << L"Floating Point Laboratory - Interactive Demo System\n";
    wcout << L"===================================================\n\n";

    // Простые консольные демонстрации как в задании
    demonstrateUnsignedIntToBinary();  // Пункт 0
    demonstrateSimpleUnion();          // Пункт 1
    demonstrateMantissaOverflow();     // Пункт 2
    demonstrateClassicInfiniteLoop();  // Пункт 3
    demonstratePrecisionLoss();
    demonstrateDiscreteNature();

    wcout << L"\nStarting graphical interface...\n";
    wcout << L"Controls:\n";
    wcout << L"  1 - Float to Binary Demo\n";
    wcout << L"  2 - Integer to Binary Demo\n";
    wcout << L"  3 - Precision Issues Demo\n";
    wcout << L"  4 - Mantissa Overflow Demo\n";
    wcout << L"  5 - Infinite Loop Demo\n";
    wcout << L"  6 - Numerical Integration Demo\n";
    wcout << L"  7 - Underflow & Denormals Demo\n";
    wcout << L"  I - Input new value (when in demo)\n";
    wcout << L"  M - Return to main menu\n";
    wcout << L"  D - Toggle DAZ/FTZ (in demo 7)\n";
    wcout << L"  H - Toggle help\n";
    wcout << L"  ESC - Exit\n\n";

    FloatingPointLabWindow window;
    window.run();

    wcout << L"Demo completed. Thank you!\n";
    return 0;
}
