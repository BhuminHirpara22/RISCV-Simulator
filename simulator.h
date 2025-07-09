#include <string>

using namespace std;
typedef long long ll;

void runInstruction(const string &instruction, int &lineNumber, const unordered_map<string, int> &labelAddresses);
void printRegisters();
void printMemory(string address, int count);
string binaryToHex(string &binaryInstruction);
void resetRegisters();
void resetMemory();
void setDoubleword(vector<string> dataValues);
void setHalfword(vector<string> dataValues);
void setWord(vector<string> dataValues);
void setByte(vector<string> dataValues);
void showStack(int extraLines);
string decimalToHex(ll number, int hexDigits);
void createStack(unordered_map<string,int> &labelAddresses);
void handleStack(unordered_map<string,int> &labelAddresses,int lineNumber);
void deleteStack();