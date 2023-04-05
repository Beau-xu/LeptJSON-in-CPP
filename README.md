# LeptJSON Parser in C++ Language

[LeptJSON Library](https://github.com/miloyip/json-tutorial) in C++11.

测试驱动开发，使用 C++ 编写一个简单的 JSON 库：
- 实现字符串解析为 JSON 格式，或 JSON 的字符串化（stringify）；
- 以 LeptValue 类管理 JSON 数据；
- JSON 字符串、数组、对象以 union 联合体形式共享存储空间，union 内是指向字符串、数组、对象的指针；
- 合理编写 LeptValue 类的构造函数、析构函数、拷贝构造函数、拷贝赋值运算符，避免内存泄漏；
- 通过 C++ vector 容器管理 JSON 的数组和对象，实现自动内存管理。
