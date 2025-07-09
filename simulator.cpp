#include <string>
#include <cmath>
#include <iostream>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <stack>
#include "simulator.h"

using namespace std;
typedef unsigned long long ull;
typedef long long ll;

vector<ll> registers(32, 0);          // Initialize all registers to 0
unordered_map<string, string> memory; // Unordered map for storing memory
string initialMemory = "10000";       // Start of data section
stack<pair<string, int>> funStack;    // Stack for functions

// Store aliases and actual register pairs
unordered_map<string, string> regMap = {
    // Integer register aliases
    {"zero", "x0"},
    {"ra", "x1"},
    {"sp", "x2"},
    {"gp", "x3"},
    {"tp", "x4"},
    {"t0", "x5"},
    {"t1", "x6"},
    {"t2", "x7"},
    {"a0", "x10"},
    {"a1", "x11"},
    {"a2", "x12"},
    {"a3", "x13"},
    {"a4", "x14"},
    {"a5", "x15"},
    {"a6", "x16"},
    {"a7", "x17"},
    {"s0", "x8"},
    {"s1", "x9"},
    {"s2", "x18"},
    {"s3", "x19"},
    {"s4", "x20"},
    {"s5", "x21"},
    {"s6", "x22"},
    {"s7", "x23"},
    {"s8", "x24"},
    {"s9", "x25"},
    {"s10", "x26"},
    {"s11", "x27"},
    {"t3", "x28"},
    {"t4", "x29"},
    {"t5", "x30"},
    {"t6", "x31"},

    // Floating-point register aliases
    {"ft0", "f0"},
    {"ft1", "f1"},
    {"ft2", "f2"},
    {"ft3", "f3"},
    {"ft4", "f4"},
    {"ft5", "f5"},
    {"ft6", "f6"},
    {"ft7", "f7"},
    {"fa0", "f10"},
    {"fa1", "f11"},
    {"fa2", "f12"},
    {"fa3", "f13"},
    {"fa4", "f14"},
    {"fa5", "f15"},
    {"fa6", "f16"},
    {"fa7", "f17"},
    {"fs0", "f8"},
    {"fs1", "f9"},
    {"fs2", "f18"},
    {"fs3", "f19"},
    {"fs4", "f20"},
    {"fs5", "f21"},
    {"fs6", "f22"},
    {"fs7", "f23"},
    {"fs8", "f24"},
    {"fs9", "f25"},
    {"fs10", "f26"},
    {"fs11", "f27"},
    {"ft8", "f28"},
    {"ft9", "f29"},
    {"ft10", "f30"},
    {"ft11", "f31"}

};

// Function to convert hex to decimal
ll hexToDecimal(string hexStr)
{
    ull decimalValue = 0;
    ull base = 1;

    for (int i = hexStr.size() - 1; i >= 0; i--)
    {
        char ch = hexStr[i];
        if (ch >= '0' && ch <= '9')
        {
            decimalValue += (ch - '0') * base; // For values 0 to 9
        }
        else if (ch >= 'A' && ch <= 'F')
        {
            decimalValue += (ch - 'A' + 10) * base; // For values A to F
        }
        else if (ch >= 'a' && ch <= 'f')
        {
            decimalValue += (ch - 'a' + 10) * base; // For values a to f
        }
        base *= 16; // Raising power by 1
    }

    // Converting unsigned to signed
    if (hexStr[0] >= '8')
    {
        ll fullRange = pow(2, hexStr.size() * 4);
        decimalValue -= fullRange;
    }

    return decimalValue;
}

// Function to convert decimal to hex
string decimalToHex(ll number, int hexDigits)
{
    string hexStr;
    char ch;
    ull num = (ull)number;
    if (number < 0)
    {
        num = (ull)pow(2, hexDigits * 4) + num;
    }
    while (num > 0 && hexStr.length()<=hexDigits)
    {
        int digit = num % 16;
        if (digit < 10)
        {
            ch = digit + '0';
            hexStr = ch + hexStr;
        }
        else
        {
            ch = (digit - 10) + 'A';
            hexStr = ch + hexStr;
        }
        num /= 16;
    }
    while (hexStr.size() < hexDigits)
    {
        if (number < 0)
        {
            hexStr = 'F' + hexStr;
        }
        else
        {
            hexStr = '0' + hexStr;
        }
    }
    return hexStr;
}

