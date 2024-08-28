#include <iostream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

typedef unsigned char BYTE;

const uint16_t FRAME_HEADER = 0xABCD;
const uint16_t BASE_INFO_MODULE = 0x4A42;
const uint16_t TARGET_INFO_MODULE = 0x4D42;
const size_t BUFFER_SIZE = 184;

size_t findFrameHeader(const BYTE* buffer, size_t length) 
{
    for (size_t i = 0; i < length - 1; ++i) 
    {
        if (buffer[i] == 0xAB && buffer[i + 1] == 0xCD) 
        {
            return i;
        }
    }
    return length;
}

uint16_t extractUint16(const BYTE* buffer, size_t offset) 
{
    return (buffer[offset] << 8) | buffer[offset + 1];
}

uint32_t extractUint32(const BYTE* buffer, size_t offset) 
{
    return (static_cast<uint32_t>(buffer[offset]) << 24) |
        (static_cast<uint32_t>(buffer[offset + 1]) << 16) |
        (static_cast<uint32_t>(buffer[offset + 2]) << 8) |
        (static_cast<uint32_t>(buffer[offset + 3]));
}

void printTableHeader(const std::string& title) 
{
    std::cout << "\n" << title << "\n";
    std::cout << std::string(title.size(), '=') << "\n";
    std::cout << std::left << std::setw(5) << "№"
        << std::setw(30) << "Название"
        << std::setw(20) << "Содержание"
        << std::setw(20) << "Длина данных (бит)"
        << std::setw(40) << "Описание"
        << std::endl;
    std::cout << std::string(115, '-') << std::endl;
}

void printTableRow(int num, const std::string& name, const std::string& content, int length, const std::string& description)
{
    std::cout << std::left << std::setw(5) << num
        << std::setw(30) << name
        << std::setw(20) << content
        << std::setw(20) << length
        << std::setw(40) << description
        << std::endl;
}

void printBaseInfoModule(const BYTE* buffer, size_t offset) 
{
    printTableHeader("Модуль базовой информации");
    printTableRow(1, "Тип информационного модуля", "0x4A42", 16, "ASCII-код для 'JB'");
    uint16_t dataLength = extractUint16(buffer, offset + 4);
    printTableRow(2, "Длина данных модуля", std::to_string(dataLength), 16, "Длина данных модуля базовой информации");
    uint8_t crc = buffer[offset + 4 + dataLength - 1];
    printTableRow(3, "CRC", std::to_string(crc), 8, "Контрольная сумма модуля");
    std::cout << std::endl;
}

void printTargetInfoModule(const BYTE* buffer, size_t offset, size_t length)
{
    printTableHeader("Модуль информации о цели");
    size_t pos = offset;
    int targetCount = 0;

    while (pos < offset + length) 
    {
        printTableRow(1, "Тип информационного модуля", "0x4D42", 16, "ASCII-код для 'MB'");
        uint16_t dataLength = extractUint16(buffer, pos + 2);
        printTableRow(2, "Длина данных модуля", std::to_string(dataLength), 16, "Длина данных целевого информационного модуля");

        size_t targetOffset = pos + 4;
        int i = 0;

        while (targetOffset + 33 <= pos + dataLength) 
        {
            targetCount++;
            uint16_t targetNum = extractUint16(buffer, targetOffset);
            if (targetNum > 255 || targetNum == 0)
            {
                printTableRow(3 + i, "Номер цели", "Ошибка", 8, "Без знака");
            }
            else
            {
                printTableRow(3 + i, "Номер цели", std::to_string(targetNum), 8, "Без знака");
            }

            uint16_t longitudinalDistance = extractUint16(buffer, targetOffset + 1);
            printTableRow(4 + i, "Продольное расстояние", std::to_string(longitudinalDistance * 0.1), 16, "м");

            uint16_t horizontalDistance = extractUint16(buffer, targetOffset + 3);
            printTableRow(5 + i, "Горизонтальное расстояние", std::to_string(horizontalDistance * 0.1), 16, "м");

            uint16_t speedY = extractUint16(buffer, targetOffset + 5);
            printTableRow(6 + i, "Скорость (Y направление)", std::to_string(speedY * 0.1), 16, "м");

            uint8_t targetType = buffer[targetOffset + 7];
            if (targetType > 4)
            {
                printTableRow(7 + i, "Тип цели", "Ошибка", 8, "Тип цели");
            }
            else
            {
                printTableRow(7 + i, "Тип цели", std::to_string(targetType), 8, "Тип цели");
            }

            uint8_t laneNumber = buffer[targetOffset + 8];
            if (laneNumber > 8)
            {
                printTableRow(8 + i, "Номер полосы движения", "Ошибка", 8, "Без знака");
            }
            else
            {
                printTableRow(8 + i, "Номер полосы движения", std::to_string(laneNumber), 8, "Без знака");
            }

            uint16_t frontDistance = extractUint16(buffer, targetOffset + 9);
            printTableRow(9 + i, "Переднее расстояние", std::to_string(frontDistance * 0.1), 16, "м");

            uint16_t frontTimeInterval = extractUint16(buffer, targetOffset + 11);
            printTableRow(10 + i, "Передний временной интервал", std::to_string(frontTimeInterval * 0.1), 16, "с");

            uint16_t speedX = extractUint16(buffer, targetOffset + 13);
            printTableRow(11 + i, "Скорость (X направление)", std::to_string(speedX * 0.1), 16, "м");

            uint16_t guideAngle = extractUint16(buffer, targetOffset + 15);
            if (guideAngle > 360)
            {
                printTableRow(12 + i, "Направляющий угол", "Ошибка", 16, "Градусы");
            }
            else
            {
                printTableRow(12 + i, "Направляющий угол", std::to_string(guideAngle * 0.01), 16, "градусы");
            }

            uint8_t eventFlags = buffer[targetOffset + 17];
            printTableRow(13 + i, "Произошло событие", std::to_string(eventFlags), 8, "Каждый бит представляет событие");

            uint32_t radarX = extractUint32(buffer, targetOffset + 18);
            printTableRow(14 + i, "Координата X радиолокационной сети", std::to_string(radarX * 0.1), 32, "м");

            uint32_t radarY = extractUint32(buffer, targetOffset + 22);
            printTableRow(15 + i, "Координата Y радиолокационной сети", std::to_string(radarY * 0.1), 32, "м");

            uint8_t blindSpot = buffer[targetOffset + 26];
            if (blindSpot != 0 && blindSpot != 1)
            {
                printTableRow(16 + i, "Заполните слепую отметку", "Ошибка", 8, "Слепой радар");
            }
            else
            {
                printTableRow(16 + i, "Заполните слепую отметку", std::to_string(blindSpot), 8, "Слепой радар");
            }

            uint8_t carLength = buffer[targetOffset + 27];
            printTableRow(17 + i, "Длина автомобиля", std::to_string(carLength * 0.1), 8, "0.1 м/бит");

            uint8_t carWidth = buffer[targetOffset + 28];
            printTableRow(18 + i, "Ширина автомобиля", std::to_string(carWidth * 0.1), 8, "0.1 м/бит");

            targetOffset += 33;
            i += 16;
        }

        uint8_t crc = buffer[pos + dataLength - 8];
        printTableRow(i + 3, "CRC", std::to_string(crc), 8, "Контрольная сумма для всех данных целей");

        pos += dataLength;
    }
    std::cout << "Количество обработанных целей: " << targetCount << std::endl;
    std::cout << std::endl;
}

