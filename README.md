# `lang808c`

_This was a course project for my Master's Degree in Computer Engineering at the University of Maryland._

## Scope

I chose to write a compiler totally from scratch for a simple language that I made up. The language was designed to be simple enough to write a compiler for in a semester, but also have some unique features. Specifically, the programming language supports novel features that make writing firmware for ARM Cortex-M chips easier. For demonstration and feasibility purposes, the compiler is tailored for generating executable code for a specific chip, the [SAMD21](https://www.microchip.com/en-us/product/ATSAMD21J18).

The course number was 808, so I called the language `lang808` and the compiler `lang808c`. 
I would have preferred to write the compiler in any language that has decent string manipulation and data structures built-in, but I was required to write it in C. Since I was writing it in C, I had some fun with memory management. Each type of object is kept in a static contiguous stack-allocated arena. This didn't scale particularly well, but it was fun to play with and good-enough for this project (and there wasn't a single bug from misuse of `malloc` or `free`).

The compiler is truly "from-scratch". Upon execution, `lang808c` will:
- read `lang808` source code, 
- parse it (recursive descent),
- generate an abstract syntax tree,
- generate a custom intermediate representation (IR),
- generate [ARMv6M](https://developer.arm.com/documentation/ddi0419/latest/armv6-m-architecture-reference-manual) _machine_ code,
- link the functions together,
- and construct a file in [Intel HEX format](https://en.wikipedia.org/wiki/Intel_HEX).

That hex file can then be flashed directly to a SAMD21 chip using a tool like [OpenOCD](https://openocd.org/).


## Cool features

`lang808` contains three special features for ARM firmware development in addition to basic imperative programming support.

The first special feature is a keyword and structure to define a hardware peripheral on an ARM chip that is mapped to a piece of memory. Here is an example with inline comments explaining the feature.

```python
# This is a snippet of the real peripheral definition of the Real Time Clock
# (RTC) for the SAMD21 chip.
# The "MemoryMappedPeripheral" keyword tells the predictive parser that what
# follows is a hardware peripheral definition.
# "RTC" is the name of this peripheral.
# "@0x40001400" tells the compiler that this peripheral's registers are mapped
# to memory with a base address at 0x4001400.
# "!3" tells the compiler that there is an interrupt associated with the RTC
# and that its interrupt vector is the 3rd in the ARM Vector table.
MemoryMappedPeripheral RTC @0x40001400 !3 {
    # Inside the braces, the individual registers of the peripheral are defined
    # This first register is named "ctrl" and it is given the type "BitField16"
    # "BitField16" means that it is 16 bits long and contains multiple fields
    # within it.
    ctrl: BitField16 {
        # Inside the braces after "BitField16", the individual fields of the
        # register are defined.
        # This first field is named "swrst" and it is 1 bit long.
        swrst: 1;
        enable: 1;
        # A field within a BitField can also have enumerated values.
        # The "mode" field here is defined as a "BitEnum2", which means it is
        # 2 bits long, and can have the following enumerated values.
        mode: BitEnum2 {
            # Inside the braces after "BitEnum2", the individual enumerated
            # values are defined.
            # This first value is named "count32" and is the value 0.
            # The values for "mode" can only be referenced by their name as
            # specified here.
            count32 = 0x0;
            count16 = 0x1;
            clock = 0x2;
        };
        # "$unused" is a special keyword when there are bits that are unassigned
        # or reserved and shouldn't be touched by the firmware.
        $unused: 3;
        matchclr: 1;
        prescaler: BitEnum4 {
            div1 = 0x0;
            div2 = 0x1;
            div4 = 0x2;
            div8 = 0x3;
            div16 = 0x4;
            div32 = 0x5;
            div64 = 0x6;
            div128 = 0x7;
            div256 = 0x8;
            div512 = 0x9;
            div1024 = 0xa;
        };
        $unused: 3;
    };
    intenset: BitField8 {
        comp0: 1;
        $unused: 5;
        syncrdy: 1;
        ovf: 1;
    };
    # The "$unused" keyword also works for registers
    $unused: u16;
    # This register in the RTC peripheral is simpler than the previous one.
    # It is named "count" and is just a 32 bit integer.
    count: u32;
    comp0: u32;
}
```

_After this project, I learned about [devicetree](https://www.devicetree.org/), which already has a nice syntax for defining these kinds of things. If I were to do this again, I might use devicetree instead of rolling my own syntax._


The second special feature of `lang808` is the peripheral initialization support. It allows a simple specification of how hardware peripherals should be initialized when the device is reset. Here is an example with inline comments explaining the feature.

```python
# The "initialize" keyword tells the compiler that what follows is an
# initialization block for a particular hardware peripheral.
# "RTC" tells the compiler that the initialization block is for the RTC
# peripheral.
initialize RTC {
    # Inside the braces, we can have any number of assignments to registers
    # defined in the "RTC" peripheral.
    # This first assignment assigns a 0 to the "count" register.
    count = 0x0;
    comp0 = 0x4000;
    # "intenset" is defined as a BitField8 with fields, so we use a different
    # syntax with braces to assign a value to it.
    intenset = {
        # Inside these braces we can assign values to any of the fields within
        # register. We don't have to include every field. Any field that is not
        # given a value explicitly gets the value 0.
        # Here we only give the "comp0" field the value of 1, and all other
        # fields get the value 0.
        comp0 = 1;
    };
    ctrl = {
        enable = 1;
        # Here is an example of using the name of an enumerated value of a
        # field within a register.
        # "count32" is defined as having the value 0 in the "BitEnum"
        # definition of "mode".
        mode = count32;
        matchclr = 1;
    };
}
```

The third and final special feature of Lang808 is the ability to easily configure an interrupt vector with a custom function. Here is an example with inline comments explaining the feature.

```python
# The "on_interrupt" keyword tells the compiler that what follows is a function
# that should be configured as the interrupt vector for the interrupt
# associated with a particular hardware peripheral.
# "RTC" tells the compiler that the interrupt is for the RTC peripheral. The
# compiler knows that the RTC's interrupt is 3rd in the ARM Vector table from
# the peripheral definition.
# The compiler will automatically add this function to the ARM Vector table in
# the correct position.
# The compiler will also automatically add initialization code to the reset
# function that enables this interrupt in the ARM Nested Vector Interrupt
# Controller (NVIC).
on_interrupt RTC {
    # The contents of the interrupt are a normal function, so you can do
    # anything you would normally want to do, including use local variables,
    # call functions, use if statements, etc.
    u32 curr_comp = 0;
    u32 next_comp = 0;

    blinkLED();

    curr_comp = RTCMode0.comp0;

    if (going_faster == 1) {
        next_comp = curr_comp - 0x200;
    } else {
        next_comp = curr_comp + 0x200;
    }

    if (next_comp == 0x4000) {
        going_faster = 1;
    }
    if (next_comp == 0x1000) {
        going_faster = 0;
    }

    RTCMode0.comp0 = next_comp;

    RTCMode0.intflag = {
        comp0 = 1;
    };
    
    # At the end of an on_interrupt function, the compiler inserts code
    # that automtically clears the interrupt in the NVIC.
}
```

Some of the more mundane things that the compiler supports are:
- Global variables
- Functions with arguments and return values
- Local variables
- If-else statements
- While loops
- Expressions including bit shifting, addition, subtraction, bitwise-and, and comparison operators

## Examples

I include [three example programs](https://github.com/orndorffgrant/lang808c/tree/main/examples) that demonstrate the language's features on the [SAMD21 Xplained Pro board](https://www.microchip.com/en-us/development-tool/ATSAMD21-XPRO).