// Function to check for decimal
bool isValidDecimal(const string &immediateStr)
{
    // Check if the immediate is a valid decimal number
    if (immediateStr.empty())
        return false;
    size_t i = 0;

    // If negative, start checking from the second character
    if (immediateStr[0] == '-')
        i = 1;

    // Ensure that every character is a digit
    for (; i < immediateStr.length(); i++)
    {
        if (!isdigit(immediateStr[i]))
        {
            cout << immediateStr[i] << endl;
            return false;
        }
    }

    return true;
}

// Function to convert registers to indices
int regToIndex(const string &reg)
{
    string actualReg = reg;

    // Check if the register is an alias
    if (regMap.find(reg) != regMap.end())
    {
        actualReg = regMap[reg]; // Get the corresponding x or f register
    }

    // If register is neither an alias nor actual register than return error
    if (actualReg[0] != 'x' && actualReg[0] != 'f')
    {
        cout << "Invalid register format: " << reg << endl;
        return -1;
    }
    if (!isValidDecimal(actualReg.substr(1)))
    {
        cout << "Invalid register format: " << reg << endl;
        return -1;
    }
    int regNum = stoi(actualReg.substr(1));
    if (regNum > 31 || regNum < 0)
    {
        cout << "Invalid register number: " << regNum << endl;
        return -1;
    }
    return regNum;
}

// Function to run R format Instructions
void runRFormat(const string &instruction)
{
    string operation, rd, rs1, rs2;
    size_t start = 0;
    size_t end = 0;

    // Extract the operation by checking the first space
    end = instruction.find(' ', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing space after the operation." << endl;
        return;
    }
    operation = instruction.substr(start, end - start);

    // Skip space
    start = end + 1;

    // Extract rd by checking the first comma
    end = instruction.find(',', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing comma after rd." << endl;
        return;
    }
    rd = instruction.substr(start, end - start);

    // Skip space after comma
    start = end + 1;
    if (start >= instruction.length() || instruction[start] != ' ')
    {
        cerr << "Error: Expected space after comma following rd." << endl;
        return;
    }
    start++;

    // Extract rs1 by checking the next comma
    end = instruction.find(',', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing comma after rs1." << endl;
        return;
    }
    rs1 = instruction.substr(start, end - start);

    // Skip comma and space
    start = end + 1;
    if (start >= instruction.length() || instruction[start] != ' ')
    {
        cerr << "Error: Expected space after comma following rs1." << endl;
        return;
    }
    start++;

    // Extract rs2 (the rest of the string)
    rs2 = instruction.substr(start);
    if (rs2.empty() || rs2.find(' ') != string::npos)
    {
        cerr << "Error: Too few or too many parameters." << endl;
        return;
    }

    // Convert registers to indices
    int rdIndex = regToIndex(rd);
    int rs1Index = regToIndex(rs1);
    int rs2Index = regToIndex(rs2);

    if (rdIndex == -1 || rs1Index == -1 || rs2Index == -1)
    {
        cerr << "Error: Invalid register format." << endl;
        return;
    }

    // Perform operation based on instruction
    if (operation == "add")
    {
        registers[rdIndex] = registers[rs1Index] + registers[rs2Index];
    }
    else if (operation == "sub")
    {
        registers[rdIndex] = registers[rs1Index] - registers[rs2Index];
    }
    else if (operation == "xor")
    {
        registers[rdIndex] = registers[rs1Index] ^ registers[rs2Index];
    }
    else if (operation == "or")
    {
        registers[rdIndex] = registers[rs1Index] | registers[rs2Index];
    }
    else if (operation == "and")
    {
        registers[rdIndex] = registers[rs1Index] & registers[rs2Index];
    }
    else if (operation == "sll")
    {
        registers[rdIndex] = registers[rs1Index] << registers[rs2Index];
    }
    else if (operation == "srl")
    {
        ull value = registers[rs1Index];
        registers[rdIndex] = (unsigned)(value >> registers[rs2Index]);
    }
    else if (operation == "sra")
    {
        registers[rdIndex] = registers[rs1Index] >> registers[rs2Index];
    }
    else
    {
        cerr << "Error: Unsupported R-format operation: " << operation << endl;
        return;
    }
}

