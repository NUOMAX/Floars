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

// ==================== ОБЪЕДИНЕНИЕ ДЛЯ ПРЕОБРАЗОВАНИЯ FLOAT/INT ====================

// Объединение для преобразования между float и int
union fu {
    float f;
    unsigned int u;
};

// ==================== МЕТОДЫ ПРЕОБРАЗОВАНИЯ В ДВОИЧНЫЙ ФОРМАТ ====================

// Функция для преобразования unsigned int в двоичный формат (Пункт 0)
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

// ==================== ФУНКЦИИ АНАЛИЗА FLOAT ====================

// Преобразование float в двоичный формат IEEE 754
wstring floatToBinaryIEEE754(float f) {
    fu converter;
    converter.f = f;
    return decimalToBinary32(converter.u);
}

// Структура для анализа числа с плавающей точкой
struct FloatAnalysis {
    unsigned int sign;        // Знак
    unsigned int exponent;    // Экспонента
    unsigned int mantissa;    // Мантисса
    wstring type;             // Тип числа
    float actual_value;       // Фактическое значение
    unsigned int raw_bits;    // Исходные биты
};

// Анализ структуры числа с плавающей точкой
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

    // Определение типа числа
    if (analysis.exponent == 0xFF) {
        analysis.type = (analysis.mantissa == 0) ? L"БЕСКОНЕЧНОСТЬ" : L"НЕ_ЧИСЛО";
    } else if (analysis.exponent == 0) {
        analysis.type = (analysis.mantissa == 0) ? L"НУЛЬ" : L"ДЕНОРМАЛИЗОВАННОЕ";
    } else {
        analysis.type = L"НОРМАЛИЗОВАННОЕ";
    }

    return analysis;
}

// ==================== ФУНКЦИИ ДЛЯ ДЕНОРМАЛИЗОВАННЫХ ЧИСЕЛ ====================

// Генерация денормализованного числа
float generateDenormal() {
    float x = 1.0f;
    // Генерация денормализованного числа путем многократного деления
    for (int i = 0; i < 150; i++) {
        x /= 2.0f;
        if (x == 0.0f) break;
    }
    return x;
}

// Измерение времени выполнения операций
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

// ==================== ДЕМОНСТРАЦИЯ ПЕРЕПОЛНЕНИЯ МАНТИССЫ ====================

// Демонстрация переполнения мантиссы (Пункт 2)
void demonstrateMantissaOverflow() {
    wcout << L"\n=== ДЕМОНСТРАЦИЯ ПЕРЕПОЛНЕНИЯ МАНТИССЫ (Пункт 2) ===\n\n";
    wcout << L"Показаны степени 10 и их двоичное представление:\n\n";

    wcout << fixed;
    wcout.precision(2);

    wcout << setw(15) << L"Десятичное" << setw(40) << L"Двоичное" << setw(15) << L"Экспонента" << setw(15) << L"Мантисса" << endl;
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
            wcout << L"\n*** ДОСТИГНУТА БЕСКОНЕЧНОСТЬ при 10^" << i+1 << L" ***\n";
            break;
        }
    }
}

