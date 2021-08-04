#include "../lib/stream/stream.h"
#include "../lib/string/stmstring.h"

void SystemClock_Config(void);
static void common_init();

Stream *console, *neoway;
String str = "very ok";

int main(void)
{
	common_init();
	console = new Stream(&huart2);
	neoway = new Stream(&huart1);
	String data_str;
	char buf[16]{};
	size_t counter = 0, print_counter = 0;
	*console << "init done" << etl::endl;

	while (1) { // readString test
		String str = neoway->readString();
		*console << "new string: " << str << etl::endl;
	}

	while (0) { // readln test
		char buf[512] {};
		neoway->readln(buf);
		*console << "new data: " <<  buf << etl::endl;
	}

	while (0) { // common test

		int data = neoway->read();
		if (data > 0) {
			//*console << static_cast<char>(data);
			buf[counter++] = static_cast<char>(data);
		}
		if (counter >= 14) {
			counter = 0;
			data_str += buf;
			++print_counter;
		}

		if (print_counter >= 10) {
			*console << data_str;
			print_counter = 0;
			data_str = "";
		}
	}

}