// Function to run I format Instructions
void runIFormat(const string &instruction, int &currentLine)
{
    string operation;
    size_t start = 0;
    size_t end = 0;

    // Extract the operation by checking for space
    end = instruction.find(' ', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing space after the operation." << endl;
        return;
    }
    operation = instruction.substr(start, end - start);

    string opcode;

    if (operation == "addi" || operation == "xori" || operation == "ori" || operation == "andi" ||
        operation == "slli" || operation == "srli" || operation == "srai")
    {
        opcode = "0010011";
    }

    if (operation == "lb" || operation == "lh" || operation == "lw" || operation == "ld" ||
        operation == "lbu" || operation == "lhu" || operation == "lwu")
    {
        opcode = "0000011";
    }

    if (operation == "jalr")
        opcode = "1100111";

    string binaryInstruction;

    if (opcode == "0010011")
    {
        string rd, rs1, immediate;

        // Skip space after the operation
        end++;
        start = end;

        // Extract rd by checking for first comma
        end = instruction.find(',', start);
        if (end == string::npos)
        {
            cerr << "Error: Missing comma after rd." << endl;
            return;
        }
        rd = instruction.substr(start, end - start);

        // Skip comma and check if space is present after it
        start = end + 1;
        if (start >= instruction.length() || instruction[start] != ' ')
        {
            cerr << "Error: Expected space after comma following rd." << endl;
            return;
        }
        start++;

        // Extract rs1 by checking the next comma
        end = instruction.find(',', start);
        if (end == string::npos)
        {
            cerr << "Error: Missing comma after rs1." << endl;
            return;
        }
        rs1 = instruction.substr(start, end - start);

        // Skip comma and check if space is present after it
        start = end + 1;
        if (start >= instruction.length() || instruction[start] != ' ')
        {
            cerr << "Error: Expected space after comma following rs1." << endl;
            return;
        }
        start++;
        end = instruction.find(' ', start);
        // Extract immediate
        immediate = instruction.substr(start, end - start);

        if (!isValidDecimal(immediate))
        {
            cerr << "Error: Immediate value must be a valid decimal number." << endl;
            return;
        }

        // Convert immediate to integer and check range for general immediates
        int immediateValue = stoi(immediate);
        if (immediateValue < -2048 || immediateValue > 2047)
        {
            cerr << "Error: Immediate value out of range (-2048 to 2047)." << endl;
            return;
        }

        // Special range check for slli, srli, and srai
        if (operation == "slli" || operation == "srli" || operation == "srai")
        {
            if (immediateValue < 0 || immediateValue > 63)
            {
                cerr << "Error: Immediate value out of range (0 to 63) for shift operations." << endl;
                return;
            }
        }

        // Convert registers to indices
        int rdIndex = regToIndex(rd);
        int rs1Index = regToIndex(rs1);

        // Perform operation based on instruction
        if (operation == "addi")
        {
            registers[rdIndex] = registers[rs1Index] + immediateValue;
        }
        else if (operation == "xori")
        {
            registers[rdIndex] = registers[rs1Index] ^ immediateValue;
        }
        else if (operation == "ori")
        {
            registers[rdIndex] = registers[rs1Index] | immediateValue;
        }
        else if (operation == "andi")
        {
            registers[rdIndex] = registers[rs1Index] & immediateValue;
        }
        else if (operation == "slli")
        {
            registers[rdIndex] = registers[rs1Index] << immediateValue; // left shift
        }
        else if (operation == "srli")
        {
            ull value = registers[rs1Index];
            registers[rdIndex] = (value >> immediateValue) & ((1ULL << (64 - immediateValue)) - 1); // Converts to unsigned value
        }
        else if (operation == "srai")
        {
            registers[rdIndex] = registers[rs1Index] >> immediateValue; // arithmetic shift
        }
    }
    else
    {
        string rd, immediateWithRegister;
        end++;
        start = end;

        // Extract rs1 by checking the first comma
        while (end < instruction.length() && instruction[end] != ',')
        {
            end++;
        }
        rd = instruction.substr(start, end - start);

        // Skip comma and space
        end += 2;
        start = end;
        // Extract immediate
        immediateWithRegister = instruction.substr(start);

        // Find the position of '(' to split the immediate value and the register
        size_t openParenPos = immediateWithRegister.find('(');
        size_t closeParenPos = immediateWithRegister.find(')');

        if (openParenPos == string::npos || closeParenPos == string::npos || openParenPos >= closeParenPos)
        {
            cout << "Invalid S-format instruction syntax: " << instruction << endl;
            return;
        }

        // Extract immediate and base register
        string immediateStr;
        string baseReg;
        if (immediateWithRegister[0] == 'x')
        {
            baseReg = immediateWithRegister.substr(0, openParenPos);
            immediateStr = immediateWithRegister.substr(openParenPos + 1, closeParenPos - openParenPos - 1);
        }
        else
        {
            immediateStr = immediateWithRegister.substr(0, openParenPos);                               // Immediate value as a string
            baseReg = immediateWithRegister.substr(openParenPos + 1, closeParenPos - openParenPos - 1); // Base register
        }

        // Check if the immediate is a valid decimal number
        if (!isValidDecimal(immediateStr))
        {
            cerr << "Error: Immediate value must be a valid decimal number." << endl;
            return;
        }

        // Convert immediate to integer and check range (-2048 to 2047)
        int immediateValue = stoi(immediateStr);
        if (immediateValue < -2048 || immediateValue > 2047)
        {
            cerr << "Error: Immediate value out of range (-2048 to 2047)." << endl;
            return;
        }

        // Convert registers to indices
        int rs1Index = regToIndex(baseReg);
        int rdIndex = regToIndex(rd);

        if (rdIndex == -1 || rs1Index == -1)
        {
            return;
        }

        ull addr = (ull)registers[rs1Index] + (ull)immediateValue; // Calculate the address
        if (operation == "ld")
        {
            string hexStr = "";
            for (int i = 0; i < 8; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            registers[rdIndex] = hexToDecimal(hexStr); // Stores the value after converting it to decimal
        }
        else if (operation == "lw")
        {
            string hexStr = "";
            for (int i = 0; i < 4; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            registers[rdIndex] = hexToDecimal(hexStr); // Stores the value after converting it to decimal
        }
        else if (operation == "lh")
        {
            string hexStr = "";
            for (int i = 0; i < 2; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            registers[rdIndex] = hexToDecimal(hexStr); // Stores the value after converting it to decimal
        }
        else if (operation == "lb")
        {
            string hexStr = "";
            for (int i = 0; i < 1; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            registers[rdIndex] = hexToDecimal(hexStr); // Stores the value after converting it to decimal
        }
        else if (operation == "lwu")
        {
            string hexStr = "";
            for (int i = 0; i < 4; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            if (hexToDecimal(hexStr) < 0)
                registers[rdIndex] = hexToDecimal(hexStr) + pow(2, 32); // Converts to deciaml and then unsigned if negative number
            else
                registers[rdIndex] = hexToDecimal(hexStr); //  Stores the value after converting it to decimal
        }
        else if (operation == "lhu")
        {
            string hexStr = "";
            for (int i = 0; i < 2; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            if (hexToDecimal(hexStr) < 0)
                registers[rdIndex] = hexToDecimal(hexStr) + pow(2, 16); // Converts to deciaml and then unsigned if negative number
            else
                registers[rdIndex] = hexToDecimal(hexStr); //  Stores the value after converting it to decimal
        }
        else if (operation == "lbu")
        {
            string hexStr = "";
            for (int i = 0; i < 1; i++)
            {
                hexStr = memory[decimalToHex(addr + i, 5)] + hexStr; // Adds the whole value in little endian format
            }
            if (hexToDecimal(hexStr) < 0)
                registers[rdIndex] = hexToDecimal(hexStr) + pow(2, 8); // Converts to deciaml and then unsigned if negative number
            else
                registers[rdIndex] = hexToDecimal(hexStr); //  Stores the value after converting it to decimal
        }
        else if (operation == "jalr")
        {
            currentLine = registers[rs1Index] / 4 - 1;
            funStack.pop();
        }
    }
}

// Function to run B format Instructions
void runBFormat(const string &instruction, const unordered_map<string, int> &labelAddresses, int &currentLine)
{
    string operation, rs1, rs2, label;
    size_t start = 0;
    size_t end = 0;

    // Extract the operation by checking empty space
    end = instruction.find(' ', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing space after the operation." << endl;
        return;
    }
    operation = instruction.substr(start, end - start);

    // Skip space
    start = end + 1;

    if (start >= instruction.length())
    {
        cerr << "Error: Missing rs1 after the operation." << endl;
        return;
    }

    // Extract rs1 by checking the fisrt comma
    end = instruction.find(',', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing comma after rs1." << endl;
        return;
    }
    rs1 = instruction.substr(start, end - start);

    // Skip comma and check if space is present after it
    start = end + 1;
    if (start >= instruction.length() || instruction[start] != ' ')
    {
        cerr << "Error: Expected space after comma following rs1." << endl;
        return;
    }
    start++;

    // Extract rs2 by checking next comma
    end = instruction.find(',', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing comma after rs2." << endl;
        return;
    }
    rs2 = instruction.substr(start, end - start);

    // Skip comma and check if space is present after it
    start = end + 1;
    if (start >= instruction.length() || instruction[start] != ' ')
    {
        cerr << "Error: Expected space after comma following rs2." << endl;
        return;
    }
    start++;

    // Extract label
    label = instruction.substr(start);
    if (label.empty())
    {
        cerr << "Error: Missing label after rs2." << endl;
        return;
    }

    // Convert registers to indices
    int reg1Index = regToIndex(rs1);
    int reg2Index = regToIndex(rs2);
    if (reg1Index == -1 || reg2Index == -1)
    {
        return;
    }

    // Find values in both register
    ll reg1Value = registers[reg1Index];
    ll reg2Value = registers[reg2Index];

    // Check if label exists
    if (labelAddresses.find(label) == labelAddresses.end())
    {
        cerr << "Error: Label not found." << endl;
        return;
    }

    int targetLine = labelAddresses.at(label);
    ull reg1Unsigned;
    ull reg2Unsigned;
    if(reg1Value<0) reg1Unsigned = reg1Value + pow(2,64);
    else reg1Unsigned = reg1Value;
    if(reg2Value<0) reg2Unsigned = reg2Value + pow(2,64);
    else reg2Unsigned = reg2Value;
    // Branch condition checks based on operation
    if ((operation == "beq" && reg1Value == reg2Value) ||
        (operation == "bne" && reg1Value != reg2Value) ||
        (operation == "blt" && reg1Value < reg2Value) ||
        (operation == "bge" && reg1Value >= reg2Value) ||
        (operation == "bltu" && reg1Unsigned < reg2Unsigned) ||
        (operation == "bgeu" && reg1Unsigned >= reg2Unsigned))
    {
        currentLine = targetLine - 1; // Adjust to zero-based indexing
    }
}

// Function to run S format Instructions
void runSFormat(const string &instruction)
{
    string operation, rs1, rs2, immediateWithRegister;
    size_t start = 0;
    size_t end = 0;

    // Extract the operation by checking space
    while (end < instruction.length() && instruction[end] != ' ')
    {
        end++;
    }
    operation = instruction.substr(start, end - start);

    // Skip space
    end++;
    start = end;

    // Extract rs1 by checking next comma
    while (end < instruction.length() && instruction[end] != ',')
    {
        end++;
    }
    rs2 = instruction.substr(start, end - start);

    // Skip comma and space
    end += 2;
    start = end;

    // Extract immediate
    immediateWithRegister = instruction.substr(start);

    // Find the position of '(' to split the immediate value and the register
    size_t openParenPos = immediateWithRegister.find('(');
    size_t closeParenPos = immediateWithRegister.find(')');

    if (openParenPos == string::npos || closeParenPos == string::npos || openParenPos >= closeParenPos)
    {
        cout << "Invalid S-format instruction syntax: " << instruction << endl;
        return;
    }

    // Extract immediate and base register
    string immediateStr = immediateWithRegister.substr(0, openParenPos);                               // Immediate value as a string
    string baseReg = immediateWithRegister.substr(openParenPos + 1, closeParenPos - openParenPos - 1); // Base register
    // Validate immediate
    if (!isValidDecimal(immediateStr))
    {
        cerr << "Error: Immediate value must be a valid decimal number." << endl;
        return;
    }

    // Convert immediate string to integer and check its range (-2048 to 2047 for 12-bit S-format instructions)
    int immediateValue = stoi(immediateStr);
    if (immediateValue < -2048 || immediateValue > 2047)
    {
        cerr << "Error: Immediate value out of range (-2048 to 2047)." << endl;
        return;
    }

    // Convert registers to indices
    int reg1Index = regToIndex(baseReg);
    int reg2Index = regToIndex(rs2);

    if (reg1Index == -1 || reg2Index == -1)
    {
        return;
    }

    ull address = registers[reg1Index] + (ull)immediateValue;// Calculate address
    if (operation == "sd")
    {
        string numHex = decimalToHex(registers[reg2Index], 16);
        for (int i = 0; i < 8; i++)
        {
            memory[decimalToHex(address + i, 5)] = numHex.substr(16 - (i + 1) * 2, 2);// Store 8 bytes at time
        }
    }
    else if (operation == "sw")
    {
        string numHex = decimalToHex(registers[reg2Index], 8);
        for (int i = 0; i < 4; i++)
        {
            memory[decimalToHex(address + i, 5)] = numHex.substr(8 - (i + 1) * 2, 2);// Store 8 bytes at time
        }
    }
    else if (operation == "sh")
    {
        string numHex = decimalToHex(registers[reg2Index], 4);
        for (int i = 0; i < 2; i++)
        {
            memory[decimalToHex(address + i, 5)] = numHex.substr(4 - (i + 1) * 2, 2);// Store 8 bytes at time
        }
    }
    else if (operation == "sb")
    {
        string numHex = decimalToHex(registers[reg2Index], 2);// Store 8 bytes at time
        memory[decimalToHex(address, 5)] = numHex;
    }
}

// Function to run J format Instructions
void runJFormat(const string &instruction, const unordered_map<string, int> &labelAddresses, int &currentLine)
{
    string operation, rd, label;
    size_t start = 0;
    size_t end = 0;

    // Extract the operation by checking the first space
    end = instruction.find(' ', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing space after the operation." << endl;
        return;
    }
    operation = instruction.substr(start, end - start);

    // Skip space
    start = end + 1;
    if (start >= instruction.length())
    {
        cerr << "Error: Missing rd after the operation." << endl;
        return;
    }

    // Extract rd by checking the first comma
    end = instruction.find(',', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing comma after rd." << endl;
        return;
    }
    rd = instruction.substr(start, end - start);

    // Skip comma and check if space is present after it
    start = end + 1;
    if (start >= instruction.length() || instruction[start] != ' ')
    {
        cerr << "Error: Expected space after comma following rd." << endl;
        return;
    }
    start++;

    // Extract label
    label = instruction.substr(start);
    if (label.empty())
    {
        cerr << "Error: Missing label after rd." << endl;
        return;
    }

    // Convert registers to indices
    int rdIndex = regToIndex(rd);

    if (rdIndex == -1)
    {
        return;
    }

    // Check if the immediate is a label or a number
    if (labelAddresses.find(label) != labelAddresses.end())
    {
        int targetLine = labelAddresses.at(label);
        registers[rdIndex] = (currentLine + 1) * 4;
        funStack.push({label, currentLine + 1});
        currentLine = targetLine - 1; // Calculate the jump
    }
    else
    {
        cerr << "Label Not Found." << endl;
        return;
    }
}

// Function to run U format Instructions
void runUFormat(const string &instruction)
{
    string operation, rd, immediate;
    size_t start = 0;
    size_t end = 0;

    // Extract the operation by checking the first space
    end = instruction.find(' ', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing space after the operation." << endl;
        return;
    }
    operation = instruction.substr(start, end - start);

    // Skip space
    start = end + 1;
    if (start >= instruction.length())
    {
        cerr << "Error: Missing rd after the operation." << endl;
        return;
    }

    // Extract rd until comma
    end = instruction.find(',', start);
    if (end == string::npos)
    {
        cerr << "Error: Missing comma after rd." << endl;
        return;
    }
    rd = instruction.substr(start, end - start);

    // Skip comma and space
    start = end + 2; // Skip the comma and space after rd
    if (start >= instruction.length())
    {
        cerr << "Error: Missing immediate after rd." << endl;
        return;
    }

    // Extract immediate
    immediate = instruction.substr(start);
    if (immediate.empty())
    {
        cerr << "Error: Missing immediate after rd." << endl;
        return;
    }
    ull immediateValue = 0;
    if (immediate[1] = 'x')
    {
        // Handles hex immediate 
        immediate = immediate.substr(2);
        immediateValue = (ull)hexToDecimal(immediate);
    }
    else
    {
        // Handles decimal immediate
        if (!isValidDecimal(immediate))
        {
            cerr << "Error: Immediate value must be a valid decimal number." << endl;
            return;
        }

        // Convert immediate to integer and check range
        immediateValue = (ull)stoi(immediate);
    }

    if (immediateValue < 0 || immediateValue > 1048575)
    {
        cerr << "Error: Immediate value out of range (0 to 1048575)." << endl;
        return;
    }

    // Convert registers to indices
    int rdIndex = regToIndex(rd);

    if (rdIndex == -1)
    {
        cerr << "Error: Invalid register format." << endl;
        return;
    }
    registers[rdIndex] = immediateValue * 4096;
}

void runInstruction(const string &instruction, int &lineNumber, const unordered_map<string, int> &labelAddresses)
{
    string operation;
    size_t i = 0;

    // Extract the operation (until the first space)
    while (i < instruction.length() && instruction[i] != ' ')
    {
        i++;
    }
    operation = instruction.substr(0, i);

    // R-format instructions
    if (operation == "add" || operation == "sub" || operation == "xor" || operation == "or" ||
        operation == "and" || operation == "sll" || operation == "srl" || operation == "sra")
    {
        runRFormat(instruction);
    }

    // I-format instructions
    else if (operation == "addi" || operation == "xori" || operation == "ori" || operation == "andi" ||
             operation == "slli" || operation == "srli" || operation == "srai" || operation == "lb" ||
             operation == "lh" || operation == "lw" || operation == "ld" || operation == "lbu" ||
             operation == "lhu" || operation == "lwu" || operation == "jalr")
    {
        runIFormat(instruction, lineNumber);
    }

    // S-format instructions
    else if (operation == "sb" || operation == "sh" || operation == "sw" || operation == "sd")
    {
        runSFormat(instruction);
    }

    // B-format instructions
    else if (operation == "beq" || operation == "bne" || operation == "blt" || operation == "bge" ||
             operation == "bltu" || operation == "bgeu")
    {
        runBFormat(instruction, labelAddresses, lineNumber);
    }

    // J-format instructions
    else if (operation == "jal")
    {
        runJFormat(instruction, labelAddresses, lineNumber);
    }

    // U-format instructions
    else if (operation == "lui")
    {
        runUFormat(instruction);
    }

    // If instruction not found then gives error
    else
    {
        cout << "Unknown instruction format at line " << lineNumber + 1 << endl;
    }
}

// Function to print register values
void printRegisters()
{
    cout << "Registers:" << endl;
    for (int i = 0; i < 32; i++)
    {
        string regHex = decimalToHex(registers[i], 16);// Convert deciaml to hex
        while (regHex[0] == '0' && regHex.length() > 1)
            regHex = regHex.substr(1);
        if (i > 9)
            cout << "x" << i << " = 0x" << regHex << endl;
        else
            cout << "x" << i << "  = 0x" << regHex << endl;
    }
}

// Function to print memory
void printMemory(string address, int count)
{
    ll addr = hexToDecimal(address.substr(2));
    for (int i = addr; i < addr + count; i++)
    {
        if(i<65336 || i>327680){
            cerr << "Cannot view memory beyond data section";
            return;
        } 
        string location = decimalToHex(i, 5);
        cout << "Memory[0x" << location << "] = 0x";
        if (memory[location] == "")
            cout << "0" << endl;
        else
            cout << memory[location] << endl;
    }
}

// Function to set register values back to 0
void resetRegisters()
{
    for (int i = 0; i < 32; i++)
    {
        registers[i] = 0;
    }
}

// Function to set memory values back to 0
void resetMemory()
{
    memory.clear();
}

// Function to store .dword values from data section in memory
void setDoubleword(vector<string> dataValues)
{
    // Case when data values are decimal
    if (dataValues[0].substr(0, 2) != "0x")
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            string decimal = dataValues[i];
            if (isValidDecimal(decimal))
            {
                ll temp = (ll)stoll(decimal);
                dataValues[i] = decimalToHex(temp, 16);// Double words are of 64 bytes 
            }
            else
            {
                cerr << "Invalid data input." << endl;
                return;
            }
        }
    }
    // Case when data values are hex
    else
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            dataValues[i] = dataValues[i].substr(2);
        }
    }
    // Storing values 8 byte at a time
    for (int i = 0; i < dataValues.size(); i++)
    {
        ll addr = hexToDecimal(initialMemory);
        while (dataValues[i].length() < 16)
            dataValues[i] = "0" + dataValues[i];
        for (int j = addr; j < addr + 8; j++)
        {
            string location = decimalToHex(j, 5);
            memory[location] = dataValues[i].substr(16 - (j - addr + 1) * 2, 2);
        }
        initialMemory = decimalToHex(addr + 8, 5);
    }
}

// Function to store .half values from data section in memory
void setHalfword(vector<string> dataValues)
{
    // Case when data values are decimal
    if (dataValues[0].substr(0, 2) != "0x")
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            string decimal = dataValues[i];
            if (isValidDecimal(decimal))
            {
                ll temp = (ll)stoi(decimal);
                dataValues[i] = decimalToHex(temp, 4);// Half words are of 16 bytes
            }
            else
            {
                cerr << "Invalid data input." << endl;
                return;
            }
        }
    }
    // Case when data values are hex
    else
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            dataValues[i] = dataValues[i].substr(2);
        }
    }
    // Storing values 8 byte at a time
    for (int i = 0; i < dataValues.size(); i++)
    {
        ll addr = hexToDecimal(initialMemory);
        while (dataValues[i].length() < 4)
            dataValues[i] = "0" + dataValues[i];
        for (int j = addr; j < addr + 2; j++)
        {
            string location = decimalToHex(j, 5);
            memory[location] = dataValues[i].substr(4 - (j - addr + 1) * 2, 2);
        }
        initialMemory = decimalToHex(addr + 2, 5);
    }
}

