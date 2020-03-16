#include "stream.h"
#include <cstring>
#include <cstdlib>

using serial_ns::ring_t;
static ring_t *USART1_DATA = nullptr;
static ring_t *USART2_DATA = nullptr;
static ring_t *USART3_DATA = nullptr;

Stream::Stream(UART_HandleTypeDef *huart):
	_huart(huart), _ring(nullptr)
{
	if (this->_huart->Instance == USART1) {
		USART1_DATA = new ring_t();
		this->_ring = USART1_DATA;
	} else if (this->_huart->Instance == USART2) {
		USART2_DATA = new ring_t();
		this->_ring = USART2_DATA;
	} else if (this->_huart->Instance == USART3) {
		USART3_DATA = new ring_t();
		this->_ring = USART3_DATA;
	}
	HAL_UART_Receive_IT(huart, this->_ring->buf, 1);
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
	int res = 0;
	if (this->read_counter == this->write_counter)
		return -86; // ENODATA

	res = static_cast<int>(this->buf[this->read_counter++]);
	if (this->read_counter >= ring_t::USART_BUF_SIZE) {
		this->read_counter = 0;
	}

	return res;
}

void ring_t::incoming_byte_handler(UART_HandleTypeDef *huart)
{
	++this->write_counter;

	if (this->write_counter == this->read_counter)
		++this->read_counter;

	if (this->read_counter >= ring_t::USART_BUF_SIZE) {
		this->read_counter = 0;
	}

	if (this->write_counter >= ring_t::USART_BUF_SIZE) {
		this->write_counter = 0;
	}

	HAL_UART_Receive_IT(huart, &(this->buf[this->write_counter]), 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1) {
		USART1_DATA->incoming_byte_handler(huart);
	} else if (huart->Instance == USART2) {
		USART2_DATA->incoming_byte_handler(huart);
	} else if (huart->Instance == USART3) {
		USART3_DATA->incoming_byte_handler(huart);
	}
}
