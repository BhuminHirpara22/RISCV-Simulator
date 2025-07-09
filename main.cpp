#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <bitset>
#include "simulator.h" // Header file for simulator functions

using namespace std;
typedef long long ll;

unordered_map<string, int> labelAddresses; // Store labels and their line numbers
vector<string> instructionList; // Store instructions
int currentLine = 0; // Global variable to track the current instruction line
vector<int> breakpoints;
bool atBreak = false; // To check if to stop at breakpoint or start executing from it
vector<string> dataValues; // Values in .data section
int extraLines = 0;

// Function to split data values
vector<string> splitValues(const string &values)
{
    vector<string> result;
    string currentValue;
    // Loop through the characters in the string
    for (char c : values)
    {
        // If we encounter a comma or space, add the current value to the result
        if (c == ',' || c == ' ')
        {
            if (!currentValue.empty())
            {
                result.push_back(currentValue);
                currentValue = ""; // Reset currentValue for the next entry
            }
        }
        else
        {
            // Add the character to the current value
            currentValue += c;
        }
    }

    // Add the last value if there's any
    if (currentValue.length() > 0)
    {
        result.push_back(currentValue);
    }

    return result;
}

// Function to run instructions continuosly
void executeInstruction(string filename)
{
    for (int i = currentLine; i < instructionList.size(); i++)
    {
        int j = i;

        if (find(breakpoints.begin(), breakpoints.end(), j) != breakpoints.end())
        {
            cout << "Execution stopped at breakpoint" << endl;
            currentLine = i;
            atBreak = true;
            return;
        }
        handleStack(labelAddresses, i + 1);
        // Function present in simulator.cpp to run the instruction
        runInstruction(instructionList[j], j, labelAddresses);

        // Convert PC to hexadecimal
        string PCHex = decimalToHex((ll)4 * i, 8);
        for (int i = 0; i < 8; i++)
        {
            if (isalpha(PCHex[i]))
                PCHex[i] = tolower(PCHex[i]);
        }
        cout << "Executed " << instructionList[i] << "; PC=0x" << PCHex << endl;

        // Update the currentLine if it was changed by a branch/jump instruction
        i = j;
    }
    currentLine = instructionList.size();
    deleteStack();
}

// Function to run single instruction at a time
void stepInstruction()
{
    if (currentLine < instructionList.size())
    {
        // Execute only one instruction
        int j = currentLine;
        if (find(breakpoints.begin(), breakpoints.end(), j) != breakpoints.end())
        {
            // Case 1 when execution resumes from breakpoint
            if (atBreak)
            {
                atBreak = false;
            }
            // Case 2 when execution stops at breakpoint
            else
            {
                cout << "Execution stopped at breakpoint" << endl;
                atBreak = true;
                return;
            }
        }
        handleStack(labelAddresses, currentLine + 1);
        // Function present in simulator.cpp to run the instruction
        runInstruction(instructionList[j], j, labelAddresses);

        // Convert PC to hexadecimal
        string PCHex = decimalToHex((ll)4 * currentLine, 8);
        for (int i = 0; i < 8; i++)
        {
            if (isalpha(PCHex[i]))
                PCHex[i] = tolower(PCHex[i]);
        }

        cout << "Executed " << instructionList[currentLine] << "; PC=0x" << PCHex << endl;

        // Increment the current line
        currentLine = j + 1;
        if(currentLine==instructionList.size()) deleteStack();
    }
    else
    {
        cout << "Nothing to step" << endl;
    }
}

string currentDataType;

void handleDataSection(string line)
{
    // Check if we're currently handling a type
    if (currentDataType.empty())
    {
        // Case where data type and values are on the same line or type is encountered alone
        if (line.substr(0, 6) == ".dword")
        {
            currentDataType = ".dword";
            if (line.size() > 6)
            {
                string values = line.substr(7);
                if (!values.empty())
                {
                    dataValues = splitValues(values);
                    setDoubleword(dataValues);
                    dataValues.clear();
                    currentDataType.clear(); // Reset after processing
                }
            }
        }
        else if (line.substr(0, 5) == ".half")
        {
            currentDataType = ".half";
            if (line.size() > 5)
            {
                string values = line.substr(6);
                if (!values.empty())
                {
                    dataValues = splitValues(values);
                    setHalfword(dataValues);
                    dataValues.clear();
                    currentDataType.clear(); // Reset after processing
                }
            }
        }
        else if (line.substr(0, 5) == ".word")
        {
            currentDataType = ".word";
            if (line.size() > 5)
            {
                string values = line.substr(6);
                if (!values.empty())
                {
                    dataValues = splitValues(values);
                    setWord(dataValues);
                    dataValues.clear();
                    currentDataType.clear(); // Reset after processing
                }
            }
        }
        else if (line.substr(0, 5) == ".byte")
        {
            currentDataType = ".byte";
            if (line.size() > 5)
            {
                string values = line.substr(6);
                if (!values.empty())
                {
                    dataValues = splitValues(values);
                    setByte(dataValues);
                    dataValues.clear();
                    currentDataType.clear(); // Reset after processing
                }
            }
        }
    }
    else
    {
        // Case where values are on the next line
        dataValues = splitValues(line);

        if (currentDataType == ".dword")
        {
            setDoubleword(dataValues);
        }
        else if (currentDataType == ".half")
        {
            setHalfword(dataValues);
        }
        else if (currentDataType == ".word")
        {
            setWord(dataValues);
        }
        else if (currentDataType == ".byte")
        {
            setByte(dataValues);
        }

        dataValues.clear();
        currentDataType.clear(); // Reset after processing
    }
}