// Function to store .word values from data section in memory
void setWord(vector<string> dataValues)
{
    // Case when data values are decimal
    if (dataValues[0].substr(0, 2) != "0x")
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            string decimal = dataValues[i];
            if (isValidDecimal(decimal))
            {
                ll temp = (ll)stoi(decimal);
                dataValues[i] = decimalToHex(temp, 8);// Words are of 64 bytes
            }
            else
            {
                cerr << "Invalid data input." << endl;
                return;
            }
        }
    }
    // Case when data values are hex
    else
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            dataValues[i] = dataValues[i].substr(2);
        }
    }
    // Storing values 8 byte at a time
    for (int i = 0; i < dataValues.size(); i++)
    {
        ll addr = hexToDecimal(initialMemory);
        while (dataValues[i].length() < 8)
            dataValues[i] = "0" + dataValues[i];
        for (int j = addr; j < addr + 4; j++)
        {
            string location = decimalToHex(j, 5);
            memory[location] = dataValues[i].substr(8 - (j - addr + 1) * 2, 2);
        }
        initialMemory = decimalToHex(addr + 4, 5);
    }
}

// Function to store .byte values from data section in memory
void setByte(vector<string> dataValues)
{
    // Case when data values are decimal
    if (dataValues[0].substr(0, 2) != "0x")
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            string decimal = dataValues[i];
            if (isValidDecimal(decimal))
            {
                ll temp = (ll)stoi(decimal);
                dataValues[i] = decimalToHex(temp, 2);
            }
            else
            {
                cerr << "Invalid data input." << endl;
                return;
            }
        }
    }
    // Case when data values are hex
    else
    {
        for (int i = 0; i < dataValues.size(); i++)
        {
            dataValues[i] = dataValues[i].substr(2);
        }
    }
    // Storing values 8 byte at a time
    for (int i = 0; i < dataValues.size(); i++)
    {
        ll addr = hexToDecimal(initialMemory);
        while (dataValues[i].length() < 2)
            dataValues[i] = "0" + dataValues[i];
        for (int j = addr; j < addr + 1; j++)
        {
            string location = decimalToHex(j, 5);
            memory[location] = dataValues[i].substr(2 - (j - addr + 1) * 2, 2);
        }
        initialMemory = decimalToHex(addr + 1, 5);
    }
}