int main() {
    setlocale(LC_ALL, "rus");
    BYTE buffer_data[BUFFER_SIZE] = {
        0xAB, 0xCD, 0x00, 0xB3, 0x4A, 0x42, 0x00, 0x19, 0x01, 0x00,
        0x01, 0x02, 0x03, 0x14, 0x0F, 0x01, 0x04, 0x1C, 0x08, 0x04,
        0x00, 0x1B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x1D, 0x4D,
        0x42, 0x00, 0x75, 0xE2, 0x02, 0x61, 0xFF, 0xC2, 0xFF, 0x04,
        0x00, 0x03, 0x04, 0x3E, 0x00, 0x2A, 0xFF, 0xFB, 0x46, 0xC3,
        0x00, 0xFF, 0xFF, 0xFF, 0xC2, 0x00, 0x00, 0x02, 0x61, 0x00,
        0x2D, 0xE3, 0x03, 0x85, 0xFF, 0xA5, 0xFE, 0xD2, 0x00, 0x02,
        0x05, 0x03, 0x00, 0x2A, 0xFF, 0xF6, 0x47, 0x1A, 0x00, 0xFF,
        0xFF, 0xFF, 0xA5, 0x00, 0x00, 0x03, 0x85, 0x00, 0x2D, 0xE4,
        0x05, 0xEE, 0xFF, 0xE0, 0xFE, 0xD3, 0x00, 0x04, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x46, 0x62, 0x00, 0xFF, 0xFF, 0xFF,
        0xE0, 0x00, 0x00, 0x05, 0xEE, 0x00, 0x2D, 0xE1, 0x06, 0x6D,
        0x00, 0x82, 0x00, 0xF7, 0x00, 0x08, 0x11, 0x63, 0x00, 0xB5,
        0x00, 0x0B, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x82, 0x00,
        0x00, 0x06, 0x6D, 0x00, 0x2F, 0xEA, 0x44, 0x4C, 0x00, 0x25,
        0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x03, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x08, 0x00,
        0x00, 0x00, 0xD9, 0xC0
    };

    size_t length = sizeof(buffer_data);
    size_t frameHeaderPos = findFrameHeader(buffer_data, length);
    if (frameHeaderPos == length) {
        std::cout << "Заголовок фрейма не найден" << std::endl;
        return 1;
    }

    std::cout << "\nЗаголовок фрейма\n";
    std::cout << "====================\n";
    printTableRow(1, "Заголовок фрейма", "0xABCD", 16, "Заголовок фрейма данных");

    uint16_t totalDataLength = extractUint16(buffer_data, frameHeaderPos + 2);
    printTableRow(2, "Общий объем данных", std::to_string(totalDataLength), 16, "Длина в байтах всех данных в модуле данных");

    size_t offset = frameHeaderPos + 4;

    while (offset < length) {
        uint16_t moduleType = extractUint16(buffer_data, offset);
        uint16_t moduleDataLength = extractUint16(buffer_data, offset + 2);

        std::cout << "\nОбработка модуля типа: 0x" << std::hex << moduleType << std::dec << std::endl;

        if (moduleType == BASE_INFO_MODULE) {
            printBaseInfoModule(buffer_data, offset);
        }
        else if (moduleType == TARGET_INFO_MODULE) {
            printTargetInfoModule(buffer_data, offset, moduleDataLength);
        }
        else {
            std::cout << "Неизвестный тип модуля: 0x" << std::hex << moduleType << std::dec << std::endl;
            break;
        }

        offset += moduleDataLength;
    }

    return 0;
}