// Function to load file
void loadFile(string filename)
{
    ifstream inputFile(filename);
    if (!inputFile.is_open())
    {
        cerr << "Error opening input file." << endl;
        return;
    }

    string line;
    bool inTextSection = true; // Assume starting with text section
    int lineNumber = 0;

    while (getline(inputFile, line))
    {
        // Trim leading and trailing spaces
        while (!line.empty() && isspace(line[0]))
            line = line.substr(1); // Remove leading spaces
        while (!line.empty() && isspace(line.back()))
            line.pop_back(); // Remove trailing spaces

        // If the line is empty or a comment, skip it
        if (line.empty() || line[0] == ';')
        {
            continue;
        }

        // Check for .data or .text sections
        if (line == ".data")
        {
            extraLines++;
            inTextSection = false;
            continue;
        }
        if (line == ".text")
        {
            extraLines++;
            inTextSection = true;
            continue;
        }

        // Handle data section
        if (!inTextSection)
        {
            extraLines++;
            handleDataSection(line);
        }
        // Handle text section
        else
        {
            // Look for labels in the line (format: label:)
            for (size_t i = 1; i < line.length(); i++)
            {
                if (line[i] == ':')
                {
                    string label = line.substr(0, i); // Get the label name
                    if (labelAddresses.find(label) != labelAddresses.end())
                    {
                        cerr << "Error at line " << lineNumber + 1 << ". Label " << label
                             << " already exists at line " << labelAddresses[label] + 1 << endl;
                        return;
                    }
                    // Add the label to the map with the line number
                    labelAddresses[label] = lineNumber;
                    line = line.substr(i + 2); // Remove label from the line
                    break;
                }
            }
            // Trim leading and trailing spaces
            while (!line.empty() && isspace(line[0]))
                line = line.substr(1); // Remove leading spaces
            // If the line still has content after removing the label, treats it as an instruction
            if (!line.empty())
            {
                instructionList.push_back(line);
                lineNumber++;
            }
        }
    }
    createStack(labelAddresses);
    inputFile.close();
}

int main()
{
    string currentCommand;
    string filename;
    bool loaded = false;

    while (true)
    {
        getline(cin, currentCommand);

        if (currentCommand.substr(0, 5) == "load ")
        {
            // Handle load command
            // Resets all the memory and variable values if there was a file loaded previously
            if (loaded)
            {
                instructionList.clear();
                labelAddresses.clear();
                currentLine = 0;
                breakpoints.clear();
                atBreak = false;
                resetRegisters();
                resetMemory();
                deleteStack();
            }
            string filename = currentCommand.substr(5);
            
            loadFile(filename);
            loaded = true;
        }
        else if (currentCommand == "run")
        {
            if (!loaded)
            {
                cerr << "Error: No file loaded. Please use the load command first." << endl;
                continue;
            }

            executeInstruction(filename); // Executes instructions
            cout << endl;
        }
        else if (currentCommand == "step")
        {
            if (!loaded)
            {
                cerr << "Error: No file loaded. Please use the load command first." << endl;
                continue;
            }

            stepInstruction(); // Execute one instruction at a time
            cout << endl;
        }
        else if (currentCommand.substr(0, 6) == "break ")
        {
            // Maximum 5 breakpoints allowed
            if (breakpoints.size() == 5)
                cerr << "Breakpoints limit exceeded.";
            int breakpoint = stoi(currentCommand.substr(6));
            int newBreakpoint = breakpoint - extraLines -1;
            if(newBreakpoint<0){
                cerr << "Please give valid breakpoint." << endl;
                return 1;
            }
            breakpoints.push_back(newBreakpoint);
            cout << "Breakpoint set at line " << breakpoint << endl;
            cout << endl;
        }
        else if (currentCommand.substr(0, 10) == "del break ")
        {
            int breakpoint = stoi(currentCommand.substr(10));
            auto it = find(breakpoints.begin(), breakpoints.end(), breakpoint - 1);
            if (it != breakpoints.end())
            {
                breakpoints.erase(it); // If present deletes the breakpoint
            }
            else
            {
                cerr << "No breakpoint present at line " << breakpoint << endl;
            }
            cout << endl;
        }
        else if (currentCommand == "regs")
        {
            printRegisters(); // Function in simulator.cpp
            cout << endl;
        }
        else if (currentCommand.substr(0, 4) == "mem ")
        {
            // Extracting address and count from the line
            string address = currentCommand.substr(4, 7);
            int count = stoi(currentCommand.substr(11));
            printMemory(address, count); // Function in simulator.cpp
        }
        else if (currentCommand == "show-stack")
        {
            showStack(extraLines); // Function in simulator.cpp
        }
        else if (currentCommand == "exit")
        {
            cout << "Exited the simulator" << endl;
            break;
        }
        else
        {
            cerr << "Unknown command." << endl;
        }
    }

    return 0;
}
