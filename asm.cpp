/*--------------------------------------------------------------------------

TITLE    : Souce code for assembler
NAME     : JUHI SAHNI
Roll No. : 2301CS88
Declaration of Authorship
This C++ file, asm.cpp, is part of the miniproject of CS2102 at the
Department of Computer Science and Engineering, IIT Patna.

----------------------------------------------------------------------------*/

#include <bits/stdc++.h>
#include <string>
using namespace std;

// -------------------------VARIABLES AND DATA STRUCTURES---------------------------------------//
string Filename;
map <string,char> Bin_Hex_Conv;
map <string,int> noOperandInstr,oneOperandInstr,offsetInstr;
map <string,int> labels;
set < string > used_labels;
set < pair<int,string> > Errors,Warnings;
vector <string> source_code[100000];
vector <string> machine_code(100000);
int machine_code_int[100000];

//---------------------------FUNCTIONS------------------------------------------------------//
void initialize();
void display_errors();
void write_obj_file();
void FirstPass(string s,int line_num,int* p);
void SecondPass();
string decimal_to_2scomplement(int data,int no_of_bits);
int string_to_decimal(string s,int no_of_bits);
bool is_valid_label(string s);
void Set_labels();
void Extra_labels();
int is_valid_operand(string s);
void display_warnings();
void write_log_file();
void calculate_machine_code(int instruction_type,string mnemonic,string operand,int itr,string label_to_be_used);
void write_list_file();


int main(int argc,char** argv)
{
	// Step 1-->initialising
	initialize();

	// Step 2-->open the source file for reading
	ifstream file_pointer(argv[1]);
    if (!file_pointer.is_open()) {
        Errors.insert({0, "Format of the file provided is wrong"});
        write_log_file();
        return 1; // Exit with an error code
    }
	Filename = "";
    // -----------------------file is opened successfully-------------------------
	// Step 3-->Store filename before extension which can be used later
	int index=0;
    while(index<strlen(argv[1])){
        bool check_it=argv[1][index]=='.';
        if(check_it) break;    //extracting the file name before extension
        Filename+=argv[1][index];
        index++;
    }

	// Step 4-->Perform first and second pass
    string presentLine;
	int line_num = 1;
	int cnt = 0;    //performs similar to a counter
		
	// Reading the file line by line
	while(getline(file_pointer,presentLine))//reads line by line of file in a string
	{
			string each_word;
            int i=0;
            while(i<presentLine.length()){
                if(presentLine[i]==';') break;
                each_word+=presentLine[i];
                i++;
            }                              //e.g.=>presentLine="Hello;World" :: each_word="Hello"

			// performing first pass
			FirstPass(each_word,line_num,&cnt); //file pointer we are incrementing inside firstPass function...changes will also be reflected here
			line_num++; //moving to next line and hence keeping a track of number of lines
	}

	Set_labels();   //taking those lines in consideration which use 'SET'

	// performing second pass and converting assembly code to machine code 
	SecondPass();
	Extra_labels();
	
    // Generate listing file and write logs
    write_list_file();
    write_log_file();

	// Check if given assembly has errors or not 
	if(Errors.empty())
	{		
		// Print warnings, if present
		if(!Warnings.empty())
			display_warnings();

		// generate object file	
		write_obj_file();
	}
	else
	{
		// print all errors along with existing warnings
		display_errors();
		if(!Warnings.empty())
			display_warnings();
	}
	// Close source file 
	file_pointer.close();
	return 0;
}

// Insert instructions along with their opcode and also categorize instructions according to their format
void initialize()
{   
    noOperandInstr={{"add", 6},{"sub", 7},{"shl", 8},{"shr", 9},{"a2sp", 11},{"sp2a", 12},{"return", 14},{"HALT", 18}};
	
    oneOperandInstr={{"ldc", 0},{"adc", 1},{"ldl", 2},{"stl", 3},{"ldnl", 4},{"stnl", 5},{"adj", 10},{"call", 13},
                      {"brz", 15},{"brlz", 16},{"br", 17},{"SET",19},{"data",20}};
                
    offsetInstr={{"call",13},{"brz",15},{"brlz",16},{"br",17}};

    Bin_Hex_Conv={{"0000",'0'},{"0001",'1'},{"0010",'2'},{"0011",'3'},{"0100",'4'},{"0101",'5'},{"0110",'6'},{"0111",'7'},{"1000",'8'},{"1001",'9'},{"1010",'A'},{"1011",'B'},{"1100",'C'},{"1101",'D'},{"1110",'E'},{"1111",'F'}};
}