// Демонстрация потери точности
void demonstratePrecisionLoss() {
    wcout << L"\n=== ДЕМОНСТРАЦИЯ ПОТЕРИ ТОЧНОСТИ ===\n\n";
    wcout << L"Показано, как большие числа теряют точность:\n\n";

    wcout << fixed;
    wcout.precision(0);

    wcout << setw(20) << L"Число" << setw(20) << L"Хранится как" << setw(20) << L"Ошибка" << endl;
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

// Демонстрация дискретной природы чисел с плавающей точкой
void demonstrateDiscreteNature() {
    wcout << L"\n=== ДИСКРЕТНАЯ ПРИРОДА ЧИСЕЛ С ПЛАВАЮЩЕЙ ТОЧКОЙ ===\n\n";
    wcout << L"Float не может представить все вещественные числа - только дискретное множество:\n\n";

    wcout << fixed;
    wcout.precision(10);

    vector<float> problematic_numbers = {0.1f, 0.2f, 0.3f, 0.4f, 0.6f, 0.7f, 0.8f, 0.9f};

    wcout << setw(15) << L"Десятичное" << setw(25) << L"Фактическое значение" << setw(20) << L"Ошибка" << endl;
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

// Демонстрация бесконечного цикла (Пункт 3)
void demonstrateInfiniteLoop() {
    wcout << L"\n=== ДЕМОНСТРАЦИЯ БЕСКОНЕЧНОГО ЦИКЛА (Пункт 3) ===\n\n";
    wcout << L"Эта демонстрация показывает, как дискретность float может создавать бесконечные циклы:\n\n";

    wcout << L"Пример 1: Подсчет с большими числами float (в конечном итоге остановится из-за потери точности)\n";
    wcout << L"Начиная с 1e30, добавляя 1.0 на каждой итерации:\n\n";

    float large_number = 1.0e30f;
    int iterations = 0;

    while (iterations < 100) {
        float next = large_number + 1.0f;
        if (next == large_number) {
            wcout << L"*** ЦИКЛ ПРЕРВАН: " << large_number << L" + 1 = " << next << L" (без изменений!)\n";
            wcout << L"*** Расстояние между числами float при этой величине больше 1\n";
            break;
        }
        large_number = next;
        iterations++;

        if (iterations % 20 == 0) {
            wcout << L"Итерация " << iterations << L": " << large_number << endl;
        }
    }

    wcout << L"\nПример 2: Классический сценарий бесконечного цикла\n";
    wcout << L"Начиная с 16777216.0f (2^24), добавляя 1.0:\n\n";

    float x = 16777216.0f;
    wcout << L"Начальное значение: " << fixed << setprecision(1) << x << endl;
    wcout << L"x + 1 = " << (x + 1.0f) << endl;
    wcout << L"Заметьте: " << x << L" + 1 = " << (x + 1.0f) << L" (они равны!)\n";
    wcout << L"Это вызовет бесконечный цикл: while (x < target) { x += 1; }\n";
}

// Демонстрация классического бесконечного цикла
void demonstrateClassicInfiniteLoop() {
    wcout << L"\n=== ДЕМОНСТРАЦИЯ КЛАССИЧЕСКОГО БЕСКОНЕЧНОГО ЦИКЛА ===\n\n";

    wcout << L"Классический сценарий бесконечного цикла:\n";
    wcout << L"for (float x = 16777216.0f; x < 16777218.0f; x += 1.0f)\n\n";

    wcout << L"Проверим шаг за шагом:\n";
    float x = 16777216.0f;
    wcout << L"x = " << fixed << x << endl;
    wcout << L"x + 1 = " << (x + 1.0f) << endl;
    wcout << L"Они равны? " << ((x + 1.0f) == x ? L"ДА - БЕСКОНЕЧНЫЙ ЦИКЛ!" : L"Нет") << endl;
}

// ==================== ЧИСЛЕННОЕ ИНТЕГРИРОВАНИЕ (Пункт 4) ====================

// Функция для интегрирования: f(x) = x²
double function(double x) {
    return x * x;
}

// Производная функции
double derivative(double x) {
    return 2 * x;
}

// Первообразная функции
double antiderivative(double x) {
    return (x * x * x) / 3.0;
}

// Аналитическое значение интеграла
double analytical_integral() {
    return 1.0 / 3.0;
}

// Интегрирование методом левых прямоугольников
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

// Интегрирование методом средних прямоугольников
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

// ==================== ПРОСТАЯ ДЕМОНСТРАЦИЯ UNION (Пункт 1) ====================

// Демонстрация простого объединения (Пункт 1)
void demonstrateSimpleUnion() {
    wcout << L"\n=== ПРОСТАЯ ДЕМОНСТРАЦИЯ UNION (Пункт 1) ===\n\n";

    fu converter;
    converter.f = 3.14f;

    wcout << L"Значение float: " << converter.f << endl;
    wcout << L"Как unsigned int: " << converter.u << endl;
    wcout << L"Двоичное: " << decimalToBinary32(converter.u) << endl;

    // Проверка на других значениях
    float test_values[] = {0.0f, 1.0f, -1.0f, 0.5f, 0.1f};
    for (float f : test_values) {
        converter.f = f;
        wcout << L"\n" << f << L" -> " << decimalToBinary32(converter.u);
    }
}

// ==================== ПРЕОБРАЗОВАНИЕ UNSIGNED INT В ДВОИЧНЫЙ (Пункт 0) ====================

// Демонстрация преобразования unsigned int в двоичный формат (Пункт 0)
void demonstrateUnsignedIntToBinary() {
    wcout << L"\n=== ПРЕОБРАЗОВАНИЕ UNSIGNED INT В ДВОИЧНЫЙ (Пункт 0) ===\n\n";

    vector<unsigned int> test_values = {0, 1, 255, 256, 65535, 16777215};

    wcout << L"Использование простых битовых операций:\n";
    for (unsigned int val : test_values) {
        wcout << L"Десятичное: " << val << L" -> Двоичное: ";
        printBinarySimple(val);
    }

    wcout << L"\nИспользование форматированной функции:\n";
    for (unsigned int val : test_values) {
        wcout << L"Десятичное: " << val << L" -> Двоичное: " << decimalToBinary32(val) << endl;
    }
}

// ==================== КЛАСС ГРАФИЧЕСКОГО ОКНА ====================

// Класс для графического интерфейса лаборатории
class FloatingPointLabWindow {
private:
    HWND hwnd;              // Дескриптор окна
    HDC hdc;                // Контекст устройства
    int width, height;      // Размеры окна
    int margin;             // Отступы
    HFONT font_title, font_normal, font_small, font_large; // Шрифты
    int current_demo;       // Текущая демонстрация
    float demo_float;       // Демонстрационное значение float
    unsigned int demo_int;  // Демонстрационное значение int
    wstring input_buffer;   // Буфер ввода
    bool input_mode;        // Режим ввода
    bool show_help;         // Показать справку
    int integration_steps;  // Количество шагов интегрирования
    bool daz_ftz_enabled;   // Включены ли DAZ/FTZ

    // Рисование пикселя
    void drawPixel(int x, int y, COLORREF color) {
        SetPixel(hdc, x, y, color);
    }

    // Рисование линии
    void drawLine(int x1, int y1, int x2, int y2, COLORREF color) {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);

        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);

        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    // Рисование прямоугольника
    void drawRect(int x, int y, int w, int h, COLORREF color) {
        HPEN pen = CreatePen(PS_SOLID, 1, color);
        HPEN oldPen = (HPEN)SelectObject(hdc, pen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));

        Rectangle(hdc, x, y, x + w, y + h);

        SelectObject(hdc, oldBrush);
        SelectObject(hdc, oldPen);
        DeleteObject(pen);
    }

    // Рисование закрашенного прямоугольника
    void drawFilledRect(int x, int y, int w, int h, COLORREF color) {
        HBRUSH brush = CreateSolidBrush(color);
        RECT rect = {x, y, x + w, y + h};
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    // Рисование текста
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

    // Рисование панели
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

    // Рисование кнопки
    void drawButton(int x, int y, int w, int h, const wstring& text, bool active = false) {
        COLORREF bgColor = active ? RGB(70, 100, 150) : RGB(60, 60, 70);
        COLORREF borderColor = active ? RGB(100, 150, 200) : RGB(90, 90, 100);

        drawFilledRect(x, y, w, h, bgColor);
        drawRect(x, y, w, h, borderColor);
        drawText(x + w/2, y + h/2 - 10, text, RGB(255, 255, 255), font_normal, true);
    }

    // Визуализация двоичного представления
    void drawBinaryVisualization(int x, int y, const wstring& binary, const wstring& title) {
        int panelWidth = min(width - 2 * margin, 1000);
        drawPanel(x, y, panelWidth, 140, title);

        wstring clean_binary = binary;
        clean_binary.erase(remove(clean_binary.begin(), clean_binary.end(), L' '), clean_binary.end());

        int bitSize = min(20, (panelWidth - 40) / 33);
        int totalWidth = (int)clean_binary.length() * bitSize;
        int startX = x + (panelWidth - totalWidth) / 2;

        // Подписи для частей числа
        drawText(startX, y + 40, L"Знак", RGB(255, 100, 100), font_small);
        drawText(startX + 32, y + 40, L"Экспонента", RGB(100, 255, 100), font_small);
        drawText(startX + 32*9, y + 40, L"Мантисса", RGB(100, 100, 255), font_small);

        for (size_t i = 0; i < clean_binary.length(); i++) {
            COLORREF bitColor;
            if (i == 0) {
                bitColor = RGB(255, 100, 100); // Знак
            } else if (i >= 1 && i <= 8) {
                bitColor = RGB(100, 255, 100); // Экспонента
            } else {
                bitColor = RGB(100, 100, 255); // Мантисса
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
        drawText(x + 15, y + 85, L"Полное двоичное: " + binary, RGB(200, 200, 255), font_small);
    }

    // Рисование системы координат
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

    // Рисование графика функции
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

    // Визуализация интегрирования
    void drawIntegrationVisualization(int x, int y, int w, int h, int steps) {
        drawPanel(x, y, w, h, L"Визуализация интегрирования - Метод прямоугольников");

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

        wstring info = L"Шаги: " + to_wstring(steps) + L", Функция: y = x², Интервал: [0, 1]";
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
            "Лаборатория чисел с плавающей точкой - Интерактивная система визуализации",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE,
            workArea.left, workArea.top,
            width, height,
            NULL, NULL, GetModuleHandleA(NULL), this
        );

        ShowWindow(hwnd, SW_SHOWMAXIMIZED);
        UpdateWindow(hwnd);
        hdc = GetDC(hwnd);

        // Создание шрифтов
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

    // Установка текущей демонстрации
    void setDemo(int demo, float f_val = 0.0f, unsigned int i_val = 0) {
        current_demo = demo;
        if (demo == 1 || demo == 3 || demo == 4 || demo == 5 || demo == 6 || demo == 7) demo_float = f_val;
        if (demo == 2) demo_int = i_val;
        input_mode = false;
        input_buffer.clear();
        redraw();
    }

    // Начало режима ввода
    void startInputMode() {
        input_mode = true;
        input_buffer.clear();
        redraw();
    }

    // Обработка ввода
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

    // Переключение справки
    void toggleHelp() {
        show_help = !show_help;
        redraw();
    }

    // Переключение режимов DAZ/FTZ
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
    // Очистка экрана
    void clear(COLORREF color = RGB(35, 35, 40)) {
        RECT rect = {0, 0, width, height};
        HBRUSH brush = CreateSolidBrush(color);
        FillRect(hdc, &rect, brush);
        DeleteObject(brush);
    }

    // Рисование заголовка
    void drawHeader(const wstring& title) {
        // Верхняя панель
        drawFilledRect(0, 0, width, 70, RGB(50, 50, 60));
        drawLine(0, 70, width, 70, RGB(80, 80, 100));

        drawText(0, 20, title, RGB(255, 255, 255), font_large, true);

        // Информация о текущем демо
        wstring demo_info = L"Демо " + to_wstring(current_demo) + L" - Нажмите M для главного меню";
        drawText(width - 300, 25, demo_info, RGB(200, 200, 255), font_small);
    }

    // Рисование панели ввода
    void drawInputPanel() {
        int panelY = 90;
        drawPanel(margin, panelY, width - 2 * margin, 70, L"Управление вводом");

        if (input_mode) {
            wstring input_text = L"Введите значение: " + input_buffer + L"_";
            drawText(margin + 20, panelY + 45, input_text, RGB(255, 255, 100), font_normal);
            drawText(margin + 20, panelY + 75, L"Нажмите ENTER для подтверждения, ESC для отмены", RGB(200, 200, 200), font_small);
        } else {
            wstring value_text;
            if (current_demo == 2) {
                value_text = L"Текущее значение: " + to_wstring(demo_int);
            } else {
                value_text = L"Текущее значение: " + to_wstring(demo_float);
            }
            drawText(margin + 20, panelY + 45, value_text, RGB(200, 255, 200), font_normal);
            drawText(margin + 400, panelY + 45, L"Нажмите I для ввода нового значения", RGB(255, 255, 100), font_normal);
        }
    }

    // Рисование панели управления
    void drawControlsPanel() {
        int controlsY = height - 80;
        drawPanel(margin, controlsY, width - 2 * margin, 60, L"Управление");

        vector<wstring> controls = {
            L"I - Ввод значения",
            L"M - Главное меню",
            L"H - Справка",
            L"ESC - Выход"
        };

        if (current_demo == 7) {
            controls.push_back(L"D - Переключить DAZ/FTZ");
        }

        int controlWidth = (width - 2 * margin - 40) / controls.size();
        for (size_t i = 0; i < controls.size(); i++) {
            drawText(margin + 20 + (int)i * controlWidth, controlsY + 35,
                    controls[i], RGB(255, 255, 100), font_small);
        }
    }

    // Демонстрация анализа float
    void drawFloatDemo() {
        clear();
        drawHeader(L"АНАЛИЗ FLOAT В ДВОИЧНЫЙ ФОРМАТ - IEEE 754");
        drawInputPanel();

        int contentY = 180;

        // Бинарное представление
        wstring binary = floatToBinaryIEEE754(demo_float);
        drawBinaryVisualization(margin, contentY, binary, L"Двоичное представление IEEE 754");

        // Детальный анализ
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        int analysisY = contentY + 160;
        drawPanel(margin, analysisY, width - 2 * margin, 180, L"Детальный анализ");

        int infoX = margin + 20;
        int columnWidth = (width - 2 * margin - 60) / 2;

        // Левая колонка
        drawText(infoX, analysisY + 45, L"ЗНАЧЕНИЕ: " + to_wstring(analysis.actual_value),
                RGB(255, 255, 255), font_normal);
        drawText(infoX, analysisY + 75, L"ТИП: " + analysis.type,
                analysis.type == L"НОРМАЛИЗОВАННОЕ" ? RGB(100, 255, 100) :
                analysis.type == L"ДЕНОРМАЛИЗОВАННОЕ" ? RGB(255, 200, 100) : RGB(255, 100, 100), font_normal);
        drawText(infoX, analysisY + 105, L"ЗНАК: " + to_wstring(analysis.sign) + L" (" +
                (analysis.sign ? L"ОТРИЦАТЕЛЬНЫЙ" : L"ПОЛОЖИТЕЛЬНЫЙ") + L")",
                analysis.sign ? RGB(255, 100, 100) : RGB(100, 255, 100), font_normal);

        // Правая колонка
        drawText(infoX + columnWidth, analysisY + 45, L"ЭКСПОНЕНТА: " + to_wstring(analysis.exponent) +
                L" (сырая) = " + to_wstring((int)analysis.exponent - 127) + L" (фактическая)",
                RGB(100, 255, 100), font_normal);
        drawText(infoX + columnWidth, analysisY + 75, L"МАНТИССА: " + to_wstring(analysis.mantissa),
                RGB(100, 100, 255), font_normal);
        drawText(infoX + columnWidth, analysisY + 105, L"СЫРЫЕ БИТЫ: " + to_wstring(analysis.raw_bits),
                RGB(200, 200, 255), font_normal);

        // Примеры быстрого тестирования
        int examplesY = analysisY + 200;
        drawPanel(margin, examplesY, width - 2 * margin, 80, L"Быстрые тестовые значения");

        vector<wstring> examples = {L"0.0", L"1.0", L"-1.0", L"0.5", L"0.1", L"3.14", L"1e10", L"1e-10"};
        int exampleSpacing = min(120, (width - 2 * margin - 40) / 8);
        for (size_t i = 0; i < examples.size(); i++) {
            drawText(margin + 20 + (int)i * exampleSpacing, examplesY + 45,
                    examples[i], RGB(200, 255, 200), font_small);
        }

        drawControlsPanel();
    }

    // Демонстрация анализа integer
    void drawIntegerDemo() {
        clear();
        drawHeader(L"ПРЕОБРАЗОВАНИЕ INTEGER В ДВОИЧНЫЙ ФОРМАТ");
        drawInputPanel();

        int contentY = 180;

        // Бинарное представление
        wstring binary = decimalToBinary32(demo_int);
        drawBinaryVisualization(margin, contentY, binary, L"32-битное двоичное представление");

        // Анализ
        int analysisY = contentY + 160;
        drawPanel(margin, analysisY, width - 2 * margin, 120, L"Анализ");

        drawText(margin + 20, analysisY + 45, L"ДЕСЯТИЧНОЕ: " + to_wstring(demo_int),
                RGB(255, 255, 255), font_normal);

        wstring hex_str;
        wstringstream hex_ss;
        hex_ss << hex << uppercase << demo_int;
        hex_ss >> hex_str;
        drawText(margin + 20, analysisY + 75, L"ШЕСТНАДЦАТЕРИЧНОЕ: 0x" + hex_str,
                RGB(200, 200, 255), font_normal);

        // Примеры быстрого тестирования
        int examplesY = analysisY + 150;
        drawPanel(margin, examplesY, width - 2 * margin, 80, L"Быстрые тестовые значения");

        vector<wstring> examples = {L"0", L"1", L"255", L"1024", L"65535", L"16777215", L"2147483647"};
        int exampleSpacing = min(150, (width - 2 * margin - 40) / 7);
        for (size_t i = 0; i < examples.size(); i++) {
            drawText(margin + 20 + (int)i * exampleSpacing, examplesY + 45,
                    examples[i], RGB(200, 255, 200), font_small);
        }

        drawControlsPanel();
    }

    // Демонстрация проблем с точностью
    void drawPrecisionDemo() {
        clear();
        drawHeader(L"АНАЛИЗ ТОЧНОСТИ ЧИСЕЛ С ПЛАВАЮЩЕЙ ТОЧКОЙ");
        drawInputPanel();

        int contentY = 180;

        // Анализ текущего значения
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        drawPanel(margin, contentY, width - 2 * margin, 100, L"Анализ текущего значения");

        drawText(margin + 20, contentY + 45, L"Значение: " + to_wstring(demo_float),
                RGB(255, 255, 255), font_normal);
        drawText(margin + 20, contentY + 75, L"Тип: " + analysis.type,
                analysis.type == L"НОРМАЛИЗОВАННОЕ" ? RGB(100, 255, 100) : RGB(255, 200, 100), font_normal);

        // Классические проблемы точности
        int problemsY = contentY + 120;
        drawPanel(margin, problemsY, width - 2 * margin, 150, L"Классические проблемы точности");

        // Проблема 0.1 + 0.2
        float f1 = 0.1f, f2 = 0.2f;
        float sum1 = f1 + f2;
        bool equal1 = (sum1 == 0.3f);

        drawText(margin + 20, problemsY + 45, L"Проблема 1: 0.1 + 0.2", RGB(255, 255, 255), font_normal);
        wstring result1 = L"Результат: " + to_wstring(sum1) + L" (ожидалось: 0.3)";
        drawText(margin + 20, problemsY + 75, result1,
                equal1 ? RGB(100, 255, 100) : RGB(255, 100, 100), font_normal);

        wstring equal_str = equal1 ? L"✓ РАВНЫ [КОРРЕКТНО]" : L"✗ НЕ РАВНЫ [ОШИБКА ТОЧНОСТИ]";
        drawText(margin + 400, problemsY + 45, equal_str,
                equal1 ? RGB(100, 255, 100) : RGB(255, 100, 100), font_normal);

        // Накопление ошибки
        drawText(margin + 20, problemsY + 105, L"Проблема 2: Накопление ошибки", RGB(255, 255, 255), font_normal);
        float accumulated = 0.0f;
        for (int i = 0; i < 100; i++) {
            accumulated += demo_float;
        }
        float expected = demo_float * 100.0f;
        float error = fabs(accumulated - expected);

        wstring accum_text = L"100 × " + to_wstring(demo_float) + L" = " + to_wstring(accumulated);
        drawText(margin + 20, problemsY + 135, accum_text, RGB(200, 200, 255), font_small);

        wstring error_text = L"Ошибка: " + to_wstring(error);
        drawText(margin + 400, problemsY + 105, error_text,
                error < 0.001f ? RGB(100, 255, 100) : RGB(255, 100, 100), font_normal);

        // Решения и лучшие практики
        int solutionsY = problemsY + 180;
        drawPanel(margin, solutionsY, width - 2 * margin, 130, L"Решения и лучшие практики");

        vector<wstring> solutions = {
            L"• Используйте double вместо float для критических вычислений",
            L"• Сравнивайте float с допуском: fabs(a-b) < 1e-6",
            L"• Избегайте вычитания чисел схожей величины",
            L"• Используйте суммирование Кахана для точного накопления",
            L"• Будьте осторожны с катастрофической потерей точности"
        };

        for (size_t i = 0; i < solutions.size(); i++) {
            drawText(margin + 20, solutionsY + 45 + (int)i * 20, solutions[i], RGB(200, 255, 200), font_small);
        }

        drawControlsPanel();
    }

    // Демонстрация переполнения мантиссы
    void drawMantissaOverflowDemo() {
        clear();
        drawHeader(L"ПЕРЕПОЛНЕНИЕ МАНТИССЫ И ДИСКРЕТНАЯ ПРИРОДА FLOAT");
        drawInputPanel();

        int contentY = 180;

        // Анализ текущего значения
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        drawPanel(margin, contentY, width - 2 * margin, 100, L"Анализ текущего значения");

        drawText(margin + 20, contentY + 45, L"Значение: " + to_wstring(demo_float), RGB(255, 255, 255), font_normal);
        drawText(margin + 20, contentY + 75, L"Экспонента: " + to_wstring((int)analysis.exponent - 127),
                RGB(100, 255, 100), font_normal);
        drawText(margin + 400, contentY + 45, L"Мантисса: " + to_wstring(analysis.mantissa),
                RGB(100, 100, 255), font_normal);

        // Бинарное представление
        wstring binary = floatToBinaryIEEE754(demo_float);
        drawBinaryVisualization(margin, contentY + 120, binary, L"Двоичное представление");

        // Прогрессия степеней
        int progressionY = contentY + 280;
        drawPanel(margin, progressionY, width - 2 * margin, 200, L"Прогрессия степеней - Переполнение мантиссы");

        float power = demo_float;
        int tableY = progressionY + 45;

        // Заголовки таблицы
        drawText(margin + 20, tableY, L"Шаг", RGB(255, 255, 100), font_small);
        drawText(margin + 80, tableY, L"Значение", RGB(255, 255, 100), font_small);
        drawText(margin + 250, tableY, L"Экспонента", RGB(255, 255, 100), font_small);
        drawText(margin + 350, tableY, L"Мантисса", RGB(255, 255, 100), font_small);
        drawText(margin + 480, tableY, L"Статус", RGB(255, 255, 100), font_small);

        tableY += 25;

        for (int i = 0; i < 8; i++) {
            if (isinf(power) || isnan(power)) break;

            FloatAnalysis power_analysis = analyzeFloatStructure(power);
            wstring status = L"Нормальный";
            COLORREF statusColor = RGB(100, 255, 100);

            if (power_analysis.exponent >= 0xFF) {
                status = L"БЕСКОНЕЧНОСТЬ";
                statusColor = RGB(255, 100, 100);
            } else if (power_analysis.mantissa == 0x7FFFFF) {
                status = L"Мантисса МАКС";
                statusColor = RGB(255, 200, 100);
            } else if (power_analysis.type == L"ДЕНОРМАЛИЗОВАННОЕ") {
                status = L"Денормал";
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
        drawPanel(margin, insightsY, width - 2 * margin, 120, L"Ключевые инсайты - Дискретная природа Float");

        vector<wstring> insights = {
            L"• Float представляет ДИСКРЕТНОЕ множество чисел, не непрерывные вещественные числа",
            L"• Многие десятичные числа не могут быть точно представлены в двоичном формате",
            L"• Двоичные хвосты молча обрезаются - нет предупреждений от компилятора",
            L"• Все вычисления с плавающей точкой имеют некоторую ошибку - неизбежно в цифровых системах"
        };

        for (size_t i = 0; i < insights.size(); i++) {
            drawText(margin + 20, insightsY + 45 + (int)i * 20, insights[i], RGB(255, 200, 100), font_small);
        }

        drawControlsPanel();
    }

    // Демонстрация бесконечного цикла
    void drawInfiniteLoopDemo() {
        clear();
        drawHeader(L"ДЕМОНСТРАЦИЯ БЕСКОНЕЧНОГО ЦИКЛА");
        drawInputPanel();

        int contentY = 180;

        // Анализ текущего значения
        FloatAnalysis analysis = analyzeFloatStructure(demo_float);
        drawPanel(margin, contentY, width - 2 * margin, 100, L"Анализ текущего значения");

        drawText(margin + 20, contentY + 45, L"Значение: " + to_wstring(demo_float), RGB(255, 255, 255), font_normal);
        drawText(margin + 20, contentY + 75, L"Экспонента: " + to_wstring((int)analysis.exponent - 127),
                RGB(100, 255, 100), font_normal);
        drawText(margin + 400, contentY + 45, L"Мантисса: " + to_wstring(analysis.mantissa),
                RGB(100, 100, 255), font_normal);

        // Симуляция бесконечного цикла
        int simulationY = contentY + 120;
        drawPanel(margin, simulationY, width - 2 * margin, 120, L"Симуляция бесконечного цикла");

        drawText(margin + 20, simulationY + 45, L"Тестирование: x = " + to_wstring(demo_float) + L" + 1.0",
                RGB(255, 255, 255), font_normal);

        float next_value = demo_float + 1.0f;
        bool will_loop = (next_value == demo_float);

        wstring result_text = L"Результат: " + to_wstring(demo_float) + L" + 1 = " + to_wstring(next_value);
        drawText(margin + 20, simulationY + 75, result_text,
                will_loop ? RGB(255, 100, 100) : RGB(100, 255, 100), font_normal);

        if (will_loop) {
            drawText(margin + 20, simulationY + 105, L"*** ОБНАРУЖЕН БЕСКОНЕЧНЫЙ ЦИКЛ! ***",
                    RGB(255, 50, 50), font_normal);
            drawText(margin + 400, simulationY + 45, L"УСЛОВИЕ ЦИКЛА: x == x + 1",
                    RGB(255, 100, 100), font_normal);
        } else {
            drawText(margin + 20, simulationY + 105, L"Значения разные - цикл будет работать нормально",
                    RGB(100, 255, 100), font_normal);
            drawText(margin + 400, simulationY + 45, L"УСЛОВИЕ ЦИКЛА: x ≠ x + 1",
                    RGB(100, 255, 100), font_normal);
        }

        // Классические примеры бесконечных циклов
        int examplesY = simulationY + 150;
        drawPanel(margin, examplesY, width - 2 * margin, 150, L"Классические примеры бесконечных циклов");

        vector<wstring> code_examples = {
            L"// ОПАСНО: Потенциальный бесконечный цикл",
            L"float x = 16777216.0f;  // 2^24",
            L"while (x < 16777218.0f) {",
            L"    x += 1.0f;  // Никогда не меняется после этой точки!",
            L"    // БЕСКОНЕЧНЫЙ ЦИКЛ!",
            L"}"
        };

        for (size_t i = 0; i < code_examples.size(); i++) {
            drawText(margin + 20, examplesY + 45 + (int)i * 18, code_examples[i],
                    i == 0 ? RGB(255, 100, 100) : RGB(200, 200, 255), font_small);
        }

        // Объяснение
        int explanationY = examplesY + 170;
        drawPanel(margin, explanationY, width - 2 * margin, 100, L"Почему это происходит");

        vector<wstring> explanations = {
            L"• Значения float становятся разреженными при больших величинах",
            L"• Расстояние между соседними float > 1 при 2^24 (16,777,216)",
            L"• Добавление 1.0 не меняет значение - нет прогрессии в цикле",
            L"• Условие цикла никогда не становится ложным"
        };

        for (size_t i = 0; i < explanations.size(); i++) {
            drawText(margin + 20, explanationY + 45 + (int)i * 20, explanations[i],
                    RGB(255, 200, 100), font_small);
        }

        drawControlsPanel();
    }

    // Демонстрация интегрирования
    void drawIntegrationDemo() {
        clear();
        drawHeader(L"ДЕМОНСТРАЦИЯ ЧИСЛЕННОГО ИНТЕГРИРОВАНИЯ");

        // Специальная панель ввода для интеграции
        int inputY = 90;
        drawPanel(margin, inputY, width - 2 * margin, 70, L"Параметры интегрирования");
        if (input_mode) {
            wstring input_text = L"Введите количество шагов (1-1000): " + input_buffer + L"_";
            drawText(margin + 20, inputY + 45, input_text, RGB(255, 255, 100), font_normal);
        } else {
            wstring display_value = L"Текущие шаги: " + to_wstring(integration_steps) +
                                   L" | Функция: y = x² | Интервал: [0, 1] | Нажмите I чтобы изменить шаги";
            drawText(margin + 20, inputY + 45, display_value, RGB(200, 255, 200), font_normal);
        }

        int contentY = 180;

        // Визуализация интегрирования
        int graphHeight = 300;
        drawIntegrationVisualization(margin, contentY, width - 2 * margin, graphHeight, integration_steps);

        // Результаты интегрирования
        int resultsY = contentY + graphHeight + 20;
        drawPanel(margin, resultsY, width - 2 * margin, 180, L"Результаты интегрирования");

        double analytical = analytical_integral();
        double rect_left_double = integrate_rectangle_left(0, 1, integration_steps, true);
        double rect_mid_double = integrate_rectangle_midpoint(0, 1, integration_steps, true);
        double rect_left_float = integrate_rectangle_left(0, 1, integration_steps, false);
        double rect_mid_float = integrate_rectangle_midpoint(0, 1, integration_steps, false);

        int textY = resultsY + 45;
        drawText(margin + 20, textY, L"Аналитический результат: ∫₀¹ x² dx = 1/3 = " + to_wstring(analytical),
                RGB(255, 255, 255), font_normal);

        // Сравнение double vs float
        int column1 = margin + 20;
        int column2 = margin + width/2;

        drawText(column1, textY + 35, L"Точность double:", RGB(100, 255, 100), font_normal);
        drawText(column1, textY + 60, L"  Левые прямоугольники: " + to_wstring(rect_left_double) +
                L" (ошибка: " + to_wstring(fabs(rect_left_double - analytical)) + L")",
                RGB(200, 255, 200), font_small);
        drawText(column1, textY + 85, L"  Правило средней точки:  " + to_wstring(rect_mid_double) +
                L" (ошибка: " + to_wstring(fabs(rect_mid_double - analytical)) + L")",
                RGB(200, 255, 200), font_small);

        drawText(column2, textY + 35, L"Точность float:", RGB(255, 200, 100), font_normal);
        drawText(column2, textY + 60, L"  Левые прямоугольники: " + to_wstring(rect_left_float) +
                L" (ошибка: " + to_wstring(fabs(rect_left_float - analytical)) + L")",
                RGB(255, 255, 200), font_small);
        drawText(column2, textY + 85, L"  Правило средней точки:  " + to_wstring(rect_mid_float) +
                L" (ошибка: " + to_wstring(fabs(rect_mid_float - analytical)) + L")",
                RGB(255, 255, 200), font_small);

        // Сравнение ошибок
        double float_error = fabs(rect_left_float - analytical);
        double double_error = fabs(rect_left_double - analytical);
        double error_ratio = (double_error > 0) ? float_error / double_error : 1.0;

        wstring comparison = L"Ошибка float / Ошибка double = " + to_wstring(error_ratio);
        drawText(margin + 20, textY + 115, comparison,
                 (error_ratio > 10) ? RGB(255, 100, 100) :
                 (error_ratio > 2) ? RGB(255, 200, 100) : RGB(100, 255, 100), font_small);

        // Графики функций
        int graphsY = resultsY + 200;
        int graphPanelHeight = 250;
        drawPanel(margin, graphsY, width - 2 * margin, graphPanelHeight, L"Анализ функции: y = x²");

        int graphWidth = (width - 2 * margin - 40) / 3;

        // Функция
        drawCoordinateSystem(margin + 20, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 1, L"Функция: y = x²");
        drawFunctionGraph(margin + 20, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 2, function,
                         RGB(0, 200, 255), L"y = x²");
        drawText(margin + 20 + graphWidth/2, graphsY + 40, L"y = x²", RGB(0, 200, 255), font_normal, true);

        // Производная
        drawCoordinateSystem(margin + 30 + graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 2, L"Производная: y' = 2x");
        drawFunctionGraph(margin + 30 + graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 2, derivative,
                         RGB(255, 100, 100), L"y' = 2x");
        drawText(margin + 30 + graphWidth + graphWidth/2, graphsY + 40, L"y' = 2x", RGB(255, 100, 100), font_normal, true);

        // Первообразная
        drawCoordinateSystem(margin + 40 + 2 * graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 0.4, L"Первообразная: ∫x²dx = x³/3");
        drawFunctionGraph(margin + 40 + 2 * graphWidth, graphsY + 50, graphWidth, graphPanelHeight - 60, 0, 1, 0, 0.4, antiderivative,
                         RGB(100, 255, 100), L"∫x²dx = x³/3");
        drawText(margin + 40 + 2 * graphWidth + graphWidth/2, graphsY + 40, L"∫x²dx = x³/3", RGB(100, 255, 100), font_normal, true);

        drawControlsPanel();
    }

    // Демонстрация потери значимости и денормализованных чисел
    void drawUnderflowDemo() {
        clear();
        drawHeader(L"ПОТЕРЯ ЗНАЧИМОСТИ И ДЕНОРМАЛИЗОВАННЫЕ ЧИСЛА");
        drawInputPanel();

        int contentY = 180;

        // Статус DAZ/FTZ
        drawPanel(margin, contentY, width - 2 * margin, 70, L"Управление DAZ/FTZ");
        wstring mode_status = daz_ftz_enabled ? L"ВКЛЮЧЕНО (DAZ/FTZ ON)" : L"ВЫКЛЮЧЕНО (DAZ/FTZ OFF)";
        drawText(margin + 20, contentY + 45, L"Текущий режим: " + mode_status,
                daz_ftz_enabled ? RGB(255, 100, 100) : RGB(100, 255, 100), font_normal);
        drawText(margin + 400, contentY + 45, L"Нажмите D чтобы переключить режим DAZ/FTZ",
                RGB(255, 255, 100), font_normal);

        // Генерация денормализованных чисел
        int denormalY = contentY + 90;
        drawPanel(margin, denormalY, width - 2 * margin, 180, L"Примеры денормализованных чисел");

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
                denormal_info.push_back(L"0.0 | Тип: НУЛЬ | Потеря значимости до нуля");
            } else {
                denormal_examples.push_back(test_value);
                FloatAnalysis analysis = analyzeFloatStructure(test_value);
                wstring info = to_wstring(test_value) + L" | Тип: " + analysis.type +
                            L" | Экспонента: " + to_wstring(analysis.exponent) +
                            L" | Мантисса: " + to_wstring(analysis.mantissa);
                denormal_info.push_back(info);
            }
        }

        int exampleY = denormalY + 45;
        for (size_t i = 0; i < denormal_examples.size(); i++) {
            wstring example_text = L"Денормал " + to_wstring(i+1) + L": " + denormal_info[i];
            drawText(margin + 20, exampleY, example_text,
                    denormal_examples[i] == 0.0f ? RGB(255, 100, 100) : RGB(255, 200, 100), font_small);
            exampleY += 25;
        }

        // Бинарное представление первого денормала
        if (!denormal_examples.empty() && denormal_examples[0] != 0.0f) {
            wstring binary = floatToBinaryIEEE754(denormal_examples[0]);
            drawText(margin + 20, exampleY, L"Двоичное представление Денормала 1: " + binary,
                    RGB(200, 200, 255), font_small);
        }

        // Сравнение производительности
        int performanceY = denormalY + 200;
        drawPanel(margin, performanceY, width - 2 * margin, 120, L"Сравнение производительности");

        double normal_time = 1.0, denormal_time = 5.0; // Значения по умолчанию
        try {
            normal_time = measureOperationTime(false, 50000);
            denormal_time = measureOperationTime(true, 50000);
        } catch (...) {}

        drawText(margin + 20, performanceY + 45, L"Время для нормальных чисел: " + to_wstring(normal_time) + L" мс",
                RGB(100, 255, 100), font_normal);
        drawText(margin + 20, performanceY + 75, L"Время для денормализованных чисел: " + to_wstring(denormal_time) + L" мс",
                RGB(255, 100, 100), font_normal);

        double slowdown = 0.0;
        if (normal_time > 0) {
            slowdown = (denormal_time / normal_time - 1.0) * 100.0;
        }
        wstring slowdown_text = L"Замедление производительности: " + to_wstring(slowdown) + L"%";
        drawText(margin + 400, performanceY + 45, slowdown_text,
                slowdown > 100 ? RGB(255, 50, 50) :
                slowdown > 50 ? RGB(255, 150, 50) : RGB(255, 200, 100), font_normal);

        // Режимы DAZ/FTZ
        int modesY = performanceY + 140;
        drawPanel(margin, modesY, width - 2 * margin, 100, L"Режимы DAZ vs FTZ");

        vector<wstring> explanations = {
            L"• DAZ (Denormals Are Zero): Денормализованные входы обрабатываются как нули",
            L"• FTZ (Flush To Zero): Денормализованные результаты сбрасываются в ноль",
            L"• Оба улучшают производительность, но уменьшают точность",
            L"• Используются в графике/играх, где скорость > точность"
        };

        for (size_t i = 0; i < explanations.size(); i++) {
            drawText(margin + 20, modesY + 45 + (int)i * 20, explanations[i], RGB(200, 200, 255), font_small);
        }

        // Влияние на точность
        int precisionY = modesY + 120;
        drawPanel(margin, precisionY, width - 2 * margin, 100, L"Влияние на точность с DAZ/FTZ");

        float small_sum = 0.0f;
        float increment = demo_float != 0.0f ? demo_float / 1e10f : 1e-38f;

        for (int i = 0; i < 100; i++) {
            small_sum += increment;
        }

        drawText(margin + 20, precisionY + 45, L"Сумма 100 × " + to_wstring(increment) + L": " + to_wstring(small_sum),
                RGB(255, 255, 255), font_normal);

        if (daz_ftz_enabled && small_sum == 0.0f) {
            drawText(margin + 20, precisionY + 75, L"*** ТОЧНОСТЬ ПОТЕРЯНА: DAZ/FTZ сбросили результаты в ноль ***",
                    RGB(255, 100, 100), font_normal);
        } else if (small_sum > 0.0f) {
            drawText(margin + 20, precisionY + 75, L"✓ Точность сохранена (малый результат накоплен)",
                    RGB(100, 255, 100), font_normal);
        } else {
            drawText(margin + 20, precisionY + 75, L"Потеря значимости до нуля (нормальное поведение)",
                    RGB(255, 200, 100), font_normal);
        }

        // Ключевые инсайты
        int insightsY = precisionY + 120;
        drawPanel(margin, insightsY, width - 2 * margin, 80, L"Ключевые инсайты");

        vector<wstring> insights = {
            L"• Денормалы позволяют постепенную потерю значимости, но МЕДЛЕННЫ",
            L"• DAZ/FTZ улучшают производительность, но жертвуют точностью",
            L"• Выбирайте на основе приложения: наука (оставить) vs игры (отключить)"
        };

        for (size_t i = 0; i < insights.size(); i++) {
            drawText(margin + 20, insightsY + 45 + (int)i * 20, insights[i], RGB(255, 200, 100), font_small);
        }

        drawControlsPanel();
    }

    // Рисование главного меню
    void drawMainMenu() {
        clear();

        // Заголовок
        drawFilledRect(0, 0, width, 120, RGB(50, 50, 60));
        drawText(0, 30, L"ЛАБОРАТОРИЯ ЧИСЕЛ С ПЛАВАЮЩЕЙ ТОЧКОЙ", RGB(255, 255, 255), font_large, true);
        drawText(0, 75, L"Интерактивная система визуализации", RGB(200, 200, 255), font_title, true);

        int startY = 150;
        int panelWidth = (width - 3 * margin) / 2;
        int panelHeight = 100;

        COLORREF panelColors[] = {
            RGB(65, 80, 100),   // Анализ Float
            RGB(80, 100, 65),   // Анализ Integer
            RGB(100, 80, 65),   // Проблемы точности
            RGB(80, 65, 100),   // Переполнение мантиссы
            RGB(100, 65, 80),   // Бесконечный цикл
            RGB(65, 100, 80),   // Численное интегрирование
            RGB(100, 80, 80)    // Потеря значимости и денормалы
        };

        wstring titles[] = {
            L"1 - Анализ Float",
            L"2 - Анализ Integer",
            L"3 - Проблемы точности",
            L"4 - Переполнение мантиссы",
            L"5 - Бесконечный цикл",
            L"6 - Численное интегрирование",
            L"7 - Потеря значимости и денормалы"
        };

        wstring descriptions[] = {
            L"Преобразование Float в двоичный\nАнализ структуры IEEE 754",
            L"Преобразование Integer в двоичный\n32-битное представление",
            L"Анализ проблем точности\nДемонстрация 0.1 + 0.2",
            L"Визуализация переполнения мантиссы\nПрогрессия степеней 10",
            L"Демонстрация бесконечного цикла\nПроблемы дискретности float",
            L"Методы численного интегрирования\nВизуализация метода прямоугольников",
            L"Потеря значимости и денормализованные числа\nСравнение режимов DAZ/FTZ"
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
        drawPanel(margin, controlsY, width - 2 * margin, 100, L"Управление", RGB(70, 70, 80));

        vector<wstring> controlLines = {
            L"1-7 - Переключение между демонстрационными модулями",
            L"I - Ввод значений (в режиме демонстрации)",
            L"M - Возврат в главное меню (из любой демонстрации)",
            L"ESC - Выход из программы"
        };

        for (size_t i = 0; i < controlLines.size(); i++) {
            drawText(margin + 20, controlsY + 35 + (int)i * 20, controlLines[i],
                    RGB(255, 255, 100), font_normal);
        }

        // Нижняя информационная панель
        drawText(margin, height - 30,
                L"Нажмите цифровые клавиши 1-7 для начала изучения арифметики с плавающей точкой! | Используйте клавишу I для ввода значений в демонстрациях",
                RGB(150, 150, 200), font_small, true);
    }

    // Перерисовка экрана
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
    // Обновление окна
    void update() {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Запуск главного цикла
    void run() {
        setDemo(0);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // Обработка нажатий клавиш
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

    // Процедура окна
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

// ==================== ГЛАВНАЯ ПРОГРАММА ====================

int main() {
    SetConsoleOutputCP(CP_UTF8);

    wcout << L"Лаборатория чисел с плавающей точкой - Интерактивная демонстрационная система\n";
    wcout << L"==============================================================================\n\n";

    // Простые консольные демонстрации как в задании
    demonstrateUnsignedIntToBinary();  // Пункт 0
    demonstrateSimpleUnion();          // Пункт 1
    demonstrateMantissaOverflow();     // Пункт 2
    demonstrateClassicInfiniteLoop();  // Пункт 3
    demonstratePrecisionLoss();
    demonstrateDiscreteNature();

    wcout << L"\nЗапуск графического интерфейса...\n";
    wcout << L"Управление:\n";
    wcout << L"  1 - Демонстрация Float в двоичный\n";
    wcout << L"  2 - Демонстрация Integer в двоичный\n";
    wcout << L"  3 - Демонстрация проблем точности\n";
    wcout << L"  4 - Демонстрация переполнения мантиссы\n";
    wcout << L"  5 - Демонстрация бесконечного цикла\n";
    wcout << L"  6 - Демонстрация численного интегрирования\n";
    wcout << L"  7 - Демонстрация потери значимости и денормалов\n";
    wcout << L"  I - Ввод нового значения (в демонстрации)\n";
    wcout << L"  M - Возврат в главное меню\n";
    wcout << L"  D - Переключить DAZ/FTZ (в демонстрации 7)\n";
    wcout << L"  H - Переключить справку\n";
    wcout << L"  ESC - Выход\n\n";

    FloatingPointLabWindow window;
    window.run();

    wcout << L"Демонстрация завершена. Спасибо!\n";
    return 0;
}