// Function to display the stack
void showStack(int extraLines)
{
    if (funStack.empty())
    {
        cout << "Empty Call Stack: Execution complete" << endl;
        return;
    }
    // Flips stack to display it from bottom
    stack<pair<string, int>> temp;
    while (!funStack.empty())
    {
        pair function = funStack.top();
        temp.push(function);
        funStack.pop();
    }
    cout << "Call Stack:" << endl;
    // Flips again to restore the original stack
    while (!temp.empty())
    {
        pair function = temp.top();
        cout << function.first << ":" << function.second+extraLines << endl;
        funStack.push(function);
        temp.pop();
    }
    cout << endl;
}

// Function to create stack
void createStack(unordered_map<string, int> &labelAddresses)
{
    if (labelAddresses.find("main") != labelAddresses.end())
    {
        funStack.push({"main", 0});
    }
}

// Function to update value of stack after executing every line
void handleStack(unordered_map<string, int> &labelAddresses, int lineNumber)
{
    if (!funStack.empty())
    {
        pair currentFun = funStack.top();
        funStack.pop();
        currentFun.second = lineNumber;
        funStack.push(currentFun);
    }
}

// Function to delete/empty stack after complete execution
void deleteStack()
{
    while (!funStack.empty())
    {
        funStack.pop();
    }
}