// A.--Check for valid labels
bool is_valid_label(string s)
{   
    bool chk1=s[0] >= '0' and s[0] <= '9';
	if(chk1) //it should not be starting with a number 
		return false;	

	for(int i=0;i<s.length();i++)
	{
        bool chk2=(s[i] >= 'a' and s[i] <= 'z');
        bool chk3=(s[i] >= 'A' and s[i] <= 'Z');
        bool chk4=(s[i] >= '0' and s[i] <= '9');
        bool chk5=(s[i] == '_');
		if( !((chk2) or (chk3) or (chk4) or (chk5)) )
			return false; //no extra special characters are allowed
	}
	return true;
}
//B.--Check for valid operands
int is_valid_operand(string s)
{
	if (is_valid_label(s)) {
        return labels.find(s) != labels.end() ? 5 : 1; // Return 5 if label exists, else 1
    }
    if (s.empty()) return 0; // Invalid if empty
    bool isOctal = true, isDecimal = true, isHexadecimal = true;

    for (char c : s) {
        if (c < '0' || c > '9') {
            isDecimal = false; // Not a decimal if it contains non-digit characters
            if (c >= 'a' && c <= 'f') {
                isHexadecimal = true; // Valid hex digit
            } else if (c >= 'A' && c <= 'F') {
                isHexadecimal = true; // Valid hex digit
            } else {
                isHexadecimal = false; // Not a valid hex digit
                if (c < '0' || c > '7') {
                    isOctal = false; // Not a valid octal digit
                }
            }
        }
    }
    if (isOctal) return 3;       // Valid octal
    if (isDecimal) return 2;     // Valid decimal
    if (isHexadecimal) return 4;  // Valid hexadecimal
	return 0;
}
void Check_instr1(vector<int>& len_words,vector<string>&words,int line_cnt){
     if (len_words[1] > 0 && words[1].back() != ':') { //something is present after the label and does not end with a colon
            string instruction = words[1];

            // Check instruction requirements
            if (noOperandInstr.find(instruction) != noOperandInstr.end()) {
                if (!words[2].empty()) {
                    Errors.insert({line_cnt, "Unexpected Operand, No Operand is allowed"});
                }
            } else if (oneOperandInstr.find(instruction) != oneOperandInstr.end()) {
                if (!words[3].empty()) {
                    Errors.insert({line_cnt, "Unexpected Operand, Only One Operand is allowed"});
                }
                if (words[2].empty()) {
                    Errors.insert({line_cnt, "No Operand found"});
                }
            } else {
                Errors.insert({line_cnt, "No such instruction"});
            }
        }
}

void Check_instr2(vector<string>& words,int line_cnt,int* ptr){
    // No Label: handle instruction directly
        string instruction = words[0];

        // Check instruction requirements
        if (noOperandInstr.find(instruction) != noOperandInstr.end()) {
            if (!words[1].empty()) {
                Errors.insert({line_cnt, "Unexpected Operand, No Operand is allowed"}); //extra operands case 
            }
        } 
        else if (oneOperandInstr.find(instruction) != oneOperandInstr.end()) {
            if (!words[2].empty()) {
                Errors.insert({line_cnt, "Unexpected Operand, Only One Operand is allowed"}); //extra operands case
            }
            if (words[1].empty()) {
                Errors.insert({line_cnt, "No Operand found"}); //0 operand case
            }
        } 
        else {
            Errors.insert({line_cnt, "No such instruction"}); //invalid instruction
        }
        (*ptr)++; //incrementing the pointer to source code
}
void FirstPass(string s, int line_cnt, int* ptr) {    //we passed address of cnt hence p is a pointer to src code
    int n=s.size();
    if(n==0) return;
    // Prepare modified string
    string modifiedString;
    for(auto character:s){
        modifiedString+=character;
        if (character==':') modifiedString+=' ';   //making label separate
    }

    // Separate line into words
    vector<string>words(5,"");  //  label + instruction + 3 operands
    vector<int> len_words(5,0);
    stringstream ss(modifiedString);
    int i = 0;
    //storing length of all words 
    while (ss >> words[i]) {
        len_words[i] = words[i].length();
        i++;
    }
    int j=0;
    while(j<5){
        if(!words[j].empty()){
            source_code[*ptr].push_back(words[j]);
        }
        j++;
    }

    //Check for label presence
    if (words[0].size()>0 && words[0].back() == ':') {
        string label_str = words[0].substr(0, len_words[0] - 1); //truncating : at the end
        //Validate Label name
        if(labels.find(label_str) != labels.end()){
            Errors.insert({line_cnt, "Duplicate Label"});
        }
        else if(!is_valid_label(label_str)){
            Errors.insert({line_cnt, "Invalid Label Name"});
        }
        else {
        // Insert valid label into labels table
            labels.insert({label_str, *ptr});  // storing label name along with the address of line in which it is present
            if (!(len_words[1] == 0 && len_words[2] == 0)) { //if there is either an instruction or operand
                (*ptr)++;     //incrementing the pointer
            }
        }

        // Check for instructions on the same line 
        Check_instr1(len_words,words,line_cnt);
    } 
    else {
        Check_instr2(words,line_cnt,ptr);
    }
}


