# Utils

The utilitary library for my own common projects. It has some count of already implemented things as:
- Project validator
- Common concepts(from C++20)
- Common classes for working with a space
- Common functions to work with Math
- Common interfaces\patterns
- Enum-generator
- Atomic string for working with compile-time strings
- Delegates
- Run-time asserts

# Getting started

## Project validator
This tools was created for using with CI\CD or using with git-hooks.
To run this tool you need to:
1. Run python script by the next path: ```/scripts/setup_env.py```
2. After this your env will be checked and the next step you can:
   1. You can run file\[s\]-checking. Just run python script: ```/scripts/file_validator.py``` and pass as argument\[s\] the path to your file\[s\]
   2. You can run project-checking. Just run python script without arguments: ```/scripts/project_validator.py```. It'll find your project by the root ```.git``` folder

## Common concepts
This file will be filled up in the future. Just track the latest version of this library.
But now, you can check all common concepts by the next path: ```sources/Utils/Concepts.h```

## Common classes
This file will be filled up in the future. Just track the latest version of this library.
But now, you can check all common classes by the next path: ```sources/Core/```
At this moment the library has next common classes:
- ```Core::Rect<ArithmeticType>```
- ```Core::GlobalPosition<Len, Type, Precision>```
- ```Core::Size<Dimension, ArithmeticType>```

## Common functions to work with Math
This file will be filled up in the future. Just track the latest version of this library.
But now, you can check all common functions to work with Math by the next path: ```sources/Core/Math.h```

## Common interfaces & patterns
This file will be filled up in the future. Just track the latest version of this library.
But now, you can check all common concepts by the next path: ```sources/Core/```
- ```Singleton.h``` - thread safe Singleton
- ```CommonInterfaces.h```

## Enum-generator
This file will be filled up in the future. Just track the latest version of this library.
But now, you can use the code to simplify working with ```enum class```-es. Just include a header file: ```sources/Core/Enum.h```
And just create your own enum type.
```c++
#include "Core/Enum.h"
#include <iostream>

CreateEnum(Color, int, Red, Green, Blue);

int main()
{
    using namespace std;
    Color color = Color::Red;
    cout << color.ToStr() << endl; // => Red
    cout << color.Cast() << endl; // => 0

    color = Color::Blue;
    cout << color.ToStr() << endl; // => Blue
    cout << color.Cast() << endl; // => 2
    cout << color.Cast() << endl; // => 2
    cout << (color == Color::FromStr("Blue")) << endl; // => true
    cout << (Color::Green == Color::FromStr("Green")) << endl; // => true
    cout << (1 == Color::FromStr("Green").Cast()) << endl; // => true

    return 0;
}
```

## Atomic strings
This file will be filled up in the future. Just track the latest version of this library.
But now, you can use this class to get better perfomance experience working with compile-time strings; but, also, you can work and with dynamic strings.
So about a class: you can find this class by the next path: ```sources/Core/String.h```
Main class is ```Core::BaseClass<CharType>```. But to use it easier just use aliases ```Core::StringAtom``` for working with one-byte charset.
I can say a lot about this class but want to highlight main concepts:
1. Try to use this class only with compile time strings. It was created mainly for these purposes. For example to increase equality comparision between two compile-time strings.
2. But if you want to put your own dynamic string you also can do it. It has the same methods like std::string and more.
Let's go to the examples:
```c++
#include "Core/Strings.h"
#include <iostream>

int main()
{
    using namespace std;
    Core::StringAtom name = "Andrew"_atom; // At this moment you put a compile-time(static) string to the internal string-pool. It's absolutely static string.
    cout << name.Data() << endl; // => Andrew
    cout << (name == "Andrew") << endl; // => true; now a compiler compare not two strings, but their addresses.

    // From this code's line you will work with a variable 'name' as with dynamic-string.
    name.PushBack("!"); // But it's not a problem to make from static string -> dynamic. Don't worry about it, just use methods as you want.
    cout << (name == "Andrew!") << endl; // => true; now a compiler compare JUST two strings. You'll get small deoptimization, but it's okay. As with std::string.
    cout << name.Data() << endl; // => Andrew!

    name.Trim("!");
    cout << name.Data() << endl; // => Andrew

    if (stdname.Match("\\w+\\!"))
        cout << "Matched" << endl;
    else
        cout << "NOT matched" << endl;
    // => Matched

    return 0;
}
```

Just want to highlight again: it has absolutely full functionality of the std::string, and also has new features.
To check it all you can look at the header file 'String.h', or make the same with examples in the 'test' directory: ```tests/StringTest.cpp```

## Delegates
It's quite common practice to extend your program; to use Event Orienting Programming. And delegates can help you to make more comfortable.
Let's look into the code:
```c++
#include "Core/Delegate.h"
#include <iostream>

int main()
{
    using namespace std;

    // At this line was created a delegate. So, it's just listener of some custom events and you can trigger subscribed function from any part of you block-scope.
    Core::Delegate<void()> delegate;

    bool wasInvoked = false;

    // Subscribing to the delegate. So, after function 'Trigger' your lambda will be invoked.
    auto id = delegate.Subscribe(
        [&]()
        {
            wasInvoked = true;
        });

    delegate.Trigger(); // Invoking of your lambda above.

    cout << (wasInvoked) << endl; // => true

    return 0;
}
```

If you want to get more complex cases & functionality just look at the test: ```tests/DelegateTest.cpp```

## Asserts
Sometimes you don't want to core user's programm especially in the enterprise\release version but want to say smth. you can use Assert function.
It's easy, let's do it:
```c++
#include "Core/Assert.h"
#include <iostream>

int main()
{
    using namespace std;

    constexpr const float tax = 0.05f;
    float bruttoSalary = 0;
    cout << "Brutto salary: ";
    cin >> bruttoSalary;

    Assert(yourMoney < 0.f, "Money can't be less then zero"); // At this moment you can see potentially OS-window with a described error.
    if (yourMoney >= 0.f)
    {
        cout << "Netto salary: " << bruttoSalary * (1.f - tax) << endl;
    }

    return 0;
}
```

## 📞 Feedback & Contacts

You can send me e-mail: Valerii.Koniushenko@gmail.com or try to find me in telegram: @markmoran24