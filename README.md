# nand2tetris
This is a C++ implementation of the Jack compiler, LLVM Interpreter, and the Hack Assembler as a part of the Nand2Tetris project.
Please visit www.nan2tetris.org for more details on the project.

## Compiling
The recommended compiler is the GNU C++14.
```bash
g++ -std=c++14 JACK_Compiler.cpp -o JACK_Compiler
g++ -std=c++14 LLVM_Interpreter.cpp -o LLVM_Interpreter
g++ -std=c++14 HACK_Assembler.cpp -o HACK_Assembler
```

## Usage
The usage of the 3 executables will be demonstrated with the help of an example, say, myprogram.jack .

The Jack Compiler is used as follows:
```bash
./JACK_Compiler myprogram.jack
```

This creates 4 output files, namely, myprogram.anz , myprogram.tok , myprogram.err , and myprogram.vm .
You may also compile many files at once, as follows:

```bash
./JACK_Compiler myprogram.jack myprogram2.jack
```

The LLVM Interpreter is used as follows:
```bash
./LLVM_Interpreter myprogram.vm myprogram.vmir myprogram.asm
```
This creates 2 output files, namely, myprogram.vmir , and myprogram.asm .

The HACK Assembler is used as follows:
```bash
./HACK_Assembler myprogram.asm myprogram.asmir myprogram.hack
```
This creates 2 output files, namely, myprogram.asmir , and myprogram.hack .