string decimal_to_2scomplement(int data, int no_of_bits) {
    // Create a string to hold the binary representation
    string bin_str(no_of_bits, '0');

    // Fill the string with binary digits
    for (int i = 0; i < no_of_bits; ++i) {
        int mask = 1 << (no_of_bits - 1 - i);
        bin_str[i] = (data & mask) ? '1' : '0';
    }

    // Convert binary string to hexadecimal
    string converted;
    for (int i = 0; i < no_of_bits; i += 4) {
        string nibble = bin_str.substr(i, 4);
        converted += Bin_Hex_Conv[nibble];
    }
    return converted;
}

int string_to_decimal(string s, int no_of_bits) {
    int value = 0;

    if (no_of_bits == 32 || no_of_bits == 24) {
        int len = s.length();

        // Handle the case for "0"
        if (s == "0") return 0;

        // Check for existing labels
        int labelStatus = is_valid_operand(s);
        if (labelStatus == 5) return labels[s];
        else if (labelStatus == 1) {
            Errors.insert({0, "No such Label"});
            return 0;
        }

        // Determine the type of number based on the first character
        char firstChar = s[0];
        switch (firstChar) {
            case '-':
                if (s[1] == '0') { // Octal
                    if (is_valid_operand(s.substr(2, len - 2)) == 3) {
                        for (int i = len - 1, multiplier = 1; i >= 2; --i, multiplier *= 8) {
                            value += ((s[i] - '0') * multiplier);
                        }
                        value *= -1;
                    } else {
                        Errors.insert({0, "Not a Valid Format(0x-hex and 0-oct)"});
                    }
                } else { // Decimal
                    bool cond1= is_valid_operand(s.substr(1, len - 1)) == 3 ;
                    bool cond2= is_valid_operand(s.substr(1, len - 1)) == 2 ;
                    if (cond1||cond2) {
                        value = stoi(s.substr(1, len - 1)) * -1;
                    } else {
                        Errors.insert({0, "Not a Valid Format(0x-hex and 0-oct)"});
                    }
                }
                return value;

            case '+':
                if (s[1] == '0') { // Octal
                    if (is_valid_operand(s.substr(2, len - 2)) == 3) {
                        for (int i = len - 1, multiplier = 1; i >= 2; --i, multiplier *= 8) {
                            value += ((s[i] - '0') * multiplier);
                        }
                    } else {
                        Errors.insert({0, "Not a Valid Format(0x-hex and 0-oct)"});
                    }
                } else { // Decimal
                    bool condn1=is_valid_operand(s.substr(1, len - 1)) == 3;
                    bool condn2=is_valid_operand(s.substr(1, len - 1)) == 2;
                    if (condn1||condn2) {
                        value = stoi(s.substr(1, len - 1));
                    } else {
                        Errors.insert({0, "Not a Valid Format(0x-hex and 0-oct)"});
                    }
                }
                return value;

            case '0':
                if (len > 1 && s[1] == 'x') { // Hexadecimal
                    bool flag1=is_valid_operand(s.substr(2, len - 2)) == 3;
                    bool flag2=is_valid_operand(s.substr(2, len - 2)) == 2;
                    bool flag3=is_valid_operand(s.substr(2, len - 2)) == 4;
                    if (flag1||flag2||flag3) {
                        string hexPart = s.substr(2, len - 2);
                        hexPart.insert(0, (no_of_bits / 4) - hexPart.length(), '0'); // Pad with zeros
                        unsigned int x;
                        stringstream ss;
                        ss << hex << hexPart;
                        ss >> x;
                        value = static_cast<int>(x);
                    } else {
                        Errors.insert({0,"Not a Valid Format(0x-hex and 0-oct)"});
                    }
                } else { // Octal
                    if (is_valid_operand(s.substr(1, len - 1)) == 3) {
                        for (int i = len - 1, multiplier = 1; i >= 1; --i, multiplier *= 8) {
                            value += ((s[i] - '0') * multiplier);
                        }
                    } else {
                        Errors.insert({0, "Not a Valid Format(0x-hex and 0-oct)"});
                    }
                }
                return value;

            default: // Decimal
                if (is_valid_operand(s) == 3 || is_valid_operand(s) == 2) {
                    value = stoi(s);
                } else {
                    Errors.insert({0, "Not a Valid Format(0x-hex and 0-oct)"});
                }
                return value;
        }
    } else {
        // Handle mnemonics
        if (noOperandInstr.find(s) != noOperandInstr.end()) {
            return noOperandInstr[s];
        } else if (oneOperandInstr.find(s) != oneOperandInstr.end()) {
            return oneOperandInstr[s];
        }
    }
    return 0;
}
void Set_labels() {
    for (int i = 0; i < 100000; i++) {
        if (source_code[i].empty())
            break;

        // Process the first case
        if (source_code[i].size() == 3 && source_code[i][1] == "SET") {
            string label = source_code[i][0].substr(0, source_code[i][0].length() - 1);
            if (labels.find(label) != labels.end()) {
                labels[label] = string_to_decimal(source_code[i][2], 32);
            }
        }
        // Process the second case
        else if (source_code[i].size() == 4 && source_code[i][2] == "SET") {
            string label = source_code[i][1].substr(0, source_code[i][1].length() - 1);
            if (labels.find(label) != labels.end()) {
                labels[label] = string_to_decimal(source_code[i][3], 32);
            }
        }
    }
}
// Used to convert given assmebly to machine code and along with that identify more errors 
void calculate_machine_code(int instruction_type,string mnemonic,string operand,int itr,string label_to_be_used)
{
	int dec = 0;
	switch(instruction_type){
		case 0: machine_code_int[itr] = noOperandInstr[mnemonic];
		        machine_code[itr] = decimal_to_2scomplement(noOperandInstr[mnemonic],32);
				break;
	    case 1: if(mnemonic == "data"){
					used_labels.insert(label_to_be_used);
					dec = string_to_decimal(operand,32);
					machine_code_int[itr] = dec;
					machine_code[itr] = decimal_to_2scomplement(dec,32);
				}
				else if(mnemonic == "SET"){
					used_labels.insert(label_to_be_used);
					dec = string_to_decimal(operand,32);
					machine_code_int[itr] = dec;
					machine_code[itr] = decimal_to_2scomplement(dec,32);
				}
				else{
					int value2 = string_to_decimal(mnemonic,8);
					int value1 = string_to_decimal(operand,24);
					if(offsetInstr.find(mnemonic) != offsetInstr.end() ){
						if(is_valid_label(operand))     value1 = (value1 - itr) - 1;								
					}
					machine_code_int[itr] = value1<<8 | value2;
					machine_code[itr] = decimal_to_2scomplement(value1,24) + decimal_to_2scomplement(value2,8);
				}
	}
}
void SecondPass() {
    for (int lineIndex = 0; lineIndex < 100000; lineIndex++) {
        if (source_code[lineIndex].empty())
            break;

        // Handle instructions with only a mnemonic (no label)
        if (source_code[lineIndex].size() == 1) {
            calculate_machine_code(0, source_code[lineIndex][0], "", lineIndex, "");
            continue;
        }

        string firstElement = source_code[lineIndex][0];
        string secondElement = source_code[lineIndex][1];
        bool hasFirstLabel = (firstElement.back() == ':');
        bool hasSecondLabel = (secondElement.back() == ':');

        string mnemonic, operand;

        // Case: instruction after two labels
        if (hasFirstLabel && hasSecondLabel) {
            mnemonic = source_code[lineIndex][2];
            if (noOperandInstr.count(mnemonic)) {
                calculate_machine_code(0, mnemonic, "", lineIndex, "");
            } else {
                if (source_code[lineIndex].size() >= 4) 
                    operand = source_code[lineIndex][3];

                if (is_valid_label(operand) && labels.count(operand)) 
                    used_labels.insert(operand);

                calculate_machine_code(1, mnemonic, operand, lineIndex, secondElement.substr(0, secondElement.length() - 1));
            }
        }
        // Case: instruction after a single label
        else if (hasFirstLabel) {
            mnemonic = source_code[lineIndex][1];
            if (noOperandInstr.count(mnemonic)) {
                calculate_machine_code(0, mnemonic, "", lineIndex, "");
            } else {
                if (source_code[lineIndex].size() >= 3) 
                    operand = source_code[lineIndex][2];

                if (is_valid_label(operand) && labels.count(operand)) 
                    used_labels.insert(operand);

                calculate_machine_code(1, mnemonic, operand, lineIndex, firstElement.substr(0, firstElement.length() - 1));
            }
        }
        // Case: instruction without label
        else {
            mnemonic = source_code[lineIndex][0];
            if (noOperandInstr.count(mnemonic)) {
                calculate_machine_code(0, mnemonic, "", lineIndex, "");
            } else {
                bool check1=source_code[lineIndex].size() >= 2;
                if (check1) 
                    operand = source_code[lineIndex][1];
                bool check2=is_valid_label(operand) && labels.count(operand);
                if (check2) 
                    used_labels.insert(operand);

                calculate_machine_code(1, mnemonic, operand, lineIndex, "");
            }
        }
    }
}

void Extra_labels()
{
	for(auto it=labels.begin();it!=labels.end();it++)
	{
		string s = it->first;
		if(used_labels.find(s) == used_labels.end())
			Warnings.insert({0,"Unused Label"});	
	}
}

void display_warnings()
{
	cout << "Warnings : (Line 0 is an unidentified location) \n";
	for(auto it=Warnings.begin();it!=Warnings.end();it++)
	{
		cout << "Line " << it->first << " : " << it->second << "\n";
	}
	cout << "\n";
}

void display_errors()
{
	cout << "Errors : (Line 0 is an unidentified location) \n";
	for(auto it=Errors.begin();it!=Errors.end();it++)
	{
		cout << "Line " << it->first << " : " << it->second << "\n";
	}
	cout << "\n";
}

void write_log_file() {
    string log_filename = Filename + ".log";
    fstream log_fptr(log_filename, ios::out);    // file is opened in the write mode,if not present then will be created

    if (log_fptr.is_open()) {   //if file is successfully opened
        log_fptr << "LabelName : Value\n";   //header to display labels and their values
        for (auto a: labels) {   //traversing on the map labels and printing in log file
            log_fptr << a.first << ":" << a.second << "\n";
        }

        log_fptr << "\nUsed Labels\n";
        for (auto b: used_labels) {   //traversing on the set used_labels and printing in log file
            if (!b.empty()) {
                log_fptr <<b<< "\n";
            }
        }

        log_fptr << "\nWarnings\n";
        for (auto c : Warnings) {    //traversing on the set warnings and printing the warnings in log file
            log_fptr << "Line " << c.first << " : " << c.second << "\n";
        }

        log_fptr << "\nErrors\n";
        for (auto d: Errors) {  //traversing on the set errors and printing the errors in log file
            log_fptr << "Line " << d.first << " : " << d.second << "\n";
        }
    }
    log_fptr.close();
}

void write_list_file() {
    fstream list_fptr(Filename + ".lst", ios::out);

    if (list_fptr) { //file has opened/created successfully
        for (int i = 0; i < 100000 && !source_code[i].empty(); i++) {
            list_fptr << decimal_to_2scomplement(i, 32) << " " 
                       << machine_code[i] << " ";

            for (const auto& token : source_code[i]) {
                list_fptr << token << " ";
            }
            list_fptr << "\n";
        }
    }
}

void write_obj_file()
{
	FILE* obj_fptr;
	string new_file_name = Filename;
	new_file_name += ".o";
	obj_fptr = fopen(new_file_name.c_str(),"wb");//opening in write_binary mode if file won't be present it will be created
	int size = 0;
    int it=0;
	while(it<100000)
	{
        bool check_empty=source_code[it].empty();
 		if(check_empty)
			break;
		size++;
        it++;
	}
	fwrite(machine_code_int,sizeof(int),size,obj_fptr);
	fclose(obj_fptr);
	return;
}







