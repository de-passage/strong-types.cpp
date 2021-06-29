This is the result of my life-long (OK, maybe just-a-couple-years-long) quest for the ultimate strong type engine for C++. Other solutions do exist but I haven't found anything that satisfies the level of granularity that I want for my strong type declarations.

# Strong types

What are strong types you say? Well, let's start from an example you've probably seen before: 
``` cpp
<complete stupid example with repeating types in function prototype here>
```
Now if you don't see what's wrong with the code above, this repo isn't for you. If, however, you enjoy robust code that can be modified and reused with confidence, the solution you're yearning for is *strong types*, the ability to produce a type that shares some or all of the characteristics of another, without being freely substitutable with the latter.
In other words, you'd like the following code to fail to compile:
``` cpp
typedef int barable;

void foo(int);
void bar(barable);

int i;
barable b;

foo(b); // fail here please
bar(i); // ugh...
```

Some day, we might get a nice standard solution to do that, but for now we have to resort to manually implement our strong types ourselves. There are several potential solutions to that problem:
1. declare that this isn't a real problem, stick with `int` everywhere and never have a civil conversation with me again
2. try to implement manually wrapper types everytime you need one before giving up and going back to option 1
3. write a template type that wraps any numeric type, some other common types and call it a day. That's not a bad solution, and there's a few options doing just that available online. It just doesn't quite satisfy my strong type OCD. I want to be able to define more precisely what different types can and cannot do.
4. implement some sort of code generator that you feed with some sort of meta-language in return for your C++ boilerplate. Again something I wouldn't consider unless your project is big enough with all the tooling necessary to support an extra step in your development process.
5. sacrifice your soul to the Great Old C++ Ones and acquire the forbidden knowledge of template metaprogramming, then proceed to summon with dark magic a meta-beast that will your bidding from within the code itself. This is the solution I chose here. It's always the solution I choose...

# Basic usage

``` cpp
#include <strong_types.hpp>

namespace st = dpsg::strong_types;

// complete the example
```