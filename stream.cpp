#include "stream.h"
#include <cstring>
#include <cstdlib>

using serial_ns::ring_t;
static ring_t *USART1_DATA = nullptr;
static ring_t *USART2_DATA = nullptr;
static ring_t *USART6_DATA = nullptr;

Stream::Stream(UART_HandleTypeDef *huart):
    _huart(huart), _ring(nullptr)
{
    if (this->_huart->Instance == USART1) {
        USART1_DATA = new ring_t();
        this->_ring = USART1_DATA;
    } else if (this->_huart->Instance == USART2) {
        USART2_DATA = new ring_t();
        this->_ring = USART2_DATA;
    } else if (this->_huart->Instance == USART6) {
        USART6_DATA = new ring_t();
        this->_ring = USART6_DATA;
    }
    HAL_UART_Receive_IT(huart, this->_ring->buf, 1);
}

void Stream::write_IT(const char *data, size_t len)
{
    char* data_casted = const_cast<char*>(data);
    uint8_t *data_p = reinterpret_cast<uint8_t*>(data_casted);
    HAL_UART_Transmit_IT(this->_huart, data_p, len);
    HAL_Delay(len);
}

void Stream::write_IT(const char *data)
{
    this->write_IT(data, strlen(data));
}

void Stream::write(const char *data)
{
    char* data_casted = const_cast<char*>(data);
    uint8_t *data_p = reinterpret_cast<uint8_t*>(data_casted);
    HAL_UART_Transmit((this->_huart), data_p, ::strlen(data), HAL_MAX_DELAY);
}

void Stream::write(const char data)
{
    uint8_t data_casted = static_cast<uint8_t>(data);
    HAL_UART_Transmit((this->_huart), &data_casted, 1, HAL_MAX_DELAY);
}

void Stream::writeln(const char *data)
{
    this->write(data);
    this->write(etl::endl);
}

void Stream::write(int val, int radix)
{
    char itoa_buf[16];
    ::itoa(val, itoa_buf, radix);
    this->write(itoa_buf);
}

void Stream::writeln(int val, int radix)
{
    this->write(val, radix);
    this->writeln("");
}

int Stream::read()
{
    return this->_ring->read();
}

int Stream::readln(char* buf)
{
    uint16_t i = 0;
    while (true) {
        const int readbyte = this->read();
        if (readbyte == '\n')
            break;
        if (readbyte == '\r')
            continue;
        if (readbyte > 0)
            buf[i++] = static_cast<char>(readbyte);
    }
    buf[i] = '\0';
    return i;
}


String Stream::readString()
{
    String res{};
    char buf[2]{0};
    if (!this->available()) // nothing to read
        return res;

    while (true) {
        const int readbyte = this->read();
        if (readbyte == '\n')
            break;
        if (readbyte == '\r')
            continue;
        if (readbyte > 0) {
            buf[0] = static_cast<char>(readbyte);
            res += buf;
        }
    }
    return res;
}

int ring_t::read()
{
    if (data_size == 0)
    {
        return -86; // ENODATA
    }
    auto popped_value = buffer[read_counter];
    if (++read_counter >= USART_BUF_SIZE)
    {
        read_counter = 0;
    }
    --data_size;
    return popped_value;
}

void ring_t::incoming_byte_handler(UART_HandleTypeDef *huart)
{
    buffer[write_counter++] = v;
    if (write_counter >= USART_BUF_SIZE)
    {
        write_counter = 0;
    }
    if (++data_size > USART_BUF_SIZE)
    {
        if (++read_counter >= USART_BUF_SIZE)
        {
            read_counter = 0;
        }
    }
    data_size = std::min(data_size, USART_BUF_SIZE);

    HAL_UART_Receive_IT(huart, &(this->buf[this->write_counter]), 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        USART1_DATA->incoming_byte_handler(huart);
    } else if (huart->Instance == USART2) {
        USART2_DATA->incoming_byte_handler(huart);
    } else if (huart->Instance == USART6) {
        USART6_DATA->incoming_byte_handler(huart);
    }
}
