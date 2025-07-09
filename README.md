# RISC-V Simulator

This project is a RISC-V simulator designed to execute RISC-V assembly instructions and provide insights into the internal workings of the simulated environment. The simulator supports handling registers, memory, and the call stack, allowing users to observe execution in real-time.

## Features

- **Register Management**: Displays current values of the 32 registers.
- **Memory Inspection**: Allows users to view memory contents at specified addresses.
- **Call Stack Handling**: Tracks function calls and displays the current call stack.
- **Error Handling**: Detects and reports common errors during instruction execution, such as invalid memory access.

## Directory Structure

The project directory contains the following files:

```
RISCV-Simulator/
├── simulator.h        
├── simulator.cpp     
├── main.cpp       
├── makefile       
├── README.md      
└── report.pdf     
```

## Prerequisites

To build and run this project, you need the following:

- **C++17 compatible compiler**: For example, `g++`
- **Makefile**: To automate the build process

## Usage

Once the project is built, you can run the simulator by providing a file containing RISC-V assembly instructions as input:

Command for running the simulator

```
make
./riscv_sim 
```

### Example

For an input file (`input.s`) containing the following assembly instructions:

```
lui x1, 0x200
add x2, x1, x3
```

Running the simulator:

```
make
./riscv_sim 
```

Will produce output showing the state of registers and memory after executing the instructions:

```
Registers:
x0  = 0x0
x1  = 0x200
x2  = 0x0
...
Memory[0x00000] = 0x200
...
Call Stack:
main: 0
```

### Supported Instructions

- **R-format**: `add`, `sub`, `xor`, `or`, `and`, `sll`, `srl`, `sra`
- **I-format**: `addi`, `xori`, `ori`, `andi`, `slli`, `srli`, `srai`, `lb`, `lh`, `lw`, `ld`, `lbu`, `lhu`, `lwu`, `jalr`
- **S-format**: `sb`, `sh`, `sw`, `sd`
- **B-format**: `beq`, `bne`, `blt`, `bge`, `bltu`, `bgeu`
- **J-format**: `jal`
- **U-format**: `lui`

## Clean Up

To remove the build files, use the `clean` command:

```bash
make clean
```
```

