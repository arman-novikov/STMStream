#pragma once
#include <usart.h>
#include "../string/stmstring.h"

namespace etl {
	constexpr const char* endl = "\n\r";
}

namespace serial_ns {
	struct ring_t {
		static constexpr size_t USART_BUF_SIZE = 64U;
		uint8_t buf[ring_t::USART_BUF_SIZE];
		size_t write_counter;
		size_t read_counter;
		int read();
	};
};

class Stream {
public:
	Stream(UART_HandleTypeDef *huart);
	void write(const char *data);
	void write(const char data);
	void writeln(const char *data);
	void write(int val, int radix = 10);
	void writeln(int val, int radix = 10);
	int read();
	int readln(char* buf);
	String readString();
	template<typename T>
	Stream& operator<<(const T& val) {
		this->write(val);
		return *this;
	}
	~Stream() = default;
protected:
	UART_HandleTypeDef *_huart;
	serial_ns::ring_t *_ring;
};
