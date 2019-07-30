[TOC]

官网：https://www.hex-rays.com/products/ida/index.shtml

# 资料

- [x] [（二）加载文件与保存数据库](http://blog.csdn.net/hursing/article/details/8923748)
- [x] [（三）函数表示与搜索函数](http://blog.csdn.net/hursing/article/details/8926315)
- [x] [（四）反汇编的符号信息与改名](http://blog.csdn.net/hursing/article/details/8929401)
- [x] [（五）F5反编译](http://blog.csdn.net/hursing/article/details/8935697)
- [ ] [（六）交叉引用](http://blog.csdn.net/hursing/article/details/8939392)
- [ ] [（七）识别类的信息](http://blog.csdn.net/hursing/article/details/8997776)
- [ ] [（八）IDA for Mac](http://blog.csdn.net/hursing/article/details/9019651)
- [ ] [（九）block](http://blog.csdn.net/hursing/article/details/9021941)

- [ ] [IDA Pro 逆向速参（链接）](https://segmentfault.com/a/1190000012834544)

# 基础

- Main window中按空格切换Graph view和Text view

- Graph view中，当出现分支时，绿色代表满足判断条件的分支，否则是红色。当没有分支时，线时蓝色。

- 字体颜色的含义

 ![image.png](https://upload-images.jianshu.io/upload_images/2224431-cb3d885ae2019f09.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

- 单击一个符号时，相同的符号都会黄色高亮显示。

- 双击一个符号时，可以查看它的实现

- 鼠标右击，弹出菜单，常用的功能有“Jump to xref to operand...”（快捷键“x”），点击后出现的窗口罗列了这个文件中显式引用这个符号的所有信息。

# IDC 脚本

IDC 脚本语言借用了 C 语言的许多语法。 从IDA5.6  开始，IDC  在面向对象特性和异常处理方面与C++ 更为相似。

## IDC 语言

### IDC 变量

IDC 没有明确的类型。IDC 使用 3 种数据类型：整数（ IDA 文档使用类型名称long）、字符串和浮点值，其中绝大部分的操作针对的是整数和字符串。字符串被视为 IDC 中的本地数据类型，因此，你不需要跟踪存储一个字符串所需的空间，或者一个字符串是否使用零终止符。从 IDA5.6 开始，IDC 加入了许多变量类型，包括对象、引用和函数指针。

在使用任何变量前，都必须先声明该变量。auto 用于引入一个局部变量声明，并且局部变量声明中可能包括初始值。如下所示是合法与非法的 IDC 局部变量声明：

```c
auto addr, reg, val; // legal, multiple variables declared with no initializers
auto count = 0; // declaration with initialization
```

IDC 并不支持 C 风格数组（ IDA 5.6 引入了切片）、指针（虽然 IDA 从 IDA 5.6 开始支持引用）或结构体和联合之类的复杂数据类型。 IDA 5.6 引入了类的概念。

IDA 使用extern 关键字引入全局变量声明，你可以在任何函数定义的内部和外部声明全局变量，但不能在声明全局变量时为其提供初始值。下面的代码清单声明了两个全局变量。

```c
extern outsideGlobal;
static main() {
	extern insideGlobal;
	outsideGlobal = "Global";
	insideGlobal = 1;
}
```

### IDC 表达式

除少数几个特例外，IDC 几乎支持 C 中的所有算术和逻辑运算符，包括三元运算符（? : ）。IDC 不支持 op= （+= 、*= 、>>= 等）形式的复合赋值运算符。 从IDA5.6 开始支持逗号运算。 所有整数操作数均作为有符号的值处理。这会影响到整数比较（始终带有符号）和右移位运算符（>>），因为它们总是会通过符号位复制进行算术移位。如果需要进行逻辑右移位，你必须修改结果的最高位，自己移位，如下所示：

```c
result = (x >> 1) & 0x7fffffff; //set most significant bit to zero
```

字符串运算与 C 中有所不同。在 IDC 中，给字符串变量中的字符串操作数赋值将导致字符串复制操作，因此，你不需要使用字符串来复制函数，如 C 语言中的strcpy 和 strdup 函数。将两个字符串操作数相加会将这两个操作数拼接起来，因此“Hello” + “World”将得到“HelloWorld”。从 IDA 5.6 开始，IDA 提供用于处理字符串的切片运算符（slice operator）。通常你可以通过切片指定与数组类似的变量的子序列。切片使用方括号和起始索引（包括）与结束索引（不包括）来指定（至少需要一个索引）。下面的代码清单说明了 IDC 切片的用法。

```c
auto str = "String to slice";
auto s1, s2, s3, s4;
s1 = str[7:9]; // "to"
s2 = str[:6]; // "String", omitting start index starts at 0
s3 = str[10:]; // "slice", omitting end index goes to end of string
s4 = str[5]; // "g", single element slice, similar to array element access
```

虽然 IDC 中并没有数组数据类型，但你可以使用分片运算符来处理 IDC 字符串，就好像它们是数组一样。

### IDC 语句

所有简单语句均以分号结束。switch 语句是 IDC 唯一不支持的 C 风格复合语句。需要注意 IDC 不支持复合赋值运算符

```c
auto i;
for (i = 0; i < 10; i += 2) {} // illegal, += is not supported
for (i = 0; i < 10; i = i + 2) {} // legal
```

在 IDA 5.6 中，IDC 引入了try/catch  块和相关的throw  语句，在语法上它们类似于C++ 异常(参见 http://www.cplusplus.com/doc/tutorial/exceptions/。)。有关 IDC 异常处理的详细信息，请参阅 IDA 的内置帮助文件。

在花括号中可以声明新的变量，只要变量声明位于花括号内的第一个语句即可。但是，IDC 并不严格限制新引入的变量的作用范围，因此，你可以从声明这些变量的花括号以外引用它们。

```c
if (1) { //always true
  auto x;
  x = 10;
}
else { //never executes
  auto y;
  y = 3;
}
Message("x = %d\n", x); // x remains accessible after its block terminates
Message("y = %d\n", y); // IDC allows this even though the else did not execute
```

### IDC 函数

IDC 仅仅在==独立程序==（ .idc 文件）中==支持==用户定义的==函数==。IDC 命令==对话框==（参见本节的“使用 IDC 命令对话框”）==不支持==用户定义的函数。IDC 用于声明用户定义的函数的语法与 C 语言差异甚大。在 IDC 中，==static 关键字用于引入一个用户定义的函数==，函数的参数列表仅包含一个以逗号分隔的参数名列表。下面详细说明了一个用户定义的函数的基本结构：

```c
static my_func(x, y, z) {
  //declare any local variables first
  auto a, b, c;
  //add statements to define the function's behavior
  // ...
}
```

由 IDA 调用函数的方式而不是声明函数的方式决定。在函数调用（而不是函数声明）中使用一元运算符 & 说明该函数采用传地址方式传递参数。

```c
auto q = 0, r = 1, s = 2;
my_func(q, r, s); //all three arguments passed using call-by-value
//upon return, q, r, and s hold 0, 1, and 2 respectively
my_func(q, &r, s); //q and s passed call-by-value, r is passed call-by-reference
//upon return, q, and s hold 0 and 2 respectively, but r may have
//changed. In this second case, any changes that my_func makes to its
//formal parameter y will be reflected in the caller as changes to r
```

注意，一个函数声明绝不会指明该函数是否明确返回一个值，以及在不生成结果时，它返回什么类型的值。

如果你希望函数返回一个值，可以使用 return 语句返回指定的值。你可通过函数的不同执行路径返回不同的数据类型。

从IDA 5.6 开始， 你可以将函数引用作为参数传递给另一个函数，并将函数引用作为函数的结果返回。例：

```c
static getFunc() {
  return Message; //return the built-in Message function as a result
}

static useFunc(func, arg) { //func here is expected to be a function reference
  func(arg);
}

static main() {
  auto f = getFunc();
  f("Hello World\n"); //invoke the returned function f
  useFunc(f, "Print me\n"); //no need for & operator, functions always call-by-reference
}
```

### IDC 对象

IDC 定义了一个称为 ==object 的根类，最终所有类都由它衍生而来==，并且在创建新类时支持==单一继承==。IDC 并不使用访问说明符，如 public 与 private。==所有类成员均为有效公共类==。类声明仅包含类成员函数的定义。要在类中创建数据成员，你只需要创建一个给数据成员赋值的赋值语句即可。下面的代码清单有助于说明这一点。

```c
class ExampleClass {
  ExampleClass(x, y) { //constructor
    this.a = x; //all ExampleClass objects have data member a
    this.b = y; //all ExampleClass objects have data member b
  }
  
  ~ExampleClass() { //destructor
  }
  
  foo(x) {
    this.a = this.a + x;
  }
  //... other member functions as desired
};

static main() {
  ExampleClass ex; //DON’T DO THIS!! This is not a valid variable declaration
  auto ex = ExampleClass(1, 2); //reference variables are initialized by assigning
  														//the result of calling the class constructor
  ex.foo(10); //dot notation is used to access members
  ex.z = "string"; //object ex now has a member z, BUT the class does not
}
```

 有关 IDC  类及其语法的更多信息，请参阅IDA  内置帮助文件中的相应章节。

### IDC 程序

如果一个脚本应用程序需要执行大量的 IDC 语句，你可能需要创建一个独立的 IDC 程序文件。另外，将脚本保存为程序，你的脚本将获得一定程度的持久性和可移植性。

IDC 程序文件要求你使用用户定义的函数。至少，必须定义一个没有参数的 main 函数。另外，主程序文件还必须包含 idc.idc 文件以获得它包含的有用宏定义。下面详细说明了一个简单的 IDC 程序文件的基本结构：

```c
#include <idc.idc> // useful include directive
//declare additional functions as required
static main() {
  //do something fun here
}
```

IDC 认可以下C 预处理指令。
- `#include<文件>`。将指定的文件包含在当前文件中。
- `#define<宏名称>[可选值]`。创建一个宏，可以选择给它分配指定的值。IDC 预定义了许多宏来测试脚本执行环境。这些宏包括`_NT_`、`_LINUX_`、`_MAC_`、`_GUI_`和`_TXT_`等。有关这些宏及其他符号的详细信息，请参阅IDA 帮助文件的“预定义的符号”（Predefined symbols）部分。
- `#ifdef<名称>`。测试指定的宏是否存在，如果该宏存在，可以选择处理其后的任何语句。
- `#else`。可以与`#ifdef`指令一起使用，如果指定的宏不存在，它提供另一组供处理的语句。
- `#endif`。`#ifdef` 或`#ifdef/#else`块所需的终止符。
- `#undef<名称>`。删除指定的宏。

### IDC 错误处理

没有人会因为IDC  的错误报告功能而称赞IDC 。在运行IDC  脚本时，你可能遇到两种错误：解析错误和运行时错误。

- 解析错误——指那些令你的程序无法运行的错误，包括语法错误、引用未定义变量、函数参数数量错误。在解析阶段，IDC 仅报告它遇到的第一个解析错误。

- 运行时错误（runtime error）——较为少见。运行时错误会使一段脚本立即终止运行

调试是 IDC  的另一个缺陷。除了大量使用输出语句外，你没有办法调试IDC  脚本。在IDA 5.6  中引入异常处理（try/catch ）之后，你就能够构建更加强大的、可根据你的需要终止或继续的 脚本。

### IDC 永久数据存储

 DC 并不支持传统意义上的数组， 但是，IDA  中有关脚本的文档确实提到“全局永久数组” （global persistent array ）。用户最好是将IDC  全局数组看成已命名的永久对象（persistent named object ）。这些对象恰巧是稀疏数组（sparse array ）^①^ 。全局数组保存在IDA  数据库中，对所有脚本调用和 IDA   会话永久有效。要将数据保存在全局数组中，你需要指定一个索引及一个保存在该索引位置的数据值。数组中的每个元素同时保存一个整数值和一个字符串值。 IDC 的全局数组无法存储浮点值。

① 稀疏数组不一定会预先给整个数组分配空间，也不仅限于使用某个特殊的最大索引。实际上，当元素添加到数组中时，它按需分配这些元素的空间。

与全局数组的所有交互通过专门用于操纵数组的 IDC 函数来完成。这些函数如下所示。

- `long CreateArray(string name)`。这个函数使用指定的名称创建一个永久对象。它的返回值是一个整数句柄，将来访问这个数组时，你需要这个句柄。如果已命名对象已经存在， 则返回-1。
- `long GetArrayId(string name)`。创建一个数组后，随后要访问这个数组，必须通过一个 整数句柄来实现，你可以通过查询数组名称获得这个句柄。这个函数的返回值是一个用 于将来与该数组交互的整数句柄。如果已命名数组并不存在，则返回-1。
- `long SetArrayLong(long id, long idx, long value)`。将整数value 存储到按id 引用的 数组中idx 指定的位置。如果操作成功，则返回1，否则返回0。如果数组id 无效，这个 操作将会失败。
- `long SetArrayString(long id, long idx, string str)`。将字符串value 存储到按id 引用的数组中idx  指定的位置。如果操作成功，则返回1 ，否则返回0 。如果数组id  无效， 这个操作将会失败。
- `string or long GetArrayElement(long tag, long id, long idx)`。虽然一些特殊的函数 可以根据数据类型将数据存储到数组中，但是，只有一个函数可以从数组中提取数据。 这个函数可以从指定数组（id）的指定索引（idx）位置提取一个整数或字符串值。提取 的是整数还是字符串，由tag 参数的值决定，这个值必须是常量AR_LONG（提取整数）或 AR_STR（提取字符串）。
- `long DelArrayElement(long tag, long id, long idx)`。从指定数组中删除指定数组位置 的内容。tag 的值决定是删除指定索引位置的整数值还是字符串值。
- `void DeleteArray(long id)`。删除按id 引用的数组及其所有相关内容。创建一个数组后， 即使一个脚本终止，它也继续存在，直到调用DeleteArray 从创建它的数据库中删除这个 数组。
- `long RenameArray(long id, string newname)`。将按id 引用的数组重命名为newname。如 果操作成功，将返回1，否则返回0。

全局数组的作用包括模拟全局变量、模拟复杂的数据类型、为所有脚本调用提供永久存储。 在数组开始执行时创建一个全局数组，然后将全局值存储到这个数组中，即可模拟这个数组的全局变量。要共享这些全局值，可以将数组句柄传递给要求访问这些值的函数，或者要求请求访问这些值的函数对相关数组进行名称查询。

IDC 全局数组中存储的值会在执行脚本的数据库中永久存在。你可以通过检查 CreateArray 函数的返回值，测试一个数组是否存在。如果一个数组中存储的值仅适用于某个特定的脚本，那么，在这个脚本终止前，应该删除该数组。删除数组可以确保全局值不会由同一个脚本的上一次执行传递到随后的执行中。

## 关联 IDC 脚本与热键

IDA 提供了一种分配热键的简单方法。每次启动 IDA，它都会执行 `<IDADIR>/idc/ida.idc` 中的脚本。为了将热键与脚本关联起来，你需要在 ida.idc 文件中添加两行代码。在第一行代码中，必须添加一个 include 指令，将脚本文件包含在 ida.idc 文件中。在第二行代码中，必须在 main 函数中添加一个对 AddHotkey 函数的调用，将特定的热键与 IDC 脚本关联起来。修改后的 ida.idc 文件如下所示。

```c
#include <idc.idc>
#include <my_amazing_script.idc>
static main() {
  AddHotkey("z", "MyAmazingFunc"); //Now 'z' invokes MyAmazingFunc
}
```

如果你尝试与脚本关联的热键已经分配给另一项 IDA 操作（菜单热键或插件激活热键），这时，AddHotkey  函数将悄无声息地失败，除了在你按下热键组合后，函数不会运行外，你无法通 过其他方式检测到这种失败。

这里你需要记住两个要点：第一，IDC 脚本的标准存储目录为`<IDADIR>/idc` ；第二，不能将脚本函数命名为main 。如果希望 IDA 能够轻易找到脚本，可以将它复制到`<IDADIR>/idc`  目录中。如果要将脚本文件存储在其他位置，你需要在 include 语句中指定脚本的完整路径。在测试脚本时，使用 main 函数以独立程序的方式运行脚本会有好处。但是，一旦你准备将脚本与热键关联起来，就不能使用 main 这个名称，因为它会与 ida.idc 中的 main 函数相互冲突。必须重命名 main 函数，并在调用 AddHotkey 时使用新的名称。

## 有用的IDC 函数

我们将介绍一些有用（根据我们的经验）的 IDC 函数，并根据功能对它们分类。即使你只计划使用 Python 编写脚本，了解下面这些函数仍会对你有所帮助，因为  IDAPython 为这里的每一个函数提供了对应的 Python 函数。

### 读取和修改数据的函数

- `long Byte(long addr)`，从虚拟地址 addr 处读取一个字节值。
- `long Word(long addr)`，从虚拟地址 addr 处读取一个字（2 字节）值。
- `long Dword(long addr)`，从虚拟地址 addr 处读取一个双字（4 字节）值。
- `void PatchByte(long addr, long val)`，设置虚拟地址 addr 处的一个字节值。
- `void PatchWord(long addr, long val)`，设置虚拟地址 addr 处的一个字值。
- `void PatchDword(long addr, long val)`，设置虚拟地址 addr 处的一个双字值。
- `bool isLoaded(long addr)`，如果 addr 包含有效数据，则返回1，否则返回0。

在读取和写入数据库时，这里的每一个函数都考虑到了当前处理器模块的字节顺序（小端或大端）。PatchXXX  函数还根据被调用的函数，通过仅使用适当数量的低位字节，将所提供的值调整到适当大小。例如，调用PatchByte(0x401010, 0x1234) 将使用字节值 0x34（0x1234 的低位字节）修改 0x401010 位置。如果在用 Byte 、Word 和 Dword 读取数据库时提供了一个无效的地址，它们将分别返回值 0xFF、0xFFFF 和0xFFFFFFF。因为你没有办法将这些错误值与存储在数据库中的合法值区分开来，因此，在尝试从数据库中的某个地址读取数据之前，你可能希望调用 isLoaded 函数，以确定这个地址是否包含任何数据。

由于 IDA 在刷新反汇编窗口时“行为古怪”，你可能会发现，修补操作的结果并不会立即在窗口中显示出来。这时，你可以拖动滚动带离开被修补的位置，然后返回这个位置，即可迫使窗口正确进行更新。

### 用户交互函数

为了进行用户交互，需要熟悉IDC  的输入/ 输出函数。下面详细介绍IDC  的一些重要的接口函数。

- `void Message(string format, ...)`，在输出窗口打印一条格式化消息。这个函数类似于 C 语言的 printf 函数，并接受 printf 风格的格式化字符串。
- `void print(...)`，在输出窗口中打印每个参数的字符串表示形式。
- `void Warning(string format, ...)`，在对话框中显示一条格式化消息。
- `string AskStr(string default, string prompt)`，显示一个输入框，要求用户输入一个字符串值。如果操作成功，则返回用户的字符串；如果对话框被取消，则返回0。
- `string AskFile(long doSave, string mask, string prompt)`，显示一个文件选择对话框，以简化选择文件的任务。你可以创建新文件保存数据（doSave=1），或选择现有的文件读取数据（doSave=0）。你可以根据mask（如*.*或*.idc）过滤显示的文件列表。如果操作成功，则返回选定文件的名称；如果对话框被取消，则返回0。
- `long AskYN(long default, string prompt)`，用一个答案为“是”或“否”的问题提示用户，突出一个默认的答案（1 为是，0 为否，-1 为取消）。返回值是一个表示选定答案的整数。
- `long ScreenEA()`，返回当前光标所在位置的虚拟地址。
- `bool Jump(long addr)`，跳转到反汇编窗口的指定地址。

因为 IDC 没有任何调试工具，你可能需要将 Message 函数作为你的主要调试工具。其他几个 AskXXX 函数用于处理更加专用的输入，如整数输入。请参考帮助系统文档了解可用的 AskXXX 函数的完整列表。如果希望创建一个根据光标位置调整其行为的脚本，这时，ScreenEA 函数就非常有用，因为你可以通过它确定光标的当前位置。同样，如果你的脚本需要将用户的注意力转移到反汇编代码清单中的某个位置，也需要用到 Jump 函数。

### 字符串操纵函数

 虽然简单的字符串赋值和拼接操作可以通过IDC  中的基本运算符实现，但是，更加复杂的 操作必须使用字符串操纵函数实现，这些函数如下所示。

- `string form(string format, ...)` //preIDA5.6，返回一个新字符串，该字符串根据所提供的格式化字符串和值进行格式化。这个函数基本上等同于C 语言的sprintf 函数。
- `string sprintf(string format,...)` //IDA5.6+，在IDA5.6 中，sprintf 用于替代form（参见上面）。
- `long atol(string val)`，将十进制值val 转换成对应的整数值。
- `long xtol(string val)`，将十六进制值val（可选择以0x 开头）转换成对应的整数值。
- `string ltoa(long val, long radix)`，以指定的radix（2、8、10 或16）返回val 的字符串值。
- `long ord(string ch)`，返回单字符字符串ch 的ASCII 值。
- `long strlen(string str)`，返回所提供字符串的长度。
- `long strstr(string str, string substr)`，返回str 中substr 的索引。如果没有发现子字符串，则返回-1。
- `string substr(string str, long start, long end)`，返回包含str 中由start 到end-1位置的字符的子字符串。如果使用切片（ IDA5.6 及更高版本）， 此函数等同于`str[start:end]`。

如前所述，IDC 中没有任何字符数据类型，它也不支持任何数组语法。如果你想要遍历字符串的每个字符，必须把字符串中的每个字符当成连续的单字符子字符串处理。

### 文件输入/输出函数

输出窗口并不总是显示脚本输出的理想位置。对于生成大量文本或二进制数据的脚本，你可能希望将其结果输出到磁盘文件上。我们已经讨论了如何使用AskFile  函数要求用户输入文件名。 但是，AskFile  仅返回一个包含文件名的字符串值。IDC  的文件处理函数如下所示。

- `long fopen(string filename, string mode)`，返回一个整数文件句柄（如果发生错误，则返回0），供所有IDC 文件输入/输出函数使用。mode 参数与C 语言的fopen 函数使用的模式（r 表示读取，w 表示写入，等等）类似。
- `void fclose(long handle)`，关闭fopen 中文件句柄指定的文件。
- `long filelength(long handle)`，返回指定文件的长度，如果发生错误，则返回-1。
- `long fgetc(long handle)`，从给定文件中读取一个字节。如果发生错误，则返回-1。
- `long fputc(long val, long handle)`，写入一个字节到给定文件中。如果操作成功，则返回0；如果发生错误，则返回1。
- `long fprintf(long handle, string format, ...)`，将一个格式化字符串写入到给定文件中。
- `long writestr(long handle, string str)`，将指定的字符串写入到给定文件中。
- `string/long readstr(long handle)`，从给定文件中读取一个字符串。这个函数读取到下一个换行符为止的所有字符（包括非ASCII 字符），包括换行符本身（ASCII 0xA）。如果操作成功，则返回字符串；如果读取到文件结尾，则返回1。
- `long writelong(long handle, long val, long bigendian)`，使用大端（bigendian=1）或小端（bigendian=0）字节顺序将一个4 字节整数写入到给定文件。
- `long readlong(long handle, long bigendian)`，使用大端（bigendian=1）或小端（bigendian=0）字节顺序从给定的文件中读取一个4 字节整数。
- `long writeshort(long handle, long val, long bigendian)`，使用大端（bigendian=1）或小端（bigendian=0）字节顺序将一个2 字节整数写入到给定的文件。
- `long readshort(long handle, long bigendian)`，使用大端（bigendian=1）或小端（bigendian=0）字节顺序从给定的文件中读取一个2 字节整数。
- `bool loadfile(long handle, long pos, long addr, long length)`，从给定文件的pos 位置读取length 数量的字节，并将这些字节写入到以addr 地址开头的数据库中。
- `bool savefile(long handle, long pos, long addr, long length)`，将以addr 数据库地址开头的length 数量的字节写入给定文件的pos 位置。

### 操纵数据库名称

在脚本中，你经常需要操纵已命名的位置。下面的IDC  函数用于处理IDA  数据库中已命名的位置。

- `string Name(long addr)`，返回与给定地址有关的名称，如果该位置没有名称，则返回空字符串。如果名称被标记为局部名称，这个函数并不返回用户定义的名称。
- `string NameEx(long from, long addr)`，返回与addr 有关的名称。如果该位置没有名称则返回空字符串。如果from 是一个同样包含addr 的函数中的地址，则这个函数返回用户定义的局部名称。
- `bool MakeNameEx(long addr, string name, long flags)`，将给定的名称分配给给定的地址。该名称使用flags 位掩码中指定的属性创建而成。这些标志在帮助系统中的 MakeNameEx 文档中有记载描述，可用于指定各种属性，如名称是局部名称还是公共名称名称是否应在名称窗口中列出。
- `long LocByName(string name)`，返回一个位置（名称已给定）的地址。如果数据库中没有这个名称，则返回BADADDR（-1）。
- `long LocByNameEx(long funcaddr, string localname)`，在包含funcaddr 的函数中搜索给定的局部名称。如果给定的函数中没有这个名称，则返回BADADDR（-1）。

### 处理函数的函数

许多脚本专用于分析数据库中的函数。IDA 为经过反汇编的函数分配大量属性，如函数局部变量区域的大小、函数的参数在运行时栈上的大小。下面的IDC 函数可用于访问与数据库中的函数有关的信息。

- `long GetFunctionAttr(long addr, long attrib)`，返回包含给定地址的函数的被请求的属性。请参考IDC 帮助文档了解属性常量。例如，要查找一个函数的结束地址，可以使用`GetFunctionAttr(addr, FUNCATTR_END)`;。
- `string GetFunctionName(long addr)`，返回包含给定地址的函数的名称。如果给定的地址并不属于一个函数，则返回一个空字符串。
- `long NextFunction(long addr)`，返回给定地址后的下一个函数的起始地址。如果数据库中给定地址后没有其他函数，则返回-1。
- `long PrevFunction(long addr)`，返回给定地址之前距离最近的函数的起始地址。如果在给定地址之前没有函数，则返回-1。

 根据函数的名称，使用LocBy Name  函数查找该函数的起始地址。

### 代码交叉引用函数

IDC 提供各种函数来访问与指令有关的交叉引用信息。要确定哪些函数能够满足你的脚本的要求，可能有些令人困惑。它要求你确定：你是否有兴趣跟从离开给定地址的流，是否有兴趣迭代引用给定地址的所有位置。下面我们将介绍执行上述两种操作的函数。其中几个函数用于支持对一组交叉引用进行迭代。这些函数支持交叉引用序列的 概念，并需要一个 current 交叉引用，以返回一个 next 交叉引用。

- `long Rfirst(long from)`，返回给定地址向其转交控制权的第一个位置。如果给定的地址没有引用其他地址，则返回BADADDR（-1）。
- `long Rnext(long from, long current)`，如果current 已经在前一次调用Rfirst 或Rnext时返回，则返回给定地址（from）转交控制权的下一个位置。如果没有其他交叉引用存在，则返回BADADDR。
- `long XrefType()`，返回一个常量，说明某个交叉引用查询函数（如Rfirst）返回的最后一个交叉引用的类型。对于代码交叉引用，这些常量包括 fl_CN（近调用）、fl_CF（远调用）、fl_JN（近跳转）、fl_JF（远跳转）和 fl_F（普通顺序流）。 
- `long RfirstB(long to)`，返回转交控制权到给定地址的第一个位置。如果不存在对给定地址的交叉引用，则返回BADADDR（-1）。
- `long RnextB(long to, long current)`，如果current 已经在前一次调用RfirstB或RnextB时返回，则返回下一个转交控制权到给定地址（to）的位置。如果不存在其他对给定位置的交叉引用，则返回BADADDR（-1）。

每次调用一个交叉引用函数，IDA  都会设置一个内部IDC  状态变量，指出返回的最后一个交 叉引用的类型。如果需要知道你收到的交叉引用的类型，那么在调用其他交叉引用查询函数之前， 必须调用XrefType  函数。

### 数据交叉引用函数

访问数据交叉引用信息的函数与访问代码交叉引用信息的函数非常类似。这些函数如下所示。

- `long Dfirst(long from)`，返回给定地址引用一个数据值的第一个位置。如果给定地址没有引用其他地址，则返回BADADDR（-1）。
- `long Dnext(long from, long current)`，如果current 已经在前一次调用Dfirst 或Dnext时返回，则返回给定地址（from）向其引用一个数据值的下一个位置。如果没有其他交叉引用存在，则返回BADADDR。
- `long XrefType()`，返回一个常量，说明某个交叉引用查询函数（如Dfirst）返回的最后一个交叉引用的类型。对于数据交叉引用，这些常量包括dr_0（提供的偏移量）、dr_W（数据写入）和dr_R（数据读取）。
- `long DfirstB(long to)`，返回将给定地址作为数据引用的第一个位置。如果不存在引用给定地址的交叉引用，则返回BADADDR（--1）。
- `long DnextB(long to, long current)`，如果currnet 已经在前一次调用DfristB 或DnextB时返回，则返回将给定地址（to）作为数据引用的下一次位置。如果没有其他对给定地址的交叉引用存在，则返回BADADDR。

和代码交叉引用一样，如果需要知道你收到的交叉引用的类型，那么在调用另一个交叉引用 查询函数之前，必须调用XrefType  函数。

### 数据库操纵函数

有大量函数可用于对数据库的内容进行格式化。这些函数如下所示。

- `void MakeUnkn(long addr, long flags)`，取消位于指定地址的项的定义。这里的标志（参见IDC 的MakeUnkn 文档）指出是否也取消随后的项的定义，以及是否删除任何与取消定义的项有关的名称。相关函数MakeUnknown 允许你取消大块数据的定义。
- `long MakeCode(long addr)`，将位于指定地址的字节转换成一条指令。如果操作成功，则返回指令的长度，否则返回0。
- `bool MakeByte(long addr)`，将位于指定地址的项目转换成一个数据字节。类似的函数还包括MakeWord 和MakeDword。
- `bool MakeComm(long addr, string comment)`，在给定的地址处添加一条常规注释。
- `bool MakeFunction(long begin, long end)`，将由begin 到end 的指令转换成一个函数。如果end 被指定为BADADDR（-1），IDA 会尝试通过定位函数的返回指令，来自动确定该函数的结束地址。
- `bool MakeStr(long begin, long end)`，创建一个当前字符串（由GetStringType 返回）类型的字符串，涵盖由begin 到end-1 之间的所有字节。如果end 被指定为BADADDR，IDA会尝试自动确定字符串的结束位置。

有许多其他MakeXXX  函数可提供类似于上述函数的操作。请参考IDC  文档资料了解所有这些 函数。

### 数据库搜索函数

在 IDC 中，IDA 的绝大部分搜索功能可通过各种 FindXXX 函数来实现，下面我们将介绍其中一些函数。FindXXX 函数中的 flags 参数是一个位掩码，可用于指定查找操作的行为。3 个最为常用的标志分别为 SEARCH_DOWN ，它指示搜索操作向更高的地址扫描；SEARCH_NEXT ，它略 过当前匹配项，以搜索下一个匹配项；SEARCH_CASE ，它以区分大小写的方式进行二进制和文本搜索。

- `long FindCode(long addr, long flags)`，从给定的地址搜索一条指令。
- `long FindData(long addr, long flags)`，从给定的地址搜索一个数据项。
- `long FindBinary(long addr, long flags, string binary)`，从给定的地址搜索一个字节序列。字符串binary 指定一个十六进制字节序列值。如果没有设置SEARCH_CASE，且一个字节值指定了一个大写或小写ASCII 字母，则搜索仍然会匹配对应的互补值。例如，“41 42”将匹配“61 62”（和“61 42”），除非你设置了SEARCH_CASE 标志。
- `long FindText(long addr, long flags, long row, long column, string text)`，在给定的地址，从给定行（row）的给定列搜索字符串text。注意，某个给定地址的反汇编文本可能会跨越几行，因此，你需要指定搜索应从哪一行开始。

还要注意的是，SEARCH_NEXT 并未定义搜索的方向，根据 SEARCH_DOWN  标志，其方向可能向 上也可能向下。此外，如果没有设置 SEARCH_NEXT 且 addr 位置的项与搜索条件匹配，则 FindXXX 函数很可能会返回 addr 参数传递给该函数的地址。

### 反汇编行组件

许多时候，我们需要从反汇编代码清单的反汇编行中提取出文本或文本的某个部分。下面的 函数可用于访问反汇编行的各种组件。

- `string GetDisasm(long addr)`，返回给定地址的反汇编文本。返回的文本包括任何注释，但不包括地址信息。
- `string GetMnem(long addr)`，返回位于给定地址的指令的助记符部分。
- `string GetOpnd(long addr, long opnum)`，返回指定地址的指定操作数的文本形式。IDA以零为起始编号，从左向右对操作数编号。
- `long GetOpType(long addr, long opnum)`，返回一个整数，指出给定地址的给定操作数的类型。请参考GetOpType 的IDC 文档，了解操作数类型代码。
- `long GetOperandValue(long addr, long opnum)`，返回与给定地址的给定操作数有关的整数值。返回值的性质取决于GetOpType 指定的给定操作数的类型。
- `string CommentEx(long addr, long type)`，返回给定地址处的注释文本。如果type 为0，则返回常规注释的文本；如果type 为1，则返回可重复注释的文本。如果给定地址处没有注释，则返回一个空字符串。

## IDC 脚本示例

### 枚举函数

许多脚本针对各个函数进行操作。例如，生成以某个特定函数为根的调用树，生成一个函数的控制流程图，或者分析数据库中每个函数的栈帧。下面代码中的脚本遍历数据库中的每 一个函数，并打印出每个函数的基本信息，包括函数的起始和结束地址、函数参数的大小、函数的局部变量的大小。所有输出全部在输口窗口中显示。

```c
#include <idc.idc>

static main() {
  auto addr, end, args, locals, frame, firstArg, name, ret, filename, fp;

  filename = "~/Desktop/Output.txt";
  fp = fopen(filename, "w");
  if (fp) {
    print("open successfully! fp: %x = ", fp);
  } else {
    print("open error!");
  }

  print("running......");
  addr = 0;
  for (addr = NextFunction(addr); addr != BADADDR; addr = NextFunction(addr)) {
    name = Name(addr);
    end = GetFunctionAttr(addr, FUNCATTR_END);
    locals = GetFunctionAttr(addr, FUNCATTR_FRSIZE);
    frame = GetFrame(addr); // retrieve a handle to the function’s stack frame
    ret = GetMemberOffset(frame, " r"); // " r" is the name of the return address
    if (ret == -1) continue;
    firstArg = ret + 4;
    args = GetStrucSize(frame) - firstArg;

    fprintf(fp, "Function: %s, starts at %x, ends at %x\n", name, addr, end);
    fprintf(fp, " Local variable area is %d bytes\n", locals);
    fprintf(fp, " Arguments occupy %d bytes (%d args)\n", args, args / 4);
  }
  print("complete successfully!");
  fclose(fp);
}
```

这个脚本使用IDC 的一些结构操纵函数，以获得每个函数的栈帧的句柄（GetFrame ），确定栈帧的大小（GetStrucSize），并确定栈中保存的返回地址的偏移量（GetMemberOffset ）。 函数的第一个参数占用保存的返回地址后面的4 个字节。函数的参数部分的大小为第一个参数与栈帧结束部分之间的空间。由于 IDA 无法为导入的函数生成栈帧，这个脚本检查函数的栈帧中是否包含一个已保存的返回地址，以此作为一种简单的方法，确定对某个导入函数的调用。

### 枚举指令

你可能想要枚举给定函数中的每一条指令。代码如下，可用于计算光标当前所 在位置的函数所包含的指令的数量。

```c
#include <idc.idc>
static main() {
  auto func, end, count, inst;
  // 1
  func = GetFunctionAttr(ScreenEA(), FUNCATTR_START);
  if (func != -1) {
    // 2
    end = GetFunctionAttr(func, FUNCATTR_END);
    count = 0;
    inst = func;
    while (inst < end) {
      count++;
      // 3
      inst = FindCode(inst, SEARCH_DOWN | SEARCH_NEXT);
    }
    Warning("%s contains %d instructions\n", Name(func), count);
  }
  else {
    Warning("No function found at location %x", ScreenEA());
  }
}
```

 这个函数从 1 处开始，它使用 GetFunctionAttr 确定包含光标地址（ScreenEA() ）的函数的起始地址。如果确定了一个函数的起始地址，下一步 2 是再次使用 GetFunctionAttr 确定该函数的结束地址。确定该函数的边界后，接下来执行一个循环，使用 FindCode 函数（ 3 ）的搜索功能，逐个识别函数中的每一条指令。在这个例子中， Warning 函数用于显示结果，因为这个函数仅仅生成一行输出，而在警告对话框中显示输出，要比在消息窗口中显示输出更加明显。请注意，这个例子假定给定函数中的所有指令都是相邻的。另一种方法可以替代 FindCode 来遍历函数中每条指令的所有代码交叉引用。只要编写适当的脚本，你就可以采用这种方法来处理非相邻的函数（也称为“分块”函数）。

### 枚举交叉引用

由于可用于访问交叉引用数据的函数的数量众多，以及代码交叉引用的双向性，如何遍历交叉引用可能会令人困惑。为了获得你想要的数据，你必须确保自己访问的是适合当前情况的正确交叉引用类型。在下面的交叉引用示例中，我们遍历函数中的每一条指令，确定这些指令是否调用了其他函数，从而获得该函数所做的全部函数调用。要完成这个任务，一个方法是解析 GetMnem 的结果，从中查找 call 指令。但是，这种方法并不是非常 方便，因为用于调用函数的指令因 CPU 类型而异。此外，要确定到底是哪一个函数被调用，你还需要进行额外的解析。使用交叉引用则可以免去这些麻烦，因为它们独立于CPU ，能够直接告诉我们交叉引用的目标。

```c
#include <idc.idc>
static main() {
  auto func, end, target, inst, name, flags, xref;
  // SEARCH_NEXT: useful only for find_text() and find_binary() 
  // for other Find.. functions it is implicitly set
  flags = SEARCH_DOWN | SEARCH_NEXT;
  func = GetFunctionAttr(ScreenEA(), FUNCATTR_START);
  if (func != -1) {
    name = Name(func);
    end = GetFunctionAttr(func, FUNCATTR_END);
    for (inst = func; inst < end; inst = FindCode(inst, flags)) {
      for (target = Rfirst(inst); target != BADADDR; target = Rnext(inst, target)) {
        xref = XrefType();
        if (xref == fl_CN || xref == fl_CF) {
          Message("%s calls %s from 0x%x\n", name, Name(target), inst);
        }
      }
    }
  }
  else {
    Warning("No function found at location %x", ScreenEA());
  }
}
```

在这个例子中，必须遍历函数中的每条指令。然后，对于每一条指令，我们必须遍历从 它们发出的每一个交叉引用。我们仅仅对调用其他函数的交叉引用感兴趣，因此，我们必须 检查XrefType 的返回值，从中查找 fl_CN 或 fl_CF 类型的交叉引用。同样，这个特殊的解决方案只能处理包含相邻指令的函数。由于这段脚本已经遍历了每条指令的交叉引用，因此我们不需要进行太大的更改，就可以使用这段脚本进行基于流程的分析，而不是上面的基于地址的分析。

另外，交叉引用还可用于确定引用某个位置的每一个位置。例如，如果希望创建一个低成本的安全分析器，我们可能会有兴趣监视对 strcpy 和 sprintf 等函数的所有调用。

在下面的例子中，如代码清单 15-4 所示，我们逆向遍历对某个符号（相对于前一个例子中 的“发出引用”）的所有交叉引用。

```c
#include <idc.idc>
static list_callers(bad_func) {
  auto func, addr, xref, source;
  // 1
  func = LocByName(bad_func);
  if (func == BADADDR) {
    Warning("Sorry, %s not found in database", bad_func);
  }
  else {
    // 2
    for (addr = RfirstB(func); addr != BADADDR; addr = RnextB(func, addr)) {
      // 3
      xref = XrefType();
      // 4
      if (xref == fl_CN || xref == fl_CF) {
        // 5
        source = GetFunctionName(addr);
        // 6                         
        Message("%s is called from 0x%x in %s\n", bad_func, addr, source);
      }
    }
  }
}

static main() {
  list_callers("_strcpy");
  list_callers("_sprintf");
}
```

 在这个例子中，LocByName (1) 函数用于查找一个给定的（按名称）bad 函数的地址。如果发现这个函数的地址，则执行一个循环 (2)，处理对这个 bad 函数的所有交叉引用。对于每一个交叉引用，如果确定了交叉引用类型 (3) 为调用类型 (4) ，则确定实施调用的函数的名称 (5) ，并向用户显示这个名称 (6) 。

需要注意的是，要正确确定一个导入函数的名称，你可能需要做出一些修改。具体来说， 在 ELF 可执行文件中（这种文件结合一个过程链接表（PLT）和一个全局偏移量表（GOT）来处理共享库链接），IDA 分配给导入函数的名称可能并不十分明确。例如，一个 PLT 条目似乎名为 _memcpy ，但实际上它叫做 .memcpy ；IDA 用下划线替换了点，因为在 IDA 名称中，点属于无效字符。使问题更加复杂的是，IDA 可能只是创建了一个名为 memcpy 的符号，该符号位于一个 IDA 称为extern 的节内。在尝试枚举对 memcpy 的交叉引用时，我们会对这个符号的 PLT 版本感兴趣，因为它是程序中其他函数调用的版本，因此也是所有交叉引用引用的版本。

### 枚举导出的函数

 。IDC  提供了一些函 数，用于遍历共享库导出的函数。下面的脚本如代码清单15-5  所示，可在 IDA 打开一个共享库后生成一个 .idt 文件。

```c
#include <idc.idc>
static main() {
  auto entryPoints, i, ord, addr, name, purged, file, fd;
  file = AskFile(1, "*.idt", "Select IDT save file");
  fd = fopen(file, "w");
  entryPoints = GetEntryPointQty();
  fprintf(fd, "ALIGNMENT 4\n");
  fprintf(fd, "0 Name=%s\n", GetInputFile());
  for (i = 0; i < entryPoints; i++) {
    ord = GetEntryOrdinal(i);
    if (ord == 0) continue;
    addr = GetEntryPoint(ord);
    if (ord == addr) {
      continue; //entry point has no ordinal
    }
    name = Name(addr);
    fprintf(fd, "%d Name=%s", ord, name);
    purged = GetFunctionAttr(addr, FUNCATTR_ARGSIZE);
    if (purged > 0) {
      fprintf(fd, " Pascal=%d", purged);
    }
    fprintf(fd, "\n");
  }
}
```

这个脚本的输出保存在用户指定的文件中。这段脚本引入的新函数包括 GetEntryPointQty ， 它返回库导出的符号的数量；GenEntryOrdinal ，它返回一个序号（库的导出表的索引）； GetEntryPoint ，它返回与一个导出函数关联的地址（该函数通过序号标识）；GetInputFile ，它返回加载到IDA 中的文件的名称。

### 查找和标记函数参数

调用一个函数之前，在x86  二进制文件中，3.4  之后的GCC  版本一直使用mov  语句（而非push  语句）将函数参数压入栈上。由于IDA  的分析引擎依靠查找push  语句来确定函数调用中压入函 数参数的位置，这给IDA  的分析造成了一些困难（IDA  的更新版本可以更好地处理这种情况）。 下面显示的是向栈压入参数时的IDA  反汇编代码清单：

```assembly
.text:08048894 push 0 ; protocol
.text:08048896 push 1 ; type
.text:08048898 push 2 ; domain
.text:0804889A call _socket
```

请注意每个反汇编行右侧的注释。只有在IDA  认识到参数正被压入，且IDA  知道被调用函 数的签名时，这些注释才会显示。如果使用mov  语句将参数压入栈中，得到的反汇编代码清单提 供的信息会更少，如下所示：

```assembly
.text:080487AD mov [esp+8], 0
.text:080487B5 mov [esp+4], 1
.text:080487BD mov [esp], 2
.text:080487C4 call _socket
```

可见，IDA  并没有认识到，在函数被调用之前，有3  个mov  语句被用于为函数调用设置参数。 因此，IDA  无法在反汇编代码清单中以自动注释的形式为我们提供更多信息。

使用一个脚本恢复我们经常在反汇编代码清单中看到的信息。下列脚本努力自动识别为函数调用设置参数的指令。

```c
#include <idc.idc>
static main() {
  auto addr, op, end, idx;
  auto func_flags, type, val, search;
  search = SEARCH_DOWN | SEARCH_NEXT;
  addr = GetFunctionAttr(ScreenEA(), FUNCATTR_START);
  func_flags = GetFunctionFlags(addr);
  if (func_flags & FUNC_FRAME) { //Is this an ebp-based frame?
    end = GetFunctionAttr(addr, FUNCATTR_END);
    for (; addr < end && addr != BADADDR; addr = FindCode(addr, search)) {
      type = GetOpType(addr, 0);
      if (type == 3) { //Is this a register indirect operand?
        if (GetOperandValue(addr, 0) == 4) { //Is the register esp?
          MakeComm(addr, "arg_0"); //[esp] equates to arg_0
        }
      }
      else if (type == 4) { //Is this a register + displacement operand?
        idx = strstr(GetOpnd(addr, 0), "[esp"); //Is the register esp?
        if (idx != -1) {
          val = GetOperandValue(addr, 0); //get the displacement
          MakeComm(addr, form("arg_%d", val)); //add a comment
        }
      }
    }
  }
}
```

这个脚本仅针对基于EBP  的帧，并依赖于此：在函数被调用之前，当参数被压入栈中时， GCC  会生成与esp  相应的内存引用。该脚本遍历函数中的所有指令。对于每一条使用esp  作为基 址寄存器向内存位置写入数据的指令，该脚本确定上述内存位置在栈中的深度，并添加一条注释， 指出被压入的是哪一个参数。GetFunctionFlags  函数提供了与函数关联的各种标志，如该函数是 否使用一个基于EBP  的栈帧。运行代码清单15-6  中的脚本，将得到一个包含注释的反汇编代码 清单，如下所示：

```assembly
.text:080487AD mov [esp+8], 0 ; arg_8
.text:080487B5 mov [esp+4], 1 ; arg_4
.text:080487BD mov [esp], 2 ; arg_0
.text:080487C4 call _socket
```

这里的注释并没有提供特别有用的信息。但是，现在，我们可以一眼看出，程序使用了3  个 mov 语句在栈上压入参数，这使我们朝正确的方向又迈进了一步。进一步扩充上述脚本，并利用 IDC  的其他一些功能，我们可以得到另一个脚本，它提供的信息几乎和IDA  在正确识别参数时提 供的信息一样多。这个新脚本的最终输出如下所示：

```assembly
.text:080487AD mov [esp+8], 0 ; int protocol
.text:080487B5 mov [esp+4], 1 ; int type
.text:080487BD mov [esp], 2 ; int domain
.text:080487C4 call _socket
```

脚本的扩充版本请参见与本书有关的[网站](http://www.idabook.com/ch15_examples) ，该脚本能够将函数签名中的数据合并到注释中。

### 模拟汇编语言行为

出于许多原因，你可能需要编写一段脚本，模拟你所分析的程序的行为。例如，你正在分析的程序可能和许多恶意程序一样，属于自修改程序，该程序也可能包含一些在运行时根据需要解码的编码数据。如果不运行该程序，并从正在运行的进程的内存中提取出被修改的数据，你如何了解这个程序的行为呢？ IDC 脚本或许可以帮你解决这个问题。如果解码过程不是特别复杂，你可以迅速编写出一个 IDC 脚本，执行和程序运行时执行的操作。如果你 不知道程序的作用，也没有可供该程序运行的平台，使用一个脚本以这种方式解码数据，你不需运行程序即可获得相关信息。如果你正使用 Windows 版本的 IDA 分析一个 MIPS 二进制文件，可能会出现上述后一种情况。没有任何 MIPS 硬件，你将无法运行这个 MIPS 二进制文件，观察它执行的任何数据解码任务。但是，你可以编写一个 IDC 脚本来模拟这个二进制文件的行为，并对 IDA 数据库进行必要的修改，所有这一切根本不需要在 MIPS 执行环境中进行。

 下面的 x86 代码摘自[DEFCON](http://www.defcon.org/) 的一个“夺旗赛”（由 DEFCON 15 CTF 的组织者Kenshoto 提供。“夺旗赛”是DEFCON 每年举办的一项黑客竞赛。） 二进制文件。

```assembly
.text:08049EDE mov [ebp+var_4], 0
.text:08049EE5
.text:08049EE5 loc_8049EE5:
.text:08049EE5 cmp [ebp+var_4], 3C1h
.text:08049EEC ja short locret_8049F0D
.text:08049EEE mov edx, [ebp+var_4]
.text:08049EF1 add edx, 804B880h
.text:08049EF7 mov eax, [ebp+var_4]
.text:08049EFA add eax, 804B880h
.text:08049EFF mov al, [eax]
.text:08049F01 xor eax, 4Bh
.text:08049F04 mov [edx], al
.text:08049F06 lea eax, [ebp+var_4]
.text:08049F09 inc dword ptr [eax]
.text:08049F0B jmp short loc_8049EE5
```

这段代码用于解码一个植入到程序二进制文件中的私钥。使用下面的 IDC脚本，不必运行程序就可以提取出这个私钥。

```c
auto var_4, edx, eax, al;
var_4 = 0;
while (var_4 <= 0x3C1) {
  edx = var_4;
  edx = edx + 0x804B880;
  eax = var_4;
  eax = eax + 0x804B880;
  al = Byte(eax);
  al = al ^ 0x4B;
  PatchByte(edx, al);
  var_4++;
}
```

代码清单15-7  只是对前面汇编语言代码（根据以下相当机械化的规则生成）的直接转换。

1. 为汇编代码中的每一个栈变量和寄存器声明一个 IDC 变量。

2. 为每一个汇编语言语句编写一个模拟其行为的 IDC 语句。
3. 通过读取和写入在 IDC 脚本中声明的对应变量，模拟读取和写入栈变量。
4. 根据被读取数据的数量（ 1 字节、2 字节或 4 字节），使用 Byte 、Word 或 Dword 函数从一个非栈位置读取数据。
5. 根据被写入数据的数量，使用 PatchByte 、PatchWord 或 PatchDword 函数向一个非栈位置写入数据。
6. 通常，如果代码中包含一个终止条件不十分明确的循环，那么，模拟程序行为的最简单方法是首先使用一个无限循环（如 while(1) {} ），然后在遇到使循环终止的语句时插入一个 break 语句。
7. 如果汇编代码调用函数，问题就变得更加复杂。为了正确模拟汇编代码的行为，你必须设法模拟被调用的函数的行为，包括提供一个被模拟的代码的上下文认可的返回值。仅仅这个事实可能就使得你无法使用 IDC 来模拟汇编语言程序的行为。

在编写和上面的脚本类似的脚本时，需要注意的是，有时候，你并不一定非要从整体上完全了解你所模拟的代码的行为。通常，一次理解一两条指令，并将这些指令正确转换成对应的 IDC 脚本就足够了。如果每一条指令都正确转换成 IDC 脚本，那么，整个脚本将能够正确模拟最初的汇编代码的全部功能。我们可以推迟分析汇编语言算法，直到 IDC 脚本编写完成，到那时，我们就可以利用 IDC 脚本深化对基本汇编代码的理解。了解上面示例中算法的工作机制后，我们可以将那个 IDC 脚本缩短成下面的脚本：

```c
auto var_4, addr;
for (var_4 = 0; var_4 <= 0x3C1; var_4++) {
	addr = 0x804B880 + var_4;
	PatchByte(addr, Byte(addr) ^ 0x4B);
}
```

另外，如果不希望以任何方式修改数据库，在处理 ASCII 数据时，我们可以用 Message 函数代替PatchByte  函数，或者在处理二进制数据时，将数据写入到一个文件中。

---

#IDAPython

##参考

- [x] [IDAPython/intro](http://magiclantern.wikia.com/wiki/IDAPython/intro)
- [x] [IDAPython/Backtracing](http://magiclantern.wikia.com/wiki/IDAPython/Backtracing)
- [ ] [IDAPython/Tracing calls tutorial](http://magiclantern.wikia.com/wiki/IDAPython/Tracing_calls_tutorial)
- [x] [IDAPython：让你的生活更美好（一）](http://www.freebuf.com/sectool/92107.html) [源文](https://researchcenter.paloaltonetworks.com/2015/12/using-idapython-to-make-your-life-easier-part-1/)
- [ ] [IDAPython：让你的生活更美好（二）](http://www.freebuf.com/sectool/92168.html) 
- [ ] [IDAPython：让你的生活更美好（三）](http://www.freebuf.com/articles/system/92488.html) 
- [ ] [IDAPython：让你的生活更美好（四）](http://www.freebuf.com/articles/system/92505.html) 
- [ ] [IDAPython：让你的生活更美好（五）](http://www.freebuf.com/articles/system/93440.html) 
- [ ] [Using IDAPython to Make Your Life Easier: Part 6](https://researchcenter.paloaltonetworks.com/2016/06/unit42-using-idapython-to-make-your-life-easier-part-6/)

##Old vs New

IDAPython 7.0 开始由 x86 迁移到 x86_64， API 做了很大改动。使用了 IDADIR\python\idc_bc695.py 文件来保证向下兼容。

我们可以使用模组 inspect 中的 getsource 获得新的 API 函数

```python
Python>import inspect
Python>inspect.getsource(MakeName)
def MakeName(ea, name): return set_name(ea, name, SN_CHECK)
```

在 IDADIR\cfg\python.cfg 中确认 `AUTOIMPORT_COMPAT_IDA695 = YES`来开启向下兼容（默认开启）。

兼容层仅仅应用在 idc.py 中的 API。

##快速开始

现在假设我们在`ROM:FF123456`

```python
Python>ea = idc.get_screen_ea()     # electronic arts?
Python>print ea          
4279383126                          # nope...
Python>print "%x" % ea
ff123456                            # bingo!

Python>ea = here()
Python>print "0x% x % s" % (ea, ea)
0x12529 75049
Python>hex(get_inf_attr(INF_MIN_EA))
0x401000
Python>hex(get_inf_attr(INF_MAX_EA))
0x437000
```

ea 是一个32位地址（无符整形）。它是在 IDA 中光标所在的地址。

##编码 ASM 指令

让我们对当前指令进行编码。我们使用 'idc' 模组。IDA 控制台默认会导入它。

```python
# ASM 字符串
Python>print idc.generate_disasm_line(ea, 0)
LDRCC   R3, [R0]
# 指令助记符，不包含后缀和其他东西
Python>idc.print_insn_mnem(ea)
LDR
# 第0个操作数作为字符串
Python>idc.print_operand(ea,0)
R3
Python>idc.print_operand(ea,1)
[R0]
# 第0个操作类型
Python>print idc.get_operand_type(ea,0)
1
Python>print idc.get_operand_type(ea,1)
3
# 第0个操作码的的值
Python>print idc.get_operand_value(ea,0)
3
Python>print idc.get_operand_value(ea,1)
0
```

操作类型

```python
o_void  =      0  # No Operand 
0xa09166 retn

o_reg  =       1  # General Register (al,ax,es,ds...)    reg
0xa09163 pop edi

o_mem  =       2  # Direct Memory Reference  (DATA)      addr
0xa05d86 cmp ds:dword_A152B8, 0
  
o_phrase  =    3  # Memory Ref [Base Reg + Index Reg]    phrase
0x1000b8c2 mov [edi+ecx], eax

o_displ  =     4  # Memory Reg [Base Reg + Index Reg + Displacement] phrase+addr
0xa05dc1 mov eax, [edi+18h]

o_imm  =       5  # Immediate Value                      value
0xa05da1 add esp, 0Ch

o_far  =       6  # Immediate Far Address  (CODE)        addr
o_near  =      7  # Immediate Near Address (CODE)        addr
o_idpspec0 =   8  # IDP specific type
o_idpspec1 =   9  # IDP specific type
o_idpspec2 =  10  # IDP specific type
o_idpspec3 =  11  # IDP specific type
o_idpspec4 =  12  # IDP specific type
o_idpspec5 =  13  # IDP specific type
```

操作码的的值

```python
operand is an immediate value  => immediate value
operand has a displacement     => displacement
operand is a direct memory ref => memory address
operand is a register          => register number
operand is a register phrase   => phrase number
otherwise                      => -1
```

有时当逆向一个可执行文件的导出内存时，操作数可能没有被识别为偏移。

```python
seg000:00BC1388 push 0Ch
seg000:00BC138A push 0BC10B8h
seg000:00BC138F push [esp+10h+arg_0]
seg000:00BC1393 call ds:_strnicmp
```

第二个被 push 的值是一个内存偏移量。我们可以右击它并改变它为数据类型。也可以自动处理

### 例

```python
import idautils
min = idc.get_inf_attr(INF_MIN_EA)
max = idc.get_inf_attr(INF_MAX_EA)
# for each known function
for func in idautils.Functions():
    flags = idc.get_func_attr(func, FUNCATTR_FLAGS)
    # skip library & thunk functions
    if flags & FUNC_LIB or flags & FUNC_THUNK:
        continue
    dism_addr = list(idautils.FuncItems(func))
    for curr_addr in dism_addr:
        if idc.get_operand_type(curr_addr, 0) == 5 and \
                (min < idc.get_operand_value(curr_addr, 0) < max):
            # 转换操作码到一个偏移量。参数1：地址，2：操作码索引，3：基地址
            idc.op_plain_offset(curr_addr, 0, 0)
        if idc.get_operand_type(curr_addr, 1) == 5 and \
                (min < idc.get_operand_value(curr_addr, 1) < max):
            idc.op_plain_offset(curr_addr, 1, 0)
```

运行上面打码后，我们将看到字符串：

```python
seg000:00BC1388 push 0Ch
seg000:00BC138A push offset aNtoskrnl_exe ; "ntoskrnl.exe"
seg000:00BC138F push [esp+10h+arg_0]
seg000:00BC1393 call ds:_strnicmp
```



- str = GetString(GetOperandValue(e,1), -1, ASCSTR_C) => this is for strings... like ADR R0, aBlahBlah ; "Blah Blah"

## Segments

idautils.Segments() 返回一个迭代类型对象。迭代所有 segments

```python
Python>for seg in idautils.Segments():
	print idc.get_segm_name(seg), idc.get_segm_start(seg), idc.get_segm_end(seg)
HEADER 65536 66208
.idata 66208 66636
.text 66636 212000
.data 212000 217088
.edata 217088 217184
INIT 217184 219872
.reloc 219872 225696
GAP 225696 229376

# 获得 segment 的名字 
Python>idc.get_segm_name(ea)
__text

# 获得 segment 的起始和结尾地址
Python>idc.get_segm_start(ea)
4294978352
Python>idc.get_segm_end(ea)
4302289835

# 获得下一个 segment
Python>idc.get_next_seg(ea)
4302289836
# 通过名字获得 segment 的 selector or BADADDR
Python>idc.selector_by_name("__text")
3
```

##转换操作

使用快捷键 'S' 转换 `[SP,#48,arg76] -> [SP,#0x04]`。IDAPython 或 IDC 使用下面的操作：

```python
Convert operand to a segment expression
OpSeg(ea, i)
     ea  - linear address
      n  - number of operand
             0 - the first operand
             1 - the second, third and all other operands
            -1 - all operands
Note: the data items use only the type of the first operand
Returns: 1-ok, 0-failure
```



相关的: OpAlt, OpBinary, OpChr, OpDecimal, OpEnumEx, OpFloat, OpHex, OpHigh, OpNot, OpNumber, OpOctal, OpOffEx, OpOff, OpSeg, OpSign, OpStkvar, OpStroffEx [[1]](http://www.hex-rays.com/idapro/idadoc/162.shtml)

##函数

首先在脚本头导入 idautils 和 idc

```python
from idautils import *
from idc import *
```

获得当前光标位置的函数

```python
Python>ea = get_screen_ea()
Python>func = idaapi.get_func(ea)
Python>type(func)
<class 'idaapi.func_t'>
Python>funcname = get_func_name(func.startEA)

# 获得函数的边界
Python>print "Start: 0x% x, End: 0x% x" % (func.startEA, func.endEA)

# 
Python>funcname = get_func_name(ea)
```

列出当前段（segment）中的所有函数

```python
for funcea in Functions(SegStart(ea), SegEnd(ea)):
    name = get_func_name(funcea)
    print name
```

当前位置的下一个、前一个函数

```python
Python>hex(idc.get_next_func(ea))
0x100004250L
Python>hex(idc.get_prev_func(ea))
0x100003f40L
```

设置当前函数的签名：

```python
SetType(ea, "int foo(int a, int b, int c)")
```

探索类

```python
import struct
Python>dir(GetFunctionAttr)
['__call__', '__class__', ... 'func_globals', 'func_name']
```

获得函数的属性

```python
start = idc.get_func_attr(ea, FUNCATTR_START)		
end = idc.get_func_attr(ea, FUNCATTR_END)
cur_addr = start
while cur_addr <= end:
    print hex(cur_addr), idc.generate_disasm_line(cur_addr)
    # 获得程序中下一个已定义的物体(指令或数据)
    cur_addr = idc.next_head(cur_addr, end)
Python>
0x100004140L push    rbp
...
```

另一个常用的函数是`idc.get_func_attr(ea, FUNCATTR_FLAGS).`。它用来检索函数的信息，例如是否为库代码或是否有返回值。有9种标志：

```python
Python>import idautils
Python>for func in idautils.Functions():
flags = idc.get_func_attr(func,FUNCATTR_FLAGS)
if flags & FUNC_NORET:
	print hex(func), "FUNC_NORET"
if flags & FUNC_FAR:
	print hex(func), "FUNC_FAR"
if flags & FUNC_LIB:
	print hex(func), "FUNC_LIB"
if flags & FUNC_STATIC:
	print hex(func), "FUNC_STATIC"
if flags & FUNC_FRAME:
	print hex(func), "FUNC_FRAME"
if flags & FUNC_USERFAR:
	print hex(func), "FUNC_USERFAR"
if flags & FUNC_HIDDEN:
	print hex(func), "FUNC_HIDDEN"
if flags & FUNC_THUNK:
	print hex(func), "FUNC_THUNK"
if flags & FUNC_LIB:
	print hex(func), "FUNC_BOTTOMBP"
```

定义：

```c
#define FUNC_NORET         0x00000001L     // function doesn't return

// This flag is rarely seen unless reversing software that uses segmented memory. 
// It is internally represented as an integer of 2.
#define FUNC_FAR           0x00000002L     // far function
#define FUNC_LIB           0x00000004L     // library 

// 标识函数被编译为静态函数。在 C 函数中默认是全局的。以有限的方式，这可以用于帮助理解源代码的结构。
#define FUNC_STATIC        0x00000008L     // static function

// 标识函数使用了帧指针 ebp ，ebp 使用帧指针的函数通常以标准函数序言开头，用于设置堆栈帧。
#define FUNC_FRAME         0x00000010L     // function uses frame pointer (BP)

// This flag is rarely seen and has little documentation. 
// HexRays describes the flag as "user has specified far-ness of the function".
#define FUNC_USERFAR       0x00000020L     // user has specified far-ness
                                           // of the function

// 标识函数是影藏的，需要需要展开查看。
#define FUNC_HIDDEN        0x00000040L     // a hidden function

// 编译器的"传名调用"实现，往往是将参数放到一个临时函数之中，再将这个临时函数传入函数体。
// 这个临时函数就叫做 Thunk 函数。
#define FUNC_THUNK         0x00000080L     // thunk (jump) function
#define FUNC_BOTTOMBP      0x00000100L     // BP points to the bottom of the stack frame
#define FUNC_NORET_PENDING 0x00000200L     // Function 'non-return' analysis
                                           // must be performed. This flag is
                                           // verified upon func_does_return()
#define FUNC_SP_READY      0x00000400L     // SP-analysis has been performed
                                           // If this flag is on, the stack
                                           // change points should not be not
                                           // modified anymore. Currently this
                                           // analysis is performed only for PC
#define FUNC_PURGED_OK     0x00004000L     // 'argsize' field has been validated.
                                           // If this bit is clear and 'argsize'
                                           // is 0, then we do not known the real
                                           // number of bytes removed from
                                           // the stack. This bit is handled
                                           // by the processor module.
#define FUNC_TAIL          0x00008000L     // This is a function tail.
                                           // Other bits must be clear
                                           // (except FUNC_HIDDEN)
```

通过函数的指令进行迭代

```python
E = list(FuncItems(ea))
for e in E:
	print "%X"%e, generate_disasm_line(e)
Python>
0x100004140L push    rbp
...
```

## 指令

### FuncItems

我们有了函数的地址后，我们可以使用`idautils.FuncItems(ea)`来获得所有地址的列表

```python
Python>dism_addr = list(idautils.FuncItems(here()))
Python>type(dism_addr)
<type 'list'>
Python>print dism_addr
[4573123, 4573126, 4573127, 4573132]
Python>for line in dism_addr: 
  print hex(line), idc.generate_disasm_line(line, 0)
0x45c7c3 mov eax, [ebp-60h]
0x45c7c6 push eax ; void *
0x45c7c7 call w_delete
0x45c7cc retn
```

有时，当逆向包代码时，知道哪里执行了动态调用是很有用的。动态调用应该是一个调用或跳转到一个操作数，该操作数是一个寄存器，例如 `call eax` 或 `jmp edi`。

#### 例

```python
Python>
for func in idautils.Functions():
    flags = idc.get_func_attr(func, FUNCATTR_FLAGS)
    # 如果是库函数或 thunk 函数就略过
    if flags & FUNC_LIB or flags & FUNC_THUNK:
        continue
    dism_addr = list(idautils.FuncItems(func))
    for line in dism_addr:
      	# 获得指令助记符
        m = idc.print_insn_mnem(line)
        if m == 'call' or m == 'jmp':
          	# 第0个操作码的类型
            op = idc.get_operand_type(line, 0)
            # 是否为寄存器类型
            if op == o_reg:
                print "0x%x %s" % (line, idc.generate_disasm_line(line, 0))
Python>
0x43ebde call eax ; VirtualProtect
```

### p/next_head/addr

如果只有一个地址，你想获得下一个指令，可以使用`idc.next_head(ea)`，获得前一个指令使用`idc.prev_head(ea)`。这些函数获得了下一个指令的起始位置而不是下一个地址。获得下一个地址可以使用`idc.next_addr(ea)`和`idc.prev_head(ea)`。

```python
Python>ea = here()
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x10004f24 call sub_10004F32
Python>next_instr = idc.next_head(ea)
Python>print hex(next_instr), idc.generate_disasm_line(next_instr, 0)
0x10004f29 mov [esi], eax
Python>prev_instr = idc.prev_head(ea)
Python>print hex(prev_instr), idc.generate_disasm_line(prev_instr, 0)
0x10004f1e mov [esi+98h], eax
Python>print hex(idc.next_addr(ea))
0x10004f25
Python>print hex(idc.prev_addr(ea))
0x10004f23
```

### decode_insn

我们可以使用`idaapi.decode_insn(ea)`来解码指令。解码指令比字符串比较运行速度更快也更不容易出错。不幸的是，用整数表示是 IDA 专用的，不方便移植到其他汇编工具上。

#### 例

```python
import idautils
Python>JMPS = [idaapi.NN_jmp, idaapi.NN_jmpfi, idaapi.NN_jmpni]
Python>CALLS = [idaapi.NN_call, idaapi.NN_callfi, idaapi.NN_callni]
Python>for func in idautils.Functions():
    flags = idc.get_func_attr(func, FUNCATTR_FLAGS)
    if flags & FUNC_LIB or flags & FUNC_THUNK:
        continue
    dism_addr = list(idautils.FuncItems(func))
    for line in dism_addr:
        idaapi.decode_insn(line)
        if idaapi.cmd.itype in CALLS or idaapi.cmd.itype in JMPS:
            if idaapi.cmd.Op1.type == o_reg:
                print "0x%x %s" % (line, idc.generate_disasm_line(line, 0))
```

### cmd

一旦执行了解码，我们就可以通过`idaapi.cmd`来访问指令的不同属性。

```python
Python>dir(idaapi.cmd)
['Op1',... 'itype', 'segpref', 'size']
```

## Xrefs

总结：

- XrefsTo：ea 为函数首地址，返回谁调用了这个函数的引用
- XrefsFrom: ea 为跳转指令的地址，返回跳转处的引用
- CodeRefsTo/From: 返回的是地址

定位调用 WriteFile 函数的所有地址

```python
# 通过名称获得 WriteFile 函数的地址
Python>wf_addr = idc.get_name_ea_simple("WriteFile")
Python>print hex(wf_addr), idc.generate_disasm_line(wf_addr, 0)
0x1000e1b8 extrn WriteFile:dword
# 参数1：地址，2：bool型，表示是否 follow normal code flow
Python>for addr in idautils.CodeRefsTo(wf_addr, 0):
		print hex(addr), idc.generate_disasm_line(addr, 0)
0x10004932 call ds:WriteFile
0x10005c38 call ds:WriteFile
0x10007458 call ds:WriteFile
```

可以通过调用`idautils.Names()`来访问 IDB 中所有重命名的函数和API。

```python
Python>[x for x in Names()]
[(268439552, 'SetEventCreateThread'), (268439615, 'StartAddress'), (268441102,
'SetSleepClose'),....]
```

如果我们希望获得代码是来自谁的引用，我们可以使用`idautisl.CodeRefsFrom(ea,flow)`。

```python
Python>ea = 0x10004932
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x10004932 call ds:WriteFile
Python>for addr in idautils.CodeRefsFrom(ea, 0):
		print hex(addr), idc.generate_disasm_line(addr, 0)
Python>
0x1000e1b8 extrn WriteFile:dword
```

使用`idautils.CodeRefsTo(ea, flow)`的一个局限，是动态导入然后手动重命名的 API 不显示为代码交叉引用。假设我们使用`idc.set_name(ea, name, SN_CHECK)`手动重命名一个双字节地址为`RtlCompareMemory`：

```python
Python>hex(ea)
0xa26c78
Python>idc.set_name(ea, "RtlCompareMemory", SN_CHECK)
True
Python>for addr in idautils.CodeRefsTo(ea, 0):
		print hex(addr), idc.generate_disasm_line(addr, 0)
```

标志位定义：

```c
#define SN_CHECK        0x01    // Fail if the name contains invalid characters
                                // If this bit is clear, all invalid chars
                                // (those !is_ident_char()) will be replaced
                                // by SUBSTCHAR
                                // List of valid characters is defined in ida.cfg
#define SN_NOCHECK      0x00    // Replace invalid chars with SUBSTCHAR
#define SN_PUBLIC       0x02    // if set, make name public
#define SN_NON_PUBLIC   0x04    // if set, make name non-public
#define SN_WEAK         0x08    // if set, make name weak
#define SN_NON_WEAK     0x10    // if set, make name non-weak
#define SN_AUTO         0x20    // if set, make name autogenerated
#define SN_NON_AUTO     0x40    // if set, make name non-autogenerated
#define SN_NOLIST       0x80    // if set, exclude name from the list
                                // if not set, then include the name into
                                // the list (however, if other bits are set,
                                // the name might be immediately excluded
                                // from the list)
#define SN_NOWARN       0x100   // don't display a warning if failed
#define SN_LOCAL        0x200   // create local name. a function should exist.
                                // local names can't be public or weak.
                                // also they are not included into the list of names
                                // they can't have dummy prefixes
```

IDA 不会标记这些 API 为交叉引用。稍后我们会描述一个活的所有交叉引用的通用技术。如果我们希望搜索交叉引用 to/from 的数据，我们可以使用`idautils.DataRefsTo(e)` or `idautils.DataRefsFrom(ea)`。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
		0x1000e3ec db 'vnc32',0
Python>for addr in idautils.DataRefsTo(ea): 
  	print hex(addr), idc.generate_disasm_line(addr, 0)
0x100038ac push offset aVnc32 ; "vnc32"
```

`idautils.DataRefsTo(ea)`输入地址，返回所有交叉引用到这个数据的迭代器。.

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x100038ac push offset aVnc32 ; "vnc32"
Python>for addr in idautils.DataRefsFrom(ea): 
		print hex(addr), idc.generate_disasm_line(addr, 0)
0x1000e3ec db 'vnc32',0
```

我们可以使用`idautils.XrefsTo(ea, flags=0)`来获取 to 一个地址的所有交叉引用，使用`idautils.XrefsFrom(ea, flags=0)`来获取 from 一个地址的所有交叉引用

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x1000eee0 unicode 0, <Path>,0
Python>for xref in idautils.XrefsTo(ea, 1):
    print xref.type, idautils.XrefTypeName(xref.type), \
        hex(xref.frm), hex(xref.to), xref.iscode
Python>
1 Data_Offset 0x1000ac0d 0x1000eee0 0
Python>print hex(xref.frm), idc.generate_disasm_line(xref.frm, 0)
0x1000ac0d push offset KeyName ; "Path"
```

`idautils.XrefTypeName(xref.type)`用来打印这个类型代表的字符串。有12种不同的类型：

```python
0 = 'Data_Unknown'
1 = 'Data_Offset'
2 = 'Data_Write'
3 = 'Data_Read'
4 = 'Data_Text'
5 = 'Data_Informational'
16 = 'Code_Far_Call'
17 = 'Code_Near_Call'
18 = 'Code_Far_Jump'
19 = 'Code_Near_Jump'
20 = 'Code_User'
21 = 'Ordinary_Flow'
```

`xref.frm`：from 地址

`xref.to`：to 地址

`xref.iscode`：判断 xref 是否在打码段中

`idautils.XrefsTo(ea, 1)`：这里 flag 位被置1。如果为0则会显示任何的交叉引用，下面的汇编代码片段说明了这个问题。

```assembly
.text:1000AAF6 jnb short loc_1000AB02 ; XREF
.text:1000AAF8 mov eax, [ebx+0Ch]
.text:1000AAFB mov ecx, [esi]
.text:1000AAFD sub eax, edi
.text:1000AAFF mov [edi+ecx], eax
.text:1000AB02
.text:1000AB02 loc_1000AB02: ; ea is here()
.text:1000AB02 mov byte ptr [ebx], 1
```

光标在 1000AB02 ，这个地址有位于 1000AAF6 的交叉引用，但也包含第二个交叉引用。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x1000ab02 mov byte ptr [ebx], 1
Python>for xref in idautils.XrefsTo(ea, 1):
    print xref.type, idautils.XrefTypeName(xref.type), \
        hex(xref.frm), hex(xref.to), xref.iscode
Python>
19 Code_Near_Jump 0x1000aaf6 0x1000ab02 1
Python>for xref in idautils.XrefsTo(ea, 0):
    print xref.type, idautils.XrefTypeName(xref.type), \
        hex(xref.frm), hex(xref.to), xref.iscode
Python>
21 Ordinary_Flow 0x1000aaff 0x1000ab02 1
19 Code_Near_Jump 0x1000aaf6 0x1000ab02 1
```

第二个交叉引用从 1000AAFF 到 1000AB02。交叉引用不一定是分支指令。它们也可能是普通的代码流。如果我们设置标志位为1， Ordinary_Flow 引用的类型不会被增加。

现在返回到我们先前的 RtlCompareMemory 例子，我们可以使用`idautils.XrefsTo(ea, flow)`来获得所有的引用：

```python
Python>hex(ea)
0xa26c78
Python>idc.set_name(ea, "RtlCompareMemory", SN_CHECK)
True
Python>for xref in idautils.XrefsTo(ea, 1):
    print xref.type, idautils.XrefTypeName(xref.type), \
        hex(xref.frm), hex(xref.to), xref.iscode
Python>
3 Data_Read 0xa142a3 0xa26c78 0
3 Data_Read 0xa143e8 0xa26c78 0
3 Data_Read 0xa162da 0xa26c78 0
```

获取所有交叉引用会更详细。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0xa21138 extrn GetProcessHeap:dword
Python>for xref in idautils.XrefsTo(ea, 1):
    print xref.type, idautils.XrefTypeName(xref.type), \
        hex(xref.frm), hex(xref.to), xref.iscode
Python>
17 Code_Near_Call 0xa143b0 0xa21138 1
17 Code_Near_Call 0xa1bb1b 0xa21138 1
3 Data_Read 0xa143b0 0xa21138 0
3 Data_Read 0xa1bb1b 0xa21138 0
Python>print idc.generate_disasm_line(0xa143b0, 0)
call ds:GetProcessHeap
```

来自 Data_Read 和 Code_Near 的内容增加到了 xrefs。获取所有地址并将其添加到集合可能有助于对所有地址瘦身。

```python
def get_to_xrefs(ea):
    xref_set = set([])
    for xref in idautils.XrefsTo(ea, 1):
    xref_set.add(xref.frm)
    return xref_set

    
def get_frm_xrefs(ea):
    xref_set = set([])
    for xref in idautils.XrefsFrom(ea, 1):
    xref_set.add(xref.to)
    return xref_set
```

下面是在输出 GetProcessHeap 上瘦身的函数示例。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0xa21138 extrn GetProcessHeap:dword
Python>get_to_xrefs(ea)
set([10568624, 10599195])
Python>[hex(x) for x in get_to_xrefs(ea)]
['0xa143b0', '0xa1bb1b']
```

## 搜索

### find_binary

有时，我们需要搜索指定的字节，例如：0x55 0x8B 0xEC 。这种字节形式是传统函数的序言 `push ebp, mov ebp, sap`。我们可以使用`idc.find_binary(ea, flag, searchstr, radix=16)`来进行字节或二进制方式的搜索。

- ea：搜索范围的起始地址

- flag: 确定方向或条件

  ```cassandra
  SEARCH_UP       0x00    // search backward
  SEARCH_DOWN     0x01    // search forward
  SEARCH_NEXT     0x02    // start the search at the next/prev item
                          // useful only for find_text() and find_binary()
                          // for other Find.. functions it is implicitly set
  SEARCH_CASE     0x04    // search case-sensitive
                          // (only for bin&txt search)
  SEARCH_REGEX    0x08    // enable regular expressions (only for txt)
  SEARCH_NOBRK    0x10    // don't test ctrl-break
  SEARCH_NOSHOW   0x20    // don't display the search progress
  SEARCH_UNICODE	0x40		// ** 所有的搜索字符串使用 Unicode 编码方式
  SEARCH_IDENT 		0x80		// ** 
  SEARCH_BRK		  0x100		// **
  ** 老版本 IDAPython 不支持
  ```

- searchstr：搜索的样式。例如：“41 42”——查找2个字节 41h，42h

- radix：在写处理模块时使用。默认依赖当前 IDP 的模式（radix for ibm pc is 16）

让我们快速浏览一下前面提到的查找函数序言字节模式

```python
# mac 64位程序为 “push rbp” = “55”，需要设置为“55 48 89”？
Python>pattern = '55 8B EC'	
addr = idc.get_inf_attr(INF_MIN_EA)
for x in range(0, 5):
    addr = idc.find_binary(addr, SEARCH_DOWN, pattern)
    if addr != idc.BADADDR:
        print hex(addr), idc.generate_disasm_line(addr, 0)
Python>
0x401000 push ebp
0x401000 push ebp
0x401000 push ebp
0x401000 push ebp
0x401000 push ebp
```

为什么地址没有增加？这是因为我们没有设置 SEARCH_NEXT 标志位。如果没有设置这个标志位，他会一直搜索到第一个匹配的地址。下面是正确的版本：

```python
Python>pattern = '55 8B EC'
addr = idc.get_inf_attr(INF_MIN_EA)
for x in range(0,5):
		addr = idc.find_binary(addr, SEARCH_DOWN|SEARCH_NEXT, pattern);
		if addr != idc.BADADDR:
				print hex(addr), idc.generate_disasm_line(addr, 0)
Python>
0x401040 push ebp
0x401070 push ebp
0x4010e0 push ebp
0x401150 push ebp
0x4011b0 push ebp
```

### find_text

有时候我们希望搜索“chrome.dll”，我们可以使用`[hex(y) for y in bytearray("chrome.dll")]`，但这有不够优雅。而且，日过字符串是 unicode 格式，我们还需要说明格式。最简单的方法时使用`idc.find_text(ea, flag, y, x, searchstr)`，它的大部分参数与`find_binary`类似

- ea：搜索范围的起始地址
- flag: 确定方向或条件
- y：从 ea 开始增加 y 行开始搜索
- x：在行里开始的横坐标
- searchstr：搜索的样式。例如：“41 42”——查找2个字节 41h，42h

现在我们来搜索“Accept”字符串。我们可以使用`shift+F12`来打开 String window ，里面的任何字符串都可以用在本例中。

```python
Python>cur_addr = idc.get_inf_attr(INF_MIN_EA)
end = idc.get_inf_attr(INF_MAX_EA)
while cur_addr < end:
    cur_addr = idc.find_text(cur_addr, SEARCH_DOWN, 0, 0, "Accept")
    if cur_addr == idc.BADADDR:
        break
    else:
        print hex(cur_addr), idc.generate_disasm_line(cur_addr, 0)
    cur_addr = idc.next_head(cur_addr)
Python>
0x40da72 push offset aAcceptEncoding; "Accept-Encoding:\n"
0x40face push offset aHttp1_1Accept; " HTTP/1.1\r\nAccept: */* \r\n "
0x40fadf push offset aAcceptLanguage; "Accept-Language: ru \r\n"
...
0x423c00 db 'Accept',0
0x423c14 db 'Accept-Language',0
0x423c24 db 'Accept-Encoding',0
0x423ca4 db 'Accept-Ranges',0
```

### is_*

除了先前描述的模式搜索之外，还有一些可用于查找其他类型的函数。find API 的命名约定可以很容易地推断出它的整体功能。在我们讨论找到不同类型之前，我们首先通过它们的地址来识别类型。有一个 API 的子集可以用来确定地址的类型。API返回布尔值True或False。

- idc.is_code(f) 如果 IDA 已将地址标记为代码，则返回 True。
- idc.is_data(f) 数据
- idc.is_tail(f) tail
- idc.is_unknown(f) 未知，IDA 不能识别出是代码还是数据
- idc.is_head(f) 地址标记为头部

参数 f 为内部标志，我们使用函数`idc.get_full_flags(ea)`通过传递一个地址获得。现在让我们来看一个例子：

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x10001000 push ebp
Python>idc.is_code(idc.get_full_flags(ea))
True
```

### idc.find_code(ea, flag)

这个函数用来查找被标记为代码的下一个地址。如果我们想找到数据块的结尾，它会很有用。如果 ea 已经被标记为代码，它会返回下一个地址。 flag 同`idc.find_text`中的描述。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x4140e8 dd offset dword_4140EC
Python>addr = idc.find_code(ea, SEARCH_DOWN|SEARCH_NEXT)
Python>print hex(addr), idc.generate_disasm_line(addr, 0)
0x41410c push ebx
```

正如我们所看到的，ea 是一些数据的地址。我们寻找下一个代码的地址并打印出来。通过这一个函数，我们略过了36字节的数据来获得标记为代码的区的开始位置。

### idc.find_data(ea, flag)

它返回标记为数据块的下一个地址的开始。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x41410c push ebx
Python>addr = idc.find_data(ea, SEARCH_UP|SEARCH_NEXT)
Python>print hex(addr), idc.generate_disasm_line(addr, 0)
0x4140ec dd 49540E0Eh, 746E6564h, 4570614Dh, 7972746Eh, 8, 1, 4010BCh
```

与搜索代码不同的是搜索数据的方向

### idc.find_unknown(ea, flag)

这个用于查找 IDA 不能识别为代码或数据的地址。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x406a05 jge short loc_406A3A
Python>addr = idc.find_unknown(ea, SEARCH_DOWN)
Python>print hex(addr), idc.generate_disasm_line(addr, 0)
0x41b004 db 0DFh ; ?
```

### idc.find_defined(ea, flag)

它用来查找 IDA 已经标识为代码或数据的地址

```python
0x41b900 db ? ;
Python>addr = idc.find_defined(ea, SEARCH_UP)
Python>print hex(addr), idc.generate_disasm_line(addr, 0)
0x41b5f4 dd ?
```

这看起来不像任何真实的值，但是如果我们打印 addr 的交叉引用，我们可以看到他被使用了。

```python
Python>for xref in idautils.XrefsTo(addr, 1):
		print hex(xref.frm), idc.generate_disasm_line(xref.frm, 0)
Python>
0x4069c3 mov eax, dword_41B5F4[ecx*4]
```

### idc.find_imm(ea, flag, value)

我们有时也需要查找一个特定的值。例如，我们感觉代码通过调用`rand`来产生随机数，但我们不能找到那个代码。如果我们知道 rand 使用 0x343FD 作为种子，我们就可以用这个数字来搜索。

```python
Python>addr = idc.find_imm(get_inf_attr(INF_MIN_EA), SEARCH_DOWN, 0x343FD )
Python>addr
[268453092, 0]
Python>print "0x%x %s %x" % (addr[0], idc.generate_disasm_line(addr[0], 0), addr[1])
0x100044e4 imul eax, 343FDh 0
```

它返回一个元组数据。第一个是地址，第二个是操作数。如果我们想搜索立即值的所有用途，我们可以这样：

```python
Python>addr = idc.get_inf_attr(INF_MIN_EA)
while True:
		addr, operand = idc.find_imm(addr, SEARCH_DOWN | SEARCH_NEXT, 0x7a)
		if addr != BADADDR:
			print hex(addr), idc.generate_disasm_line(addr, 0), "Operand ", operand
else:
break
Python>
0x402434 dd 9, 0FF0Bh, 0Ch, 0FF0Dh, 0Dh, 0FF13h, 13h, 0FF1Bh, 1Bh Operand 0
0x40acee cmp eax, 7Ah Operand 1
0x40b943 push 7Ah Operand 0
0x424a91 cmp eax, 7Ah Operand 1
0x424b3d cmp eax, 7Ah Operand 1
0x425507 cmp eax, 7Ah Operand 1
```

大部分代码看起来是相似的，但是因为我们正在搜索多个值，所以使用了循环和`SEARCH_DOWN|SEARCH_NEXT`标志。

## 搜索数据

有时我们已经知道了代码或数据的位置，但我们想选择它来进行分析。在这种情况下我们可能仅仅想聚焦在代码上并在 IDAPython 中使用它开始工作。我们使用`idc.read_selection_start()`和`idc.read_selection_end()`分别获取已选择的数据的起始和结束地址。

假定我们选择了下列代码

```assembly
.text:00408E46 push ebp
.text:00408E47 mov ebp, esp
.text:00408E49 mov al, byte ptr dword_42A508
.text:00408E4E sub esp, 78h
.text:00408E51 test al, 10h
.text:00408E53 jz short loc_408E78
.text:00408E55 lea eax, [ebp+Data]
```

我们可以使用下列代码来输出地址：

```python
Python>start = idc.read_selection_start()
Python>hex(start)
0x408e46
Python>end = idc.read_selection_end()
Python>hex(end)
0x408e58
```

有一点需要注意的是，end 不是已选择地址的最后，而是最后的下个地址的开始。我们也可以只用一个函数`idaapi.read_selection()`来获取。返回结果中的 Worked 是 a bool if the selection was read。

```python
Python>Worked, start, end = idaapi.read_selection()
Python>print Worked, hex(start), hex(end)
True 0x408e46 0x408e58
```

使用64位样本时要小心。基址并不总是正确的，因为所选的起始地址可能导致整数溢出，并且前导数字可能不正确。 这可能已在 IDA 7.+ 中修复。

## 注释和重命名

注释有两种类型：

1. regular comment

   出现在地址 0041136B 作为文本 regular comment

2. repeatable comment

   可重复注释将附加到当前的物体与所有引用它的物体上。

   出现在地址 00411372，00411386 和 00411392。只有最后一个注释是手动输入的。其他的注释在当一个指令引用了包含可重复注释的地址（如条件分支）时出现。

```assembly
00411365 mov [ebp+var_214], eax
0041136B cmp [ebp+var_214], 0 ; regular comment
00411372 jnz short loc_411392 ; repeatable comment
00411374 push offset sub_4110E0
00411379 call sub_40D060
0041137E add esp, 4
00411381 movzx edx, al
00411384 test edx, edx
00411386 jz short loc_411392 ; repeatable comment
00411388 mov dword_436B80, 1
00411392
00411392 loc_411392:
00411392
00411392 mov dword_436B88, 1 ; repeatable comment
0041139C push offset sub_4112C0
```

### 指令的注释

#### 增加注释

为了增加注释，我们使用`idc.set_cmt(ea, comment,0)`增加注释，使用`idc.set_cmt(ea, comment, 1)`增加可重复注释。

- ea 是地址
- comment 是我们将添加的字符串
- 0 标识注释不是可重复的而 1 标识是。下面的代码在每次指令使用XOR将寄存器或值清零时，都会添加注释。

```python
for func in idautils.Functions():
    flags = idc.get_func_attr(func, FUNCATTR_FLAGS)
    # skip library & thunk functions
    if flags & FUNC_LIB or flags & FUNC_THUNK:
        continue
    dism_addr = list(idautils.FuncItems(func))
    for ea in dism_addr:
        if idc.print_insn_mnem(ea) == "xor":
            if idc.print_operand(ea, 0) == idc.print_operand(ea, 1):
                comment = "%s = 0" % (idc.print_operand(ea, 0))
                idc.set_cmt(ea, comment, 0)
```

在最后我们确认操作数是否相等，然后增加非可重复注释。

```assembly
0040B0F7 xor al, al ; al = 0
0040B0F9 jmp short loc_40B163
```

为了增加一个可重复注释，我们用`set_cmt(ea, comment, 1)`替换`set_cmt(ea, comment, 0)`。这可能会更有用，因为我们会看到对分支的引用，它将值清零并可能返回0。

#### 获得注释

我们使用`idc.get_cmt(ea, repeatable)`来获得注释。

- ea 是包含注释的地址，
- repeatable：1为真，0位假。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x40b0f7 xor al, al ; al = 0
Python>idc.get_cmt(ea, False)
al = 0
```

### 函数的注释

不是只有指令可以增加注释，函数也能。我们使用`idc.set_func_cmt(ea, cmt, repeatable)`来增加，使用`idc.get_func_cmt(ea, repeatable)`来获得。

- ea：可以是在函数中的任意地址
- cmt：我们将增加的字符串
- repeatable：1为真，0位假。

将函数注释设置为可重复时，只要在IDA的GUI中交叉引用，调用或查看函数，就会添加注释。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x401040 push ebp
Python>idc.get_func_name(ea)
sub_401040
Python>idc.set_func_cmt(ea, "check out later", 1)
True
```

我们给函数增加了注释“check out later”

```assembly
00401040 ; check out later
00401040 ; Attributes: bp-based frame
00401040
00401040 sub_401040 proc near
00401040 .
00401040 var_4 = dword ptr -4
00401040 arg_0 = dword ptr 8
00401040
00401040 push ebp
```

由于注释是可重复的，因此只要查看该功能，就会显示该注释。这是添加有关功能的提醒或注释的好地方。

```assembly
00401C07 push ecx
00401C08 call sub_401040 ; check out later
00401C0D add esp, 4
```

### 重命名函数和地址

重命名函数和地址是一项常见的自动化任务，尤其是在处理与位置无关的代码（PIC），打包器（packers）或封装函数时。 PIC 或解包代码中常见的原因是因为转储中可能不存在导入表。 在包装函数的情况下，完整函数只是调用API。

```assembly
10005B3E sub_10005B3E proc near
10005B3E
10005B3E dwBytes = dword ptr 8
10005B3E
10005B3E push ebp
10005B3F mov ebp, esp
10005B41 push [ebp+dwBytes] ; dwBytes
10005B44 push 8 ; dwFlags
10005B46 push hHeap ; hHeap
10005B4C call ds:HeapAlloc
10005B52 pop ebp
10005B53 retn
10005B53 sub_10005B3E endp
```

在上面的代码中`w_HeapAlloc`函数被调用。`w_`是封装的简写。我们可以使用函数`idc.set_name(ea, name, SN_CHECK)`来重命名一个地址。

- ea：地址。必须是函数的第一个地址。

- name：字符串名，例如“w_HeapAlloc”

- SN_CHECK：标识位，“SN_…”常量的组合

  ```c
  SN_CHECK        0x01    // Fail if the name contains invalid characters
                          // If this bit is clear, all invalid chars
                          // (those !is_ident_char()) will be replaced
                          // by SUBSTCHAR
                          // List of valid characters is defined in ida.cfg
  SN_NOCHECK      0x00    // Replace invalid chars with SUBSTCHAR
  SN_PUBLIC       0x02    // if set, make name public
  SN_NON_PUBLIC   0x04    // if set, make name non-public
  SN_WEAK         0x08    // if set, make name weak
  SN_NON_WEAK     0x10    // if set, make name non-weak
  SN_AUTO         0x20    // if set, make name autogenerated
  SN_NON_AUTO     0x40    // if set, make name non-autogenerated
  SN_NOLIST       0x80    // if set, exclude name from the list
                          // if not set, then include the name into
                          // the list (however, if other bits are set,
                          // the name might be immediately excluded
                          // from the list)
  SN_NOWARN       0x100   // don't display a warning if failed
  SN_LOCAL        0x200   // create local name. a function should exist.
                          // local names can't be public or weak.
                                  // also they are not included into the list of names
                                  // they can't have dummy prefixes
  ```

我们使用下列代码来重命名 HeapAlloc 封装函数：

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x10005b3e push ebp
Python>idc.set_name(ea, "w_HeapAlloc", SN_CHECK)
True
```

```assembly
10005B3E w_HeapAlloc proc near
10005B3E
10005B3E dwBytes = dword ptr 8
10005B3E
10005B3E push ebp
10005B3F mov ebp, esp
10005B41 push [ebp+dwBytes] ; dwBytes
10005B44 push 8 ; dwFlags
10005B46 push hHeap ; hHeap
10005B4C call ds:HeapAlloc
10005B52 pop ebp
10005B53 retn
10005B53 w_HeapAlloc endp
```

我们可以看到，函数已经被重命名了。

我们可以使用`idc.get_func_name(ea)`打印新的函数名来确认它是否已经被重命名了

```python
Python>idc.get_func_name(ea)
w_HeapAlloc
```

为了重命名操作数，我们首先需要获得他的地址。在地址：004047B0 处，我们有一个需要重命名的 dword 。

```assembly
.text:004047AD lea ecx, [ecx+0]
.text:004047B0 mov eax, dword_41400C
.text:004047B6 mov ecx, [edi+4BCh]
```

我们可以使用`get_operand_value(ea, n)`来获得操作数的值。

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x4047b0 mov eax, dword_41400C
Python>op = idc.get_operand_value(ea, 1)
Python>print hex(op), idc.generate_disasm_line(op, 0)
0x41400c dd 2
Python>idc.set_name(op, "BETA", SN_CHECK)
True
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0x4047b0 mov eax, BETA[esi]
```

现在我们拥有良好的知识基础，我们可以使用迄今为止学到的东西来自动化封装函数的命名。请参阅内联注释以了解逻辑。

```python
import idautils


def rename_wrapper(name, func_addr):
  	# SN_NOWARN：不弹出警告对话框
    if idc.set_name(func_addr, name, SN_NOWARN):
        print "Function at 0x%x renamed %s" % (func_addr, idc.get_func_name(func))
    else:
        print "Rename at 0x%x failed. Function %s is being used." % (func_addr, name)
    return

    
def check_for_wrapper(func):
    flags = idc.get_func_attr(func, FUNCATTR_FLAGS)
    # skip library & thunk functions
    if flags & FUNC_LIB or flags & FUNC_THUNK:
        return
    dism_addr = list(idautils.FuncItems(func))
    # get length of the function
    func_length = len(dism_addr)
    # if over 32 lines of instruction return
    if func_length > 0x20:
        return
    func_call = 0
    instr_cmp = 0
    op = None
    op_addr = None
    op_type = None
    # for each instruction in the function
    for ea in dism_addr:
        m = idc.print_insn_mnem(ea)
        if m == 'call' or m == 'jmp':
            if m == 'jmp':
                temp = idc.get_operand_value(ea, 0)
                # ignore jump conditions within the function boundaries
                if temp in dism_addr:
                    continue
            func_call += 1
            # wrappers should not contain multiple function calls
            if func_call == 2:
                return
            op_addr = idc.get_operand_value(ea, 0)
            op_type = idc.get_operand_type(ea, 0)
        elif m == 'cmp' or m == 'test':
            # wrappers functions should not contain much logic.
            instr_cmp += 1
            if instr_cmp == 3:
                return
        else:
            continue
    # all instructions in the function have been analyzed
    if op_addr == None:
        return
    name = idc.get_name(op_addr, ida_name.GN_VISIBLE)
    # skip mangled function names
    if "[" in name or "$" in name or "?" in name or "@" in name or name == "":
        return
    name = "w_" + name
    if op_type == 7:
        if idc.get_func_attr(op_addr, FUNCATTR_FLAGS) & FUNC_THUNK:
            rename_wrapper(name, func)
            return
    if op_type == 2 or op_type == 6:
        rename_wrapper(name, func)
        return


for func in idautils.Functions():
		check_for_wrapper(func)
```

示例的输出：

```python
Function at 0xa14040 renamed w_HeapFree
Function at 0xa14060 renamed w_HeapAlloc
Function at 0xa14300 renamed w_HeapReAlloc
Rename at 0xa14330 failed. Function w_HeapAlloc is being used.
Rename at 0xa14360 failed. Function w_HeapFree is being used.
Function at 0xa1b040 renamed w_RtlZeroMemory
```

## 访问原始数据

Raw data 是二进制的代码和数据。我们可以在地址的右边看到原始数据。

```assembly
00A14380 8B 0D 0C 6D A2 00 mov ecx, hHeap
00A14386 50 push eax
00A14387 6A 08 push 8
00A14389 51 push ecx
00A1438A FF 15 30 11 A2 00 call ds:HeapAlloc
00A14390 C3 retn
```

为了访问数据，我们首先要做的是决定单位尺寸（unit size）。用来访问数据的 API 的命名习惯是单元尺寸。我们将调用`idc.get_wide_byte(ea)`和`idc.get_wide_word(ea)`来分别访问字节和字，等等

- idc.get_wide_byte(ea)
- idc.get_wide_word(ea)
- idc.get_wide_dword(ea)
- idc.get_qword(ea)
- idc.GetFloat(ea)
- idc.GetDouble(ea)

假设光标位于上面代码的 00A14680 处，我们将会得到以下输出：

```python
Python>print hex(ea), idc.generate_disasm_line(ea, 0)
0xa14380 mov ecx, hHeap
Python>hex( idc.get_wide_byte(ea) )
0x8b
Python>hex( idc.get_wide_word(ea) )
0xd8b
Python>hex( idc.get_wide_dword(ea) )
0x6d0c0d8b
Python>hex( idc.get_qword(ea) )
0x6a5000a26d0c0d8bL
Python>idc.GetFloat(ea) # Example not a float value
2.70901711372e+27
Python>idc.GetDouble(ea)
1.25430839165e+204
```

在编写解码器时，获取单个字节或读取双字并读取原始数据块并不总是有用的。要在地址读取指定大小的字节，我们可以使用`idc.get_bytes(ea，size，use_dbg = False)`。 最后一个参数是可选的，只有在需要调试内存时才需要。

```python
Python>for byte in idc.get_bytes(ea, 6):
print "0x%X" % ord(byte),
0x8B 0xD 0xC 0x6D 0xA2 0x0
```

应该注意，`idc.get_bytes(ea，size)`返回字节的 char 表示。这与返回整数的`idc.get_wide_word(ea)`或`idc.get_qword(ea)`不同。

## 补丁

有时，当逆向恶意软件时，样本包含编码的字符串。这样做是为了减慢分析过程并阻止使用字符串查看器来恢复指示器。在这样的情况下使用 IDB 修补非常有用。我们可以重命名地址，但重命名是被限制的。这是由于命名约定的限制。要使用值修补地址，我们可以使用以下函数。

- idc.patch_byte(ea, value)
- idc.patch_word(ea, value)
- idc.patch_dword(ea, value)

ea 是地址，value 是我们希望打补丁的整形值。值的尺寸需要与我们选择的函数名的尺寸相匹配。下面是我们发现编码字符串的一个例子：

```assembly
.data:1001ED3C aGcquEUdg_bUfuD  db 'gcqu^E]~UDG_B[uFU^DC',0
.data:1001ED51                  align 8
.data:1001ED58 aGcqs_cuufuD     db 'gcqs\_CUuFU^D',0
.data:1001ED66                  align 4
.data:1001ED68 aWud@uubQU       db 'WUD@UUB^Q]U',0
.data:1001ED74                  align 8
```

在我们分析期间，我们能够标识编码函数。

```assembly
100012A0    push    esi
100012A1    mov     esi, [esp+4+_size]
100012A5    xor     eax, eax
100012A7    test    esi, esi
100012A9    jle     short _ret
100012AB    mov     dl, [esp+4+_key]    ; assign key
100012AF    mov     ecx, [esp+4+_string]
100012B3    push    ebx
100012B4
100012B4 _loop:                         ;
100012B4    mov     bl, [eax+ecx]
100012B7    xor     bl, dl              ; data ^ key
100012B9    mov     [eax+ecx], bl       ; save off byte
100012BC    inc     eax                 ; index/count
100012BD    cmp     eax, esi
100012BF    jl      short _loop
100012C1    pop     ebx
100012C2
100012C2 _ret:                          ;
100012C2    pop     esi
100012C3    retn
```

该函数是具有大小，密钥和解码缓冲区的参数的标准的XOR解码器函数。

```python
Python>start = idc.read_selection_start()
Python>end = idc.read_selection_end()
Python>print hex(start)
0x1001ed3c
Python>print hex(end)
0x1001ed50
Python>def xor(size, key, buff):
    for index in range(0, size):
        cur_addr = buff + index
        temp = idc.get_wide_byte(cur_addr ) ^ key
        idc.patch_byte(cur_addr, temp)
Python>
Python>xor(end - start, 0x30, start)
Python>idc.get_strlit_contents(start)
WSAEnumNetworkEvents
```

我们使用`idc.read_selection_start()`和`idc.read_selection_end()`来选择高亮的数据地址的开始和结束。然后我们有一个函数通过调用`idc.get_wide_byte(ea)`来读取字节，XOR 将带有密钥的字节传递给函数，然后通过调用`idc.patch_byte(ea，value)`来修补字节。

## 输入和输出

我们可以使用`ida_kernwin.ask_file(forsave, mask, prompt)`函数通过名称来导入或保存一个文件。

- forsave：0-“打开”对话框；1-“保存”对话框
- mask：文件的扩展名或样式。如果你希望仅仅打开 .dll 文件，你可以使用掩码“*.dll”
- prompt： the prompt to display in the dialog box
- Returns: the selected file.

输入和输出以及选择数据的一个很好的例子是以下 IO_DATA 类。

```python
import sys
import idaapi


class IO_DATA():
    def __init__(self):
        self.start = idc.read_selection_start()
        self.end = idc.read_selection_end()
        self.buffer = ''
        self.ogLen = None
        self.status = True
        self.run()

    def checkBounds(self):
        if self.start is BADADDR or self.end is BADADDR:
            self.status = False

    def getData(self):
        """get data between start and end put them into object.buffer"""
        self.ogLen = self.end - self.start
        self.buffer = ''
        try:
            for byte in idc.get_bytes(self.start, self.ogLen):
                self.buffer = self.buffer + byte 
        except:
            self.status = False
        return

    def run(self):
        """basically main"""
        self.checkBounds()
        if self.status == False:
            sys.stdout.write('ERROR: Please select valid data\n')
            return
        self.getData()


    def patch(self, temp=None):
        """patch idb with data in object.buffer"""
        if temp != None:
            self.buffer = temp
            for index, byte in enumerate(self.buffer):
                idc.patch_byte(self.start + index, ord(byte))

    def importb(self):
        '''import file to save to buffer'''
        fileName = ida_kernwin.ask_file(0, "*.*", 'Import File')
        try:
            self.buffer = open(fileName, 'rb').read()
        except:
            sys.stdout.write('ERROR: Cannot access file')

    def export(self):
        '''save the selected buffer to a file'''
        exportFile = ida_kernwin.ask_file(1, "*.*", 'Export Buffer')
        f = open(exportFile, 'wb')
        f.write(self.buffer)
        f.close()

    def stats(self):
        print "start: %s" % hex(self.start)
        print "end: %s" % hex(self.end)
        print "len: %s" % hex(len(self.buffer))
```

通过此类可以选择将数据保存到缓冲区，然后存储到文件中。这对IDB中的编码或加密数据很有用。我们可以使用 IO_DATA 来选择在Python中解码缓冲区的数据然后修补IDB。下面是如何使用 IO_DATA 类的示例。

```python
Python>f = IO_DATA()
Python>f.stats()
start:	0x401528
end:		0x401549
len:		0x21
```

与其解释代码的每一行，不如让读者逐个查看函数并查看它们是如何工作的。以下要点解释了每个变量以及功能的作用。obj 是我们分配类的任何变量。f 是`f = IO_DATA()`中的obj。

- obj.start

  包含已选择偏移的起始地址

- obj.end
  包含已选择偏移的结束地址

- obj.buffer
  包含二进制数据

- obj.ogLen
  包含缓冲区的大小

- obj.getData()

  复制 obj.start 与 obj.end 间的二进制数据到 obj.buffer，

- obj.run() 

  选择的数据以二进制格式复制到缓冲区

- obj.patch()

  用 obj.buffer 中的数据在 obj.start 给 IDB 打补丁

- obj.patch(d)
  patch the IDB at obj.start with the argument data.

  用引用数据在 obj.start 给 IDB 打补丁

- obj.importb()
  打开一个文件并将数据存入

- obj.buffer. obj.export()

  输出在 obj.buffer 中的数据保存为文件

- obj.stats()
  以16进制格式打印 obj.start，obj.end 和 obj.buffer 的长度。

## Intel Pin Logger

Pin 是 IA-32 和 x86-64 的动态二进制检测框架。将PIN的动态分析结果与IDA的静态分析相结合，使其成为一种强大的组合。梳理 IDA 和 Pin 的障碍是 Pin 的初始设置和运行。以下步骤是安装，执行跟踪可执行文件的 Pintool 并将执行的地址添加到 IDB 的30秒（减去下载）指南。

```Notes about steps
    * Pre-install Visual Studio 2010 (vc10) or 2012 (vc11)
    * If executing malware do steps 1,2,6,7,8,9,10 & 11 in an analysis machine
1. Download PIN
    * https://software.intel.com/en-us/articles/pintool-downloads
    * Compiler Kit is for version of Visual Studio you are using.
2. Unzip pin to the root dir and rename the folder to "pin"
    * example path C:\pin\
    * There is a known but that Pin does not always parse the arguments correctly if there is spacing in the file path
3. Open the following file in Visual Studio
    * C:\pin\source\tools\MyPinTool\MyPinTool.sln
        - This file contains all the needed setting for Visual Studio.
        - Useful to back up and reuse the directory when starting new pintools.
4. Open the below file, then cut and paste the code into MyPinTool.cpp (currently opened in Visual Studio)
    * C:\pin\source\tools\ManualExamples\itrace.cpp
        - This directory along with ../SimpleExamples is very useful for example code.

5. Build Solution (F7)
6. Copy traceme.exe to C:\pin
7. Copy compiled MyPinTool.dll to C:\pin
    * path C:\pin\source\tools\MyPinTool\Debug\MyPinTool.dll
8. Open a command line and set the working dir to C:\pin
9. Execute the following command
    * pin -t traceme.exe -- MyPinTool.dll
        - "-t" = name of file to be analyzed
        - "-- MyPinTool.dll" = specifies that pin is to use the following pintool/dll
10. While pin is executing open traceme.exe in IDA.
11. Once pin has completed (command line will have returned) execute the following in IDAPython
    * The pin output (itrace.out) must be in the working dir of the IDB. \
```

itrace.cpp 是一个 pintool，它打印执行到 itrace.out 的每条指令的EIP。数据看起来像以下输出。

```python
00401500
00401506
00401520
00401526
00401549
0040154F
0040155E
00401564
0040156A
```

pintools执行后，我们可以运行以下IDAPython代码，为所有执行的地址添加注释。 输出文件 itrace.out 需要位于 IDB 的工作目录中。

```python
f = open('itrace.out', 'r')
lines = f.readlines()

for y in lines:
    y = int(y, 16)
    idc.set_color(y, CIC_ITEM, 0xfffff)
    com = idc.get_cmt(y, 0)
    if com == None or 'count' not in com:
        idc.set_cmt(y, "count:1", 0)
    else:
        try:
            count = int(com.split(':')[1], 16)
        except:
            print hex(y)
        tmp = "count:0x%x" % (count + 1)
        idc.set_cmt(y, tmp, 0)
f.close()
```

我们首先打开 itrace.out 并将所有行读入列表。然后我们迭代列表中的每一行。由于输出文件中的地址是十六进制字符串格式，我们需要将其转换为整数。

```assembly
.text:00401500 loc_401500: ; CODE XREF:
sub_4013E0+106?j
.text:00401500      cmp     ebx, 457F4C6Ah      ; count:0x16
.text:00401506      ja      short loc_401520    ; count:0x16
.text:00401508      cmp     ebx, 1857B5C5h      ; count:1
.text:0040150E      jnz     short loc_4014E0    ; count:1
.text:00401510      mov     ebx, 80012FB8h      ; count:1
.text:00401515      jmp     short loc_4014E0    ; count:1
.text:00401515 ; ---------------------------------------------------------
.text:00401517      align 10h
.text:00401520
.text:00401520 loc_401520:                      ; CODE XREF:
sub_4013E0+126?j
.text:00401520      cmp     ebx, 4CC5E06Fh      ; count:0x15
.text:00401526      ja      short loc_401549    ; count:0x15
```

## 批量文件生成

有时，为目录中的所有文件创建 IDB 或 ASM 会很有用。这有助于在分析属于同一恶意软件系列的一组样本时节省时间。批处理文件生成比在大型集合上手动执行要容易得多。要进行批处理分析，我们需要将 -B 参数传递给 textidaw.exe。可以将以下代码复制到包含我们要为其生成文件的所有文件的目录中。

```python
import os
import subprocess
import glob

paths = glob.glob("*")
ida_path = os.path.join(os.environ['PROGRAMFILES'], "IDA", "idat.exe")

for file_path in paths:
    if file_path.endswith(".py"):
        continue
    subprocess.call([ida_path, "-B", file_path])
```

我们使用`glob.glob("*")`来获取目录中所有文件的列表。如果只想选择某个正则表达式模式或文件类型, 则可以修改该参数。如果我们只想获取具有 .exe 扩展名的文件, 我们将使用`glob.glob("*.exe")`。

`os.path.join(os.environ['PROGRAMFILES'], "IDA", "idat.exe")`用于获取 idat.exe 的路径。IDA 的某些版本的文件夹名称中存在版本号。如果是这种情况, 则需要将参数 "IDA" 修改为文件夹名称。此外, 如果我们选择使用 IDA 的非标准安装位置, 则可能需要修改整个命令。现在让我们假设 IDA 的安装路径是`C:\Program Files\IDA`。找到路径后, 我们循环遍历目录中不包含 .py 的扩展然后传递它们到 IDA。对于单个文件, 它看起来像`C:\Program Files\IDA\idat.exe -B bad_file.exe.`。一旦运行，它将为该文件生成 ASM 和 IDB。所有文件都将写入工作目录。下面是一个示例输出。

```shell
C:\injected>dir
0?/**/____ 09:30 AM <DIR> .
0?/**/____ 09:30 AM <DIR> ..
0?/**/____ 10:48 AM 167,936 bad_file.exe
0?/**/____ 09:29 AM 270 batch_analysis.py
0?/**/____ 06:55 PM 104,889 injected.dll
C:\injected>python batch_analysis.py
Thank you for using IDA. Have a nice day!
C:\injected>dir
0?/**/____ 09:30 AM <DIR> .
0?/**/____ 09:30 AM <DIR> ..
0?/**/____ 09:30 AM 506,142 bad_file.asm
0?/**/____ 10:48 AM 167,936 bad_file.exe
0?/**/____ 09:30 AM 1,884,601 bad_file.idb
0?/**/____ 09:29 AM 270 batch_analysis.py
0?/**/____ 09:30 AM 682,602 injected.asm
0?/**/____ 06:55 PM 104,889 injected.dll
```

产生文件 bad_file.asm, bad_file.idb, injected.asm and injected.idb。

## 执行脚本

IDAPython脚本可以从命令行执行。我们可以使用以下代码计算 IDB 中的每条指令，然后将其写入名为 instru_count.txt 的文件中。

```python
import idc
import idaapi
import idautils

idaapi.autoWait()
count = 0
for func in idautils.Functions():
    # Ignore Library Code
    flags = idc.get_func_attr(func, FUNCATTR_FLAGS)
    if flags & FUNC_LIB:
        continue
    for instru in idautils.FuncItems(func):
        count += 1

f = open("instru_count.txt", 'w')
print_me = "Instruction Count is %d" % (count)
f.write(print_me)
f.close()

idc.Exit(0)
```

从命令行的角度来看，两个最重要的函数是 idaapi.autoWait() 和 idc.Exit(0) 。当 IDA 打开文件时，等待分析完成非常重要。这允许 IDA 构建基于 IDA 分析引擎的所有函数，结构或其他值。我们调用 idaapi.autoWait() 来等待分析完成。它将等待/暂停，直到 IDA 完成其分析。分析完成后，它会将控制权返回给脚本。在我们调用任何依赖于分析完成的 IDAPython 函数之前，在脚本开头执行此操作非常重要。一旦我们的脚本执行完毕，我们需要调用 idc.Exit(0) 。这将停止执行我们的脚本，关闭数据库并返回脚本的调用者。如果不这样做，我们的 IDB 将无法正常关闭。

如果我们想要执行 IDAPython 来计算 IDB 的所有行，我们将执行以下命令行。

```python
C:\Cridix\idbs>"C:\Program Files (x86)\IDA 6.3\idat.exe" -A -Scount.py
cur-analysis.idb
```

`-A`用于自主模式，`-s`信号用于 IDA 在 IDB 打开后在 IDB 上运行脚本。在工作目录中，我们将看到一个名为 instru_count.txt 的文件，其中包含所有指令的计数。



---

# IDAPython

## OpS

- [lichao890427/personal_script](https://github.com/lichao890427/personal_script)

- [duo-labs/idapython](https://github.com/duo-labs/idapython)
    - [ ] [使用 IDA Pro 的 REobjc 模块逆向 Objective-C 二进制文件](https://paper.seebug.org/887/)
    
- [ ] [bazad/ida_kernelcache](https://github.com/bazad/ida_kernelcache): An IDA Toolkit for analyzing iOS kernelcaches.


## IDAPython 脚本示例

### 枚举函数 

这段脚本的目的在于遍历数据库中的每一个函数，并打印出与每个函数有关的基本信息，包括函数的起始和结束地址、函数参数的大小、函数的局部变量空间的大小。所有输出全部在消息窗口中显示。

```python
funcs = Functions()		# 1
for f in funcs:		   # 2			
  name = Name(f)
  end = GetFunctionAttr(f, FUNCATTR_END)
  locals = GetFunctionAttr(f, FUNCATTR_FRSIZE)
  frame = GetFrame(f) # retrieve a handle to the function’s stack frame
  if frame is None: continue
  ret = GetMemberOffset(frame, " r") # " r" is the name of the return address
  if ret == -1: continue
  firstArg = ret + 4
  args = GetStrucSize(frame) - firstArg
  Message("Function: %s, starts at %x, ends at %x\n" % (name, f, end))
  Message(" Local variable area is %d bytes\n" % locals)
  Message(" Arguments occupy %d bytes (%d args)\n" % (args, args / 4))
```

### 枚举指令

利用 idautils 模块中的列表生成器以 Python 编写指令计数脚本。

```python
from idaapi import *

func = get_func(here()) # here() is synonymous with ScreenEA()
if not func is None:
    fname = Name(func.startEA)
    count = 0
    for i in FuncItems(func.startEA)􀁙: count = count + 1
    Warning("%s contains %d instructions\n" % (fname,count))
else:
    Warning("No function found at location %x" % here())
```

由于我们无法在生成器上使用 Python 的 len 函数，因此我们仍然需要检索生成器列表，以逐个计算每一条指令。

### 枚举交叉引用

使用 XrefsFrom 生成器（取自 idautils ）从当前指令中检索所有交叉引用。XrefsFrom 将返回对  xrefblk_t 对象（其中包含有关当前交叉引用的详细信息）的引用。

```python
from idaapi import *
func = get_func(here())
if not func is None:
    fname = Name(func.startEA)
    items = FuncItems(func.startEA)
    for i in items:
        for xref in XrefsFrom(i, 0):􀁘
            if xref.type == fl_CN or xref.type == fl_CF:
                Message("%s calls %s from 0x%x\n" % (fname, Name(xref.to), i))
else:
    Warning("No function found at location %x" % here())
```

### 枚举导出的函数

```python
file = AskFile(1, "*.idt", "Select IDT save file")
with open(file, 'w') as fd:
    fd.write("ALIGNMENT 4\n")
    fd.write("0 Name=%s\n" % GetInputFile())
    for i in range(GetEntryPointQty()):
        ord = GetEntryOrdinal(i)
        if ord == 0: 
            continue
        addr = GetEntryPoint(ord)
        if ord == addr: 
            continue #entry point has no ordinal
        fd.write("%d Name=%s" % (ord, Name(addr)))
        purged = GetFunctionAttr(addr, FUNCATTR_ARGSIZE)
        if purged > 0:
            fd.write(" Pascal=%d" % purged)
        fd.write("\n")
```

 IDAPython 没有采用 IDCstst的文件处理函数，而是使用了 Python 内置的文件处理函数。



---

##Github

###ida-swift-demangle

Github: https://github.com/tobefuturer/ida-swift-demangle



------

# Debug

## 参考

- [ ] [[IDA断点和搜索](https://www.cnblogs.com/Fang3s/p/4367588.html)](https://www.cnblogs.com/Fang3s/p/4367588.html)



## Mac OS X 调试

### Remote Mac OS X debugger

参考： [IDA Mac OS X Debugging](https://www.hex-rays.com/products/ida/debugger/mac/index.shtml)

我们首先必须为调试器服务器设置适当的权限：我们需要使它成为 setgid “procmod”。

```shell
$ sudo chgrp procmod /Applications/ida.app/Contents/MacOS/dbgsrv/mac_server64
$ sudo chmod g+s /Applications/ida.app/Contents/MacOS/dbgsrv/mac_server64
$ ls -l /Applications/ida.app/Contents/MacOS/dbgsrv/mac_server64
```

启动调试服务。注意：密码与 -P 参数间不能有空格

```shell
$ /Applications/ida.app/Contents/MacOS/dbgsrv/mac_server64 -Pmy_password
IDA Mac OS X 64-bit remote debug server(MT) v1.22. Hex-Rays (c) 2004-2017
The switch -P is unsecure. Please store the password in the IDA_DBGSRV_PASSWD environment variable
Listening on 0.0.0.0:23946...
```

选择 Debugger - Select debugger ，接着选 “Remote Mac OS X debugger”

![Select_a_debugger.png](https://upload-images.jianshu.io/upload_images/2224431-661432bb9c6a809b.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

配置 Debugger - Process option

![screenshot.png](https://upload-images.jianshu.io/upload_images/2224431-857a37117c0eddd0.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)



---


## iOS 调试

参考：

- [x] [Debugging iOS Applications With IDA](https://www.hex-rays.com/products/ida/support/tutorials/ios_debugger_tutorial.pdf)
- [x] [ida通过usb调试ios下的app](http://3xp10it.cc/二进制/2017/12/25/ida通过usb调试ios下的app/)

### 准备

1. Debugger -> Process options 里面有两个选项 Application，Inputfile

   填写 iOS 上的应用路径。

   ![700.jpeg](https://upload-images.jianshu.io/upload_images/2224431-5afbd46183b19b76.jpeg?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)


2. Debugger>Debugger options>Set specific options 

   设置`Symbol path`路径：`~/Library/Developer/Xcode/iOS DeviceSupport/<iOS version>/Symbols`

3. 设置断点，点击 Debugger>Start process 



