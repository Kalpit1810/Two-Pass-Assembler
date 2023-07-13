/*****************************************************************************
TITLE: Assembler C++ Program
AUTHOR:   	Kalpit Agrawal
ROLL NUMBER: 	2101CS34
Declaration of Authorship
This txt file, claims.txt, is part of the miniproject of CS209/CS210 at the
department of Computer Science and Engg, IIT Patna .
*****************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cctype>
#include <array>
#include <vector>
#include <utility>
#include <tuple>
#include <bitset>
using namespace std;

// Mnemonic Struct
// opr_type = 0 : No Operand
// opr_type = 1 : Value Operand
// opr_type = 2 : 'data'
// opr_type = 3 : 'set'
// opr_type = 4 : Offset Operand
struct Mnemonic
{
    string mnemonic;
    string opcode;
    int opr_type;
};

Mnemonic all_mnemonics[] = {
    {"ldc", "00000000", 1},
    {"adc", "00000001", 1},
    {"ldl", "00000010", 4},
    {"stl", "00000011", 4},
    {"ldnl", "00000100", 4},
    {"stnl", "00000101", 4},
    {"add", "00000110", 0},
    {"sub", "00000111", 0},
    {"shl", "00001000", 0},
    {"shr", "00001001", 0},
    {"adj", "00001010", 1},
    {"a2sp", "00001011", 0},
    {"sp2a", "00001100", 0},
    {"call", "00001101", 4},
    {"return", "00001110", 0},
    {"brz", "00001111", 4},
    {"brlz", "00010000", 4},
    {"br", "00010001", 4},
    {"HALT", "00010010", 0},
    {"data", "", 2},
    {"SET", "", 3}};

array<string, 10> errors = {
    "No Error in Pass One",
    "Duplicate Label",
    "Invalid Label",
    "Invalid Mnemonic",
    "Missing Operand",
    "Invalid Operand",
    "Unexpected Operand",
    "Extra on End of Line",
    "Label Not Declared",
    "Undefined Error"};

vector<pair<string, long>> symbolTable; // label, PC
vector<pair<long, long>> literalTable;  // PC, constant

// checks if the string is valid mnemonic
// opr_type = 0 : No Operand
// opr_type = 1 : Value Operand
// opr_type = 2 : 'data'
// opr_type = 3 : 'set'
// opr_type = 4 : Offset Operand
int isvalidMnemonic(string word)
{
    for (int i = 0; i < 21; i++)
    {
        if (all_mnemonics[i].mnemonic == word)
            return 1;
    }
    return 0;
}

// returns the operand type required for the mnemonic
int operand_required(string str)
{
    for (int i = 0; i < 21; i++)
    {
        if (all_mnemonics[i].mnemonic == str)
        {
            return all_mnemonics[i].opr_type;
        }
    }
}

// Gets the opcode of the mnemonic;
string get_opcode(string str)
{
    for (int i = 0; i < 21; i++)
    {
        if (all_mnemonics[i].mnemonic == str)
            return all_mnemonics[i].opcode;
    }
}

// True if the given lable is currently not in Symbol Table
bool newSymbol(string label)
{
    for (int i = 0; i < symbolTable.size(); i++)
    {
        if (label == symbolTable[i].first)
            return false;
    }
    return true;
}

// Returns the PC of the label. if label PC not avialable then returns -1.
int getSymbolPC(string label)
{
    for (int i = 0; i < symbolTable.size(); i++)
    {
        if (label == symbolTable[i].first)
            return symbolTable[i].second;
    }
}

// Updates the PC of given label. Used in case of SET mnemonic.
void updateSymbolsPC(string label, int PC)
{
    for (int i = 0; i < symbolTable.size(); i++)
    {
        if (label == symbolTable[i].first)
        {
            symbolTable[i].second = PC;
        }
    }
}

// Writes the error in log file
int log_error(int err_code, int line_no, ofstream &w_log)
{

    if (err_code < 9 && err_code)
        w_log << "Error: " << line_no << ": " << errors.at(err_code) << "\n";
    else if (err_code)
        w_log << "Error: " << errors.at(err_code);
    else
        w_log << errors.at(err_code) << "\n";

    return err_code;
}

// Scan a word from the string
string scanWord(string str)
{
    // word variable to store word
    string word;

    // making a string stream
    stringstream iss(str);

    // Retrurn the word
    iss >> word;
    return word;
}

// removes comments, leading/trailling whitespaces
void Optimize_line(string &line)

{
    // truncates line after';' (including ';')
    line = line.substr(0, line.find(';'));

    // remove the trailing white spaces
    while (line[line.size() - 1] == ' ')
    {
        line.pop_back();
    }

    // remove leading whitespaces
    while (line[0] == ' ')
    {
        line.erase(line.begin());
    }
}

// checks if the line is lable Declared
bool label_Declared(string line)
{
    if (line.find(':') < line.size())
        return true;

    return false;
}

// checks if the label format is satiesfied
bool valid_Label_Format(string line)
{
    string lable = line.substr(0, line.find(':'));
    if (lable.size() == 0 || !isalpha(lable[0]))
    {
        return false;
    }
    for (int i = 0; i < lable.size(); i++)
    {
        if (!iswalnum(lable[i]))
        {
            return false;
        }
    }

    return true;
}

// checks if the number is in decimal/hex/octal
bool valid_Number_Format(string opr)
{
    char *end = NULL;
    int num = strtol(opr.c_str(), &end, 0);
    if (*end != 0)
    {
        return false;
    }
    else
        return true;
}

// converts operand into 32-bit
string operand_32bit(string opr)
{
    // if operand is a number then convetr it to 32-bit, else if it is a label get its PC and convert it into 32-bit
    if (valid_Number_Format(opr))
    {
        // convert opr to decimal(if it is in any special type) then change it to binary 32 bit.
        long t = strtoll(opr.c_str(), nullptr, 0);
        string temp3 = bitset<32>(t).to_string();
        return temp3;
    }
    else
    {
        int temp1 = getSymbolPC(opr);
        // if temp<0, then there is an unassigned label;
        if (temp1 < 0)
            return "";

        string temp2 = bitset<32>(temp1).to_string();
        return temp2;
    }
}

// converts operand into 24-bit
string operand_24bit(string mnemonic, string opr, int PC)
{
    long temp1;
    if (valid_Label_Format(opr))
    {
        temp1 = getSymbolPC(opr);

        // if temp1<0 then label is not declared
        if (temp1 < 0)
            return "";
    }
    else
    {
        temp1 = strtoll(opr.c_str(), nullptr, 0);
    }

    if (operand_required(mnemonic) == 4)
    {
        temp1 -= (PC + 1);
    }

    string s = bitset<24>(temp1).to_string();
    return s;
}

// converts the operand into binary, Adjusts for PC value offset
string convertOperandToBinary(string mnemonic, string opr, int PC)
{
    if (!operand_required(mnemonic))
        return NULL;
    else
    {
        int opr_type = operand_required(mnemonic);

        // if mnemonic is data, then convert operand into 32-bit type. Else convert opr to 24-bit type
        if (opr_type == 3)
        {
            return operand_32bit(opr);
        }
        else
            return operand_24bit(mnemonic, opr, PC);
    }
}

// stores the listing and log file temporarily
int Second_Pass(FILE *rasm, FILE *lst, FILE *obj, ofstream &w_log, ofstream &w_lst, ofstream &w_obj)
{
    long PC = 0, line_no = 0, error = 0;

    char buffer[512];
    rewind(rasm);
    while (fgets(buffer, 512, rasm))
    {

        stringstream ss(buffer);

        string tempo, w1, w2, w3, label = "", mnemonic = "", opr = "";
        ss >> tempo;
        ss >> w1;
        ss >> w2;
        ss >> w3;

        line_no = stol(tempo);

        // check if w1 is label.
        // If yes then assign words accordingly.
        // If no then w1 must be a mnemonic.

        if (label_Declared(w1))
        {
            w1.pop_back(); // remove ":"
            label = w1;
            mnemonic = w2;
            opr = w3;
        }
        else
        {
            mnemonic = w1;
            opr = w2;
        }

        // Now generate machine code and listing file.

        if (!label.empty())
        {
            string temp1 = label + ":";
            long t = getSymbolPC(label);
            if (t < 0)
            {
                error = log_error(8, line_no, w_log);
                continue;
            }
            fprintf(lst, "%08x\t\t\t\t", t);
            fputs(temp1.c_str(), lst);
        }

        if (mnemonic == "SET")
        {
            fputs("\n", lst);
            PC++;
            continue;
        }
        else if (!mnemonic.empty())
        {
            if (!label.empty())
            {
                fputs("\n", lst);
            }

            string opcode = get_opcode(mnemonic);

            string opr_binary;
            if (!opr.empty())
                opr_binary = convertOperandToBinary(mnemonic, opr, PC);
            else
                opr_binary = "000000000000000000000000";

            // if operand is required and opr_binary is NULL, then raise error
            if (operand_required(mnemonic) && opr_binary.empty())
            {
                error = log_error(8, line_no, w_log);
                continue;
            }

            // add in lst file PC,instruction code , mnemonic, operand.

            string machine_code = opr_binary + opcode;
            if (!opcode.empty())
                fputs((machine_code + "\n").c_str(), obj);
            else
                fputs((machine_code + "00000000\n").c_str(), obj);

            long long t = strtoll(machine_code.c_str(), nullptr, 2);
            stringstream inst_code;
            inst_code << hex << t;

            fprintf(lst, "%08x\t", PC);

            string s;
            inst_code >> s;
            while (s.size() < 8)
            {
                s = "0" + s;
            }

            fputs(("0x" + s).c_str(), lst);
            fprintf(lst, "\t");

            fputs(mnemonic.c_str(), lst);
            fputs("\t", lst);

            if (valid_Label_Format(opr))
            {
                fputs(opr.c_str(), lst);
            }
            else
            {
                // opr is a number convert it into hex and add it to lst file
                long t = strtoll(opr_binary.c_str(), nullptr, 2);

                stringstream op_hex;
                op_hex << hex << t;
                fputs(("0x" + op_hex.str()).c_str(), lst);
            }
        }
        else
            PC--;
        fprintf(lst, "\n");
        PC++;
    }

    if (error)
        return 1;
    else
        return 0;
}

// checks for syntax errors and creates the symbol, lable, literal table
int First_pass(ifstream &r_asm, ofstream &w_log, FILE *rasm)
{
    int PC = -1, error = 0, line_no = 0;
    string line;

    // take input as long as lines are avialable
    while (getline(r_asm, line))
    {
        line_no += 1; // current line no.

        // optimize the instruction line
        Optimize_line(line);
        if (line.empty())
        {
            continue;
        }

        PC += 1; // current PC value

        // Check if instruction has a lable Declaration.
        // If yes, check if it is valid. If not, raise invalid label error.
        // If valid, check if it is a valid mnemonic.
        // if NOT a valid mnemonic, then check if the label already exists in the symbol table. If not then insert it.
        // else if exists and PC value is < 0 then update the PC value else raise duplicate label error.
        string label = "";
        if (label_Declared(line))
        {
            if (valid_Label_Format(line))
            {
                label = line.substr(0, line.find(':'));
                if (isvalidMnemonic(label))
                {
                    error = log_error(2, line_no, w_log);
                    continue;
                }

                if (newSymbol(label))
                {
                    symbolTable.push_back(make_pair(label, PC));
                }
                else
                {
                    if (getSymbolPC(label) < 0)
                    {
                        updateSymbolsPC(label, PC);
                    }
                    else
                    {
                        error = log_error(1, line_no, w_log);
                    }
                }
            }
            else
            {
                error = log_error(2, line_no, w_log);
            }

            // Remove the lable declaration part from line. Remove any white space.
            line = line.substr(line.find(":") + 1);
            Optimize_line(line);
        }

        // Extract the next word and optimize the line.
        // If word is empty then move to next line.
        // If there is a word then check if it is a valid mnemonic, if not raise invalid mnemonic error and move to next line.
        string mnemonic = scanWord(line);
        line.erase(0, mnemonic.size());
        Optimize_line(line);

        if (mnemonic.empty())
        {
            PC--;
            fputs((to_string(line_no) + " " + label + ":\n").c_str(), rasm);
            continue;
        }
        if (!isvalidMnemonic(mnemonic))
        {
            error = log_error(3, line_no, w_log);
            continue;
        }

        // Extract the operand and optimize the line.
        string opr = scanWord(line);
        line.erase(0, opr.size());
        Optimize_line(line);

        // if operand is required and operand is empty or of wrong format, then raise error
        // if valid operand is label and if not already pressent, then insert label into symbolTable
        // if valid operand is number then if mnemonic "SET" change the PC value of the lable in symbolTable,
        // else if mnemonic is "data" then add it to memory.
        // otherwise enter the operands numerical value in literal table.
        if (operand_required(mnemonic))
        {
            if (opr.empty())
            {
                error = log_error(4, line_no, w_log);
                continue;
            }
            else if (!valid_Label_Format(opr) && !valid_Number_Format(opr))
            {
                error = log_error(5, line_no, w_log);
                continue;
            }

            if (valid_Label_Format(opr))
            {
                int f = 1;
                for (int i = 0; i < symbolTable.size(); i++)
                {
                    if (opr == symbolTable[i].first)
                    {
                        f = 0;
                        break;
                    }
                }

                if (f)
                {
                    symbolTable.push_back(make_pair(opr, -1));
                }
            }
            else if (valid_Number_Format(opr))
            {
                if (mnemonic == "SET")
                {
                    int f=1;
                    long t = strtoll(opr.c_str(), nullptr, 0);

                    for (int i = 0; i < symbolTable.size(); i++)
                    {
                        if (label == symbolTable[i].first)
                        {
                            f=0;
                            symbolTable[i].second = t;
                        }
                    }

                    if (f)
                    {
                        symbolTable.push_back(make_pair(label,t));
                    }
                }
                else if (mnemonic == "data")
                {
                    long t = strtoll(opr.c_str(), nullptr, 0);

                    for (int i = 0; i < symbolTable.size(); i++)
                    {
                        if (label == symbolTable[i].first)
                        {
                            if (symbolTable[i].second < 0)
                            {
                                symbolTable[i].second = PC;
                                literalTable.push_back(make_pair(PC, t));
                            }
                        }
                    }
                }
                else
                {
                    long t = strtoll(opr.c_str(), nullptr, 0);
                    literalTable.push_back(make_pair(PC, t));
                }
            }
        }
        else if (!operand_required(mnemonic) && !opr.empty())
        {
            error = log_error(6, line_no, w_log);
            continue;
        }

        // After extracting operand if there is something left in line then raise error
        if (!line.empty())
        {
            error = log_error(7, line_no, w_log);
            continue;
        }

        if (!error)
        {
            if (!label.empty())
                fputs((to_string(line_no) + " " + label + ": " + mnemonic + " " + opr + "\n").c_str(), rasm);
            else
                fputs((to_string(line_no) + " " + mnemonic + " " + opr + "\n").c_str(), rasm);
        }
    }

    if (error)
        return 1;

    error = log_error(0, line_no, w_log);
    return 0;
}

int main(int argc, char *argv[])
{
    string filename = argv[1];
    int pos = filename.find('.');
    filename = filename.substr(0, pos);

    ifstream r_asm(argv[1]);
    ofstream w_log((filename + ".log").c_str());

    FILE *rasm = tmpfile();
    FILE *lst = tmpfile();
    FILE *obj = tmpfile();

    int first_status = First_pass(r_asm, w_log, rasm);
    if (first_status)
    {
        return 0;
    }

    ofstream w_obj((filename + ".o").c_str());
    ofstream w_lst((filename + ".lst").c_str());
    int sec_status = Second_Pass(rasm, lst, obj, w_log, w_lst, w_obj);
    if (sec_status)
    {
        return 0;
    }

    char buffer[512];
    rewind(obj);
    while (fgets(buffer, 512, obj))
    {
        w_obj << buffer;
    }
    rewind(lst);
    while (fgets(buffer, 512, lst))
    {
        w_lst << buffer;
    }
    w_log << "No Error in Pass Two";
    return 0;
}