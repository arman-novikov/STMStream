### HAL based uart facade class for STM MCUs

#### usage:
```
Stream console{&huart6};

console.write("hello");
console.writeln("world");
console.write(100500);
console << "hello" << 123;
if (console.available()) {
    char buf[128];
    if (int res = readln(buf); res > 0) {
        console << res << " bytes read;
    } else {
        console << "error" << res;
    }
    console << etl::endl;
    const auto str = console.readString();
    console << "echo: " << str.data() << etl::endl;
}
```