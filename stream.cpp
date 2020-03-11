#include "stream.h"
#include <cstring>
#include <cstdlib>

using serial_ns::ring_t;
static ring_t USART1_DATA{};

Stream::Stream(UART_HandleTypeDef *huart):
	_huart(huart), _ring(nullptr)
{
	if (this->_huart->Instance == USART1) {
		this->_ring = &USART1_DATA;
		HAL_UART_Receive_IT(huart, &USART1_DATA.buf[0], 1);
	}
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
	String res;
	char buf[2]{0};
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1) {
		auto &udata = USART1_DATA;
		++udata.write_counter;

		if (udata.write_counter == udata.read_counter)
			++udata.read_counter;

		if (udata.read_counter >= ring_t::USART_BUF_SIZE) {
					udata.read_counter = 0;
		}

		if (udata.write_counter >= ring_t::USART_BUF_SIZE) {
			udata.write_counter = 0;
		}

		HAL_UART_Receive_IT(huart, &udata.buf[udata.write_counter], 1);
	}
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
