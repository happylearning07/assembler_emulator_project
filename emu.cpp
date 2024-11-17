/*--------------------------------------------------------------------------

TITLE    : Souce code for emulator
NAME     : JUHI SAHNI
Roll No. : 2301CS88
Declaration of Authorship
This C++ file, emu.cpp, is part of the miniproject of CS2102 at the
Department of Computer Science and Engineering, IIT Patna.

----------------------------------------------------------------------------*/
#include <bits/stdc++.h>
using namespace std;
#define ui unsigned int

// memory
const int n = 1 << 24;
vector<ui> mem(n, 0);

// registers
int SP = 0;
int A = 0;
int B = 0;
int PC = 0;

// Function to execute instructions based on mnemonic codes
void func(int m_code, int oprd) {
    switch (m_code) {
        case 0: // ldc
            B = A;
            A = oprd;
            break;
        case 1: // adc
            A += oprd;
            break;
        case 2: // ldl
            B = A;
            A = mem[SP + oprd];
            break;
        case 3: // stl
            mem[SP + oprd] = A;
            A = B;
            break;
        case 4: // ldnl
            A = mem[A + oprd];
            break;
        case 5: // stnl
            mem[A + oprd] = B;
            break;
        case 6: // add
            A += B;
            break;
        case 7: // sub
            A = B - A;
            break;
        case 8: // shl
            A = B << A;
            break;
        case 9: // shr
            A = B >> A;
            break;
        case 10: // adj
            SP += oprd;
            break;
        case 11: // a2sp
            SP = A;
            A = B;
            break;
        case 12: // sp2a
            B = A;
            A = SP;
            break;
        case 13: // call
            B = A;
            A = PC;
            PC += oprd;
            break;
        case 14: // return
            PC = A;
            A = B;
            break;
        case 15: // brz
            if (A == 0) {
                PC += oprd;
            }
            break;
        case 16: // brlz
            if (A < 0) {
                PC += oprd;
            }
            break;
        case 17: // br
            PC += oprd;
            break;
        default:
            // Handle invalid mnemonic (if needed)
            break;
    }
}

ui readFileIntoMemory(fstream &ipt_f);
string convertUnsignedIntToHex(ui dec);
ui countLinesInFile(fstream &fl);
ui readIntegerFromFile(fstream &fl);

// Function to read integers from a .o file into memory
ui readFileIntoMemory(std::fstream &inputFile) {
    ui lineCount = countLinesInFile(inputFile); // Get the number of lines
    ui value; // Variable to hold each integer read

    // Read integers from the file into memory
    for (unsigned int index = 0; index < lineCount; ++index) {
        value = readIntegerFromFile(inputFile); // Read the integer
        mem[index] = value; // Store it in memory
    }

    return lineCount; // Return the total number of lines read
}

string convertUnsignedIntToHex(ui decimalNumber) {
    ostringstream hexOutput; // Create a string output stream
    // Format the output as a zero-padded hexadecimal string
    hexOutput << std::setfill('0') << std::setw(8) << std::hex << decimalNumber;
    return hexOutput.str(); // Return the formatted string
}

// Function to get the count of lines in a .o file
unsigned int countLinesInFile(fstream &fileStream) {
    // Move the cursor to the end of the file
    fileStream.seekg(0, ios::end);
    
    // Get the size of the file
    long fileSize = fileStream.tellg();
    
    // Reset the cursor to the beginning of the file
    fileStream.seekg(0, ios::beg);

    // Check for a valid file size and calculate the line count
    if (fileSize < 0) {
        cerr << "Error: Failed to determine file size.\n";
        return 0; // Handle error appropriately, possibly returning a sentinel value
    }
    
    // Each line is assumed to represent 4 bytes
    return static_cast<unsigned int>(fileSize / 4);
}
ui readIntegerFromFile(fstream &fileStream) {
    ui number = 0;
    
    // Attempt to read the integer from the file
    fileStream.read(reinterpret_cast<char*>(&number), sizeof(number));
    
    // Check for read errors
    if (!fileStream) {
        cerr << "Error: Failed to read integer from file.\n";
        // Handle the error appropriately (e.g., throw an exception or return a sentinel value)
    }
    
    return number;
}
void dumpMemoryToFile(ofstream &fileStream, int lineCount) {
    // Use a string stream to build the output before writing to the file
    ostringstream output;

    // Iterate through memory and format the dump
    for (int index = 0; index < lineCount; ++index) {
        // Print the address every 4 entries
        if (index % 4 == 0) {
            output << "\n0x" << convertUnsignedIntToHex(index);
        }
        output << "\t0x" << convertUnsignedIntToHex(mem[index]);
    }

    // Write the final output to the file
    fileStream << output.str() << '\n';
}

// Function for printing the memory dump
void printMemoryDump(ui lineCount, bool isBefore) {
    // Output message indicating whether it's before or after execution
    cout << (isBefore ? "Showing memory dump before execution\n" : "Showing memory dump after execution\n");

    ostringstream output;
    
    // Iterate through the memory and format the dump
    for (ui index = 0; index < lineCount; ++index) {
        // Print the address every 4 entries
        if (index % 4 == 0) {
            output << "\n0x" << convertUnsignedIntToHex(index);
        }
        output << "\t0x" << convertUnsignedIntToHex(mem[index]);
    }
    
    // Print the final output
    cout << output.str() << '\n';
}

// Function to print the Instruction Set Architecture (ISA)
void printISA() {
    // Use a map to associate opcodes with mnemonics and operands
    map<int, pair<string, string>> instructionSet = {
        {0, {"ldc", "val"}},
        {1, {"adc", "val"}},
        {2, {"ldl", "off"}},
        {3, {"stl", "off"}},
        {4, {"ldnl", "off"}},
        {5, {"stnl", "off"}},
        {6, {"add", ""}},
        {7, {"sub", ""}},
        {8, {"shl", ""}},
        {9, {"shr", ""}},
        {10, {"adj", "val"}},
        {11, {"a2sp", ""}},
        {12, {"sp2a", ""}},
        {13, {"call", "off"}},
        {14, {"return", ""}},
        {15, {"brz", "off"}},
        {16, {"brlz", "off"}},
        {17, {"br", "off"}},
        {18, {"HALT", ""}}
    };
    // Print the header
    cout << "Opcode\tMnemonic\tOperand\n";
    // Iterate through the instruction set and print each entry
    for (const auto& instruction : instructionSet) {
        cout << instruction.first << "\t" 
             << instruction.second.first << "\t\t" 
             << instruction.second.second << '\n';
    }
}

int executeInstructions(bool f) {
    // initialising stack pointer
    SP = 8388607;

    ui oprd_l = 0;
    ui instr_num = 0;
    ui mem_cd = 0;
    ui oprd = 0;

    for (; mem_cd <= 18;) {
        // getting last 8 bits of mem_code
        int m = 1 << 8;
        mem_cd = mem[PC] % m;
        if (mem_cd == 18) {
            break;
        } else if (mem_cd > 18)
            return 1;

        // access operand
        oprd_l = mem[PC] >> 8;
        if (oprd_l > 8388607) {
            oprd = (int)oprd_l - n;
        } else {
            oprd = (int)oprd_l;
        }

        // implementing trace printing according to bool f
        if (f) {
            printf("PC = 0x");
            cout << convertUnsignedIntToHex(PC);
            printf("\tA = 0x");
            cout << convertUnsignedIntToHex(A);
            printf("\tB = 0x");
            cout << convertUnsignedIntToHex(B);
            printf("\tSP = 0x");
            cout << convertUnsignedIntToHex(SP);
            cout << '\n';
        }

        func(mem_cd, oprd);
        PC++;
        instr_num++;
    }
    cout << '\n';
    printf("%d", instr_num);
    printf(" instruction executed.\n");
    return 0;
}

// New function to print the sorted array
void printSortedArray(ui startAddress, ui length) {
    cout << "Sorted Array: ";
    for (ui i = 0; i < length; ++i) {
        cout << mem[startAddress + i] << " ";
    }
    cout << endl;
}

int main() {
    cout << "Enter the filename with .o extension: ";
    string filename;
    cin >> filename;

    cout << "Choose an option:\n";
    cout << "trace : Display instruction trace.\n";
    cout << "after : Show memory dump after execution.\n";
    cout << "before : Show memory dump before execution.\n";
    cout << "isa : Display Instruction Set Architecture.\n";

    cout << "Enter option: ";
    string user_option;
    cin >> user_option;

    int trace = 0, before_dump = 0, after_dump = 0, isa_display = 0;
    int line_count;

    if (user_option == "isa") {
        isa_display = 1;
    } else if (user_option == "trace") {
        trace = 1;
    } else if (user_option == "before") {
        before_dump = 1;
    } else if (user_option == "after") {
        after_dump = 1;
    } else {
        cout << "Invalid option provided.\n";
        return 0;
    }

    fstream input_file(filename, ios::binary | ios::in);
    bool check_open = input_file.is_open();
    if (check_open) {
        line_count = readFileIntoMemory(input_file);
        input_file.close();
    } else {
        cout << "ERROR: File could not be opened or is empty.\n";
        return 0;
    }
    bool check_before_dump = before_dump;
    if (before_dump) {
        printMemoryDump(line_count, true);
    } else if (isa_display) {
        printISA();
    }

    int execution_error = executeInstructions(trace);
    if (execution_error == 1) {
        cout << "ERROR: Invalid mnemonic encountered.\n";
        return 0;
    }

    // Print the sorted output
    // ui startAddress = 0x0000004c; // Address of the first element of the sorted array
    // ui arrayLength = 11; // Number of elements in the array
    // printSortedArray(startAddress, arrayLength);

    if (after_dump) {
        printMemoryDump(line_count, false);
    }

    ofstream memory_dump_file;
    string memory_dump_filename = filename + "_memdump.txt";
    memory_dump_file.open(memory_dump_filename, ios::out);

    dumpMemoryToFile(memory_dump_file, line_count);
    memory_dump_file.close();

    return 0;
}