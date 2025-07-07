#pragma once
#include <usart.h>
#include "../stmstring/stmstring.h"

namespace etl {
	constexpr const char* endl = "\r\n";
}

namespace serial_ns {
	struct ring_t {
		static constexpr size_t USART_BUF_SIZE = 64U;
		uint8_t buffer[ring_t::USART_BUF_SIZE]{};
		size_t write_counter = 0;
		size_t read_counter = 0;
		size_t data_size = 0;
		inline size_t available() const {
			return data_size;
		}
		void incoming_byte_handler(UART_HandleTypeDef *huart);
		int read();
	};
};

class Stream {
public:
	Stream(UART_HandleTypeDef *huart);
	void write_IT(const char *data, size_t len);
	void write_IT(const char *data);
	void write(const char *data);
	void write(const char data);
	void writeln(const char *data);
	void write(int val, int radix = 10);
	void writeln(int val, int radix = 10);
	int read();
	int readln(char* buf);
	String readString();
	inline bool available() const {
		return this->_ring->available();
	}

	template<typename T>
	Stream& operator<<(const T& val) {
		this->write(val);
		return *this;
	}

	inline UART_HandleTypeDef * const get_huart() {
		return this->_huart;
	}

	~Stream() = default;
protected:
	UART_HandleTypeDef *_huart;
	serial_ns::ring_t *_ring;
};
