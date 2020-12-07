#include <stage1.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <cctype>
#include <stack>
#include <iomanip>


//    make stage1
//    ./stage1 101.dat my.lst my.asm
//    diff /usr/local/4301/data/stage1/101.asm my.asm
//    make my               change makefile to targetasm  my                    this is to make sure it compiles and runs just enter numbers and then hit enter





using namespace std;
//member function

Compiler::Compiler(char** argv) { // constructor
	//Open the input and output streams
   sourceFile.open(argv[1]);
   listingFile.open(argv[2]);
   objectFile.open(argv[3]);
}

Compiler::~Compiler() { // destructor
	// PrintSymbolTable();
  sourceFile.close();
  listingFile.close();
  objectFile.close();
}

void Compiler::createListingHeader() {
	time_t now = time (NULL); // This returns the current calendar time of the system in number of seconds elapsed since January 1, 1970.
	listingFile << "STAGE1:" << "Brett Hedden & David Roberts " << ctime(&now) << endl;
	listingFile << "LINE NO." << setw(30) <<  "SOURCE STATEMENT" << endl << endl;
	
	//line numbers and source statements should be aligned under the headings
}

void Compiler::parser() {
	nextChar();
	//ch must be initialized to the first character of the source file
   
   
     //set charac to the first char in the file
   if(nextToken() != "program") 
   {
       //
      processError("keyword \"program\" expected");
   }
   //a call to nextToken() has two effec	// (1) the variable, token, is assigned the value of the next token
	// (2) the next token is read from the source file in order to make
	// the assignment. The value returned by nextToken() is also
	// the next token.ts

   prog();
   //parser implements the grammar rules, calling first rule
}

void Compiler::createListingTrailer() {
	//print "COMPILATION TERMINATED", "# ERRORS ENCOUNTERED"
   if (errorCount == 1) {
      listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERROR ENCOUNTERED" << endl;
   }
   else
      listingFile << "\nCOMPILATION TERMINATED      " << errorCount << " ERRORS ENCOUNTERED" << endl;
}


///////////////////////////////////////////////////////////////////////////////
// stage 0 production 1 token should be "program"
void Compiler::prog() {
	if (token != "program")
	{
		processError("keyword \"program\" expected");
	}
	progStmt();
	if (token == "const")
	{
		consts();
	}
	if (token == "var")
	{
		vars();
	}
	if (token != "begin")
	{
		processError("keyword \"begin\" expected");
	}
	beginEndStmt();
	if (token[0] != END_OF_FILE)
	{
		processError("no text may follow \"end\"");
	}
}

// stage 0 production 2

void Compiler::progStmt() {
	string x;
	if (token != "program")
	{
		processError("keyword \"program\" expected");
	}
	x = nextToken();
	if (!isNonKeyId(token))
	{
		processError("program name expected");
	}
	if (nextToken() != ";")
	{
		processError("semicolon expected");
	}
	nextToken();
	code("program", x);
	insert(x,PROG_NAME,CONSTANT,x,NO,0);
}

// stage 0 production 3

void Compiler::consts() {
	if (token != "const")
	{
		processError("keyword \"const\" expected");
	}
	if (!isNonKeyId(nextToken()))
	{
		processError("non-keyword identifier must follow \"const\"");
	}
	constStmts();
}

// stage 0 production 4

void Compiler::vars() {
	if (token != "var")
	{
		processError("keyword \"var\" expected");
	}
	if (!isNonKeyId(nextToken()))
	{
		processError("non-keyword identifier must follow \"var\"");
	}
	varStmts();
}

// stage 0 production 5

void Compiler::beginEndStmt() {
	if (token != "begin")
		processError("keyword \"begin\" expected");

	// change as of stage 1
	nextToken();
	if (isNonKeyId(token) || token == "read" || token == "write" || token == ";" || token == "begin") {
		execStmts();
	}
	
	if (token != "end")
		processError("keyword \"end\" expected");
	if (nextToken() != ".")
		processError("period expected");
	nextToken();
	code("end", ".");
}

// stage 0 production 6

void Compiler::constStmts() {
	string x, y;

	if (!isNonKeyId(token)){
		processError("non-keyword identifier expected");
   }
	x = token;	// x has the constant's name

	// check for format x = y;
	if (nextToken() != "="){
		processError("\"=\" expected");
   }
	y = nextToken();	// y has the value of that constant

	// if y is not one of "+","-","not",NON_KEY_ID,"true","false",INTEGER
	if (y != "+" && y != "-" && y != "not" && !isNonKeyId(y) && !isBoolean(y) && !isInteger(y)){	// y is not a number, true-false or a non-key ID
		processError("token to right of \"=\" illegal");
   }
	if (y == "+" || y == "-"){
		if (!isInteger(nextToken()))
			processError("integer expected after sign");

		y += token;
	}

	if (y == "not"){
		if (!isBoolean(nextToken())){	// if after not isn't "true" or "false"
			processError("boolean expected after \"not\"");
      }
		if (token == "true"){
			y = "false";
      }
		else{
			y = "true";
      }
	}



	// check for format: x = y;
	if (nextToken() != ";"){
		processError("semicolon expected");
   }
	if (whichType(y) != INTEGER && whichType(y) != BOOLEAN){
		processError("data type of token on the right-hand side must be INTEGER or BOOLEAN");
   }
	insert(x, whichType(y), CONSTANT, whichValue(y), YES, 1);
	x = nextToken();

	// if after a constant declaration is another non_key_id or "var" or "begin"
	if (x != "begin" && x != "var" && !isNonKeyId(x)){
		processError("non-keyword identifier, \"begin\", or \"var\" expected");
   }
	if (isNonKeyId(x)){
		constStmts();	// call it again to insert another constant
   }
 
}

// stage 0 production 7

void Compiler::varStmts() {
  
	string x,y;
	if (!isNonKeyId(token))
	{
		processError("non-keyword identifier expected");
	}
	x = ids();
   
	if (token != ":")
	{
		processError("\":\" expected");
	}
	if (nextToken() != "integer" && token != "boolean")
	{
		processError("illegal type follows \":\"");
	}
	y = token;
   
	if (nextToken() != ";")
	{
		processError("semicolon expected");
	}
	if (y == "integer") insert(x, storeTypes::INTEGER, modes::VARIABLE, "1", allocation::YES, 1);
	else insert(x, storeTypes::BOOLEAN, modes::VARIABLE, "1", allocation::YES, 1);
	if (nextToken() != "begin"&& !isNonKeyId(token))
	{
		processError("non-keyword identifier or \"begin\" expected");
	}
	if (isNonKeyId(token))
	{
		varStmts();
	}
   
}


// stage 0 production 8
string Compiler::ids() {
	//token should be NON_KEY_ID
   string temp,tempString;
	if(!isNonKeyId(token))
	{
		processError("non-keyword identifier expected");
	}
	tempString = token;
	temp = token;
	if(nextToken() == ",")
	{
		if(!isNonKeyId(nextToken()))
		{
			processError("non-keyword identifier expected");
		}
		tempString = temp + "," + ids();
	}
	return tempString;
}

///////////////////////////////////////////////////////////////////////////////
void Compiler::execStmts(){       // stage 1, production 2
 
	if (isNonKeyId(token) || token == "read" || token == "write" || token == ";" ||  token == "begin")
	{
		execStmt();
		nextToken();
		execStmts(); 
	}else if (token == "end");

	else processError("non - keyword identifier, \"read\", \"write\", or \"begin\" expected");
   
   
}

void Compiler::execStmt() {       // stage 1, production 3 //done
   	if (isNonKeyId(token))
   	{
		assignStmt();
	}
	else if (token == "read")
	{
		readStmt();
	}
	else if (token == "write") 
	{
		writeStmt();
	}
   else 
   {
	   processError("non-keyword id, \"read\", or \"write\" expected");
   }
}

void Compiler::assignStmt() {     // stage 1, production 4 //done
	string secondOperand, firstOperand;
	if (!isNonKeyId(token))
	{
		processError("non - keyword identifier expected");
	}
	//Token must be already defined
	if (symbolTable.count(token) == 0) processError("reference to undefined variable");
	{
	pushOperand(token);
	nextToken();
	}
	if (token != ":=") 
	{
		processError("':=' expected; found " + token);
	}
	else 
	{
		pushOperator(":=");
	}
	nextToken();

	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token) && token != ";")
	{
		processError("one of \"*\", \"and\", \"div\", \"mod\", \")\", \"+\", \"-\", \";\", \"<\", \"<=\", \"<>\", \"=\", \">\", \">=\", or \"or\" expected");
	}
	else 
	{
		express();
	}
	secondOperand = popOperand();
	firstOperand = popOperand();
	code(popOperator(), secondOperand, firstOperand);
}

void Compiler::readStmt() {       // stage 1, production 5 //done
	string x = "";
	if (nextToken() != "(") 
	{
		processError("'(' expected");
	}

	nextToken();
	string readList = ids();

	for (unsigned int i = 0; i < readList.length(); i++) 
	{
		if (readList[i] == ',') 
		{
			code("read", x);
			x = "";
		}
		else 
		{
			x += readList[i];
		}
	}
	code("read", x);

	if (token != ")") 
	{
		processError("',' or ')' expected after non-keyword identifier");
	}
	if (nextToken() != ";") 
	{
		processError("';' expected");
	}
}

void Compiler::writeStmt() {      // stage 1, production 7 //done
	string x = "";
	if (nextToken() != "(") 
	{
		processError("'(' expected");
	}

	nextToken();
	string writeList = ids();

	for (unsigned int i = 0; i < writeList.length(); i++) 
	{
		if (writeList[i] == ',') 
		{
			code("write", x);
			x = "";
		}
		else 
		{
			x += writeList[i];
		}
	}
	code("write", x);

	if (token != ")") 
	{
		processError("',' or ')' expected after non-keyword identifier");
	}
	if (nextToken() != ";") 
	{
		processError("';' expected");
	}
}

void Compiler::express() {        // stage 1, production 9 //done
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+" && token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", non - keyword identifier or integer expected");
   	}
	term();

	if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">")
	{
		expresses();
   	}
}

void Compiler::expresses() {      // stage 1, production 10 //done
	string x = "";
	string operand1, operand2;
	if (token != "=" && token != "<>" && token != "<=" && token != ">=" && token != "<" && token != ">")
	{
		processError("\"=\", \"<>\", \"<=\", \">=\", \"<\", or \">\" expected");
	}
	pushOperator(token);
	nextToken();

	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}
	else 
	{
		term();
	}
	operand1 = popOperand();
	operand2 = popOperand();

	code(popOperator(), operand1, operand2);

	if (token == "<>" || token == "=" || token == "<=" || token == ">=" || token == "<" || token == ">")
	{
		expresses();
	}
}

void Compiler::term() {           // stage 1, production 11 //done
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}
	factor();

	if (token == "-" || token == "+" || token == "or")
	{
		terms();
	}
}

void Compiler::terms() {          // stage 1, production 12 //done
	string x = "";
	string operand1, operand2;

	if (token != "+" && token != "-" && token != "or")
	{
		processError("\"+\", \"-\", or \"or\" expected");
	}
	pushOperator(token);
	nextToken();

	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token))
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}
	else
	{
		factor();
	}
	operand1 = popOperand();
	operand2 = popOperand();
	code(popOperator(), operand1, operand2);

	if (token == "+" || token == "-" || token == "or")
	{
		terms();
	}
}

void Compiler::factor() {         // stage 1, production 13 //done
	if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
		&& token != "-" && !isInteger(token) && !isNonKeyId(token)){
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}

	part();

	if (token == "*" || token == "div" || token == "mod" || token == "and")
	{
		factors();
	}
	else if (token == "=" || token == "<>" || token == "<=" || token == ">=" || token == "<" || token == ">" ||  
		token == ")" || token == ";" || token == "-" || token == "+" || token == "or" || token == "begin"){}
	else
	{
		processError("invalid expression");
	}
}

void Compiler::factors() {        // stage 1, production 14 //done
	string x = "";
	string operand1, operand2;
	if (token != "*" && token != "div" && token != "and" && token != "mod")
   {
		processError("\"*\", \"div\", \"mod\", or \"and\" expected");
   }
	pushOperator(token);
	nextToken();

	if (token != "not" && token != "+" && token != "-" && token != "(" && token != "true" && token != "false" && !isInteger(token) && !isNonKeyId(token))
	{  
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
	}
   else
   {
	   part();
   }
	operand1 = popOperand();
	operand2 = popOperand();
	code(popOperator(), operand1, operand2);
	if (token == "*" || token == "div" || token == "mod" || token == "and")
  	{
		factors();
   }
}

void Compiler::part() {           // stage 1, production 15 //done
	string x = "";
	if (token == "not")
	{
		nextToken();
		if (token == "(") 
		{
			nextToken();
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
				&& token != "-" && !isInteger(token) && !isNonKeyId(token))
			{
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			}
			express();
			if (token != ")")
			{
				processError(") expected; found " + token);
			}
			nextToken();
			code("not", popOperand());
		}

		else if (isBoolean(token)) 
		{
			if (token == "true") 
			{
				pushOperand("false");
				nextToken();
			}
			else 
			{
				pushOperand("true");
				nextToken();
			}
		}

		else if (isNonKeyId(token)) 
		{
			code("not", token);
			nextToken();
		}
	}

	else if (token == "+")
	{
		nextToken();
		if (token == "(") 
		{
			nextToken();
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
				&& token != "-" && !isInteger(token) && !isNonKeyId(token))
			{
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			}
			express();
			if (token != ")") 
			{
				processError("expected ')'; found " + token);
			}
			nextToken();
		}
		else if (isInteger(token) || isNonKeyId(token)) 
		{
			pushOperand(token);
			nextToken();
		}

		else
		{
			processError("expected '(', integer, or non-keyword id; found " + token);
		}
	}

	else if (token == "-")
	{
		nextToken();
		if (token == "(") 
		{
			nextToken();
			if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
				&& token != "-" && !isInteger(token) && !isNonKeyId(token))
			{
				processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
			}
			express();
			if (token != ")")
			{
				processError("expected ')'; found " + token);
			}
			nextToken();
			code("neg", popOperand());
		}
		else if (isInteger(token)) 
		{
			pushOperand("-" + token);
			nextToken();
		}
		else if (isNonKeyId(token))
		{
			code("neg", token);
			nextToken();
		}
	}

	else if (token == "(") 
	{
		nextToken();
		if (token != "not" && token != "true" && token != "false" && token != "(" && token != "+"
			&& token != "-" && !isInteger(token) && !isNonKeyId(token))
		{
			processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, or non - keyword identifier expected");
		}
		express();
		if (token != ")")
		{
			processError(") expected; found " + token);
		}
		nextToken();
	}

	else if (isInteger(token) || isBoolean(token) || isNonKeyId(token))
	{
		pushOperand(token);
		nextToken();
	}

	else
	{
		processError("\"not\", \"true\", \"false\", \"(\", \"+\", \"-\", integer, boolean, or non - keyword identifier expected");
	}
}
////////////////////////////////////////
// Helper functions
bool Compiler::isKeyword(string s) const { // determines if s is a keyword //done
	if (  s == "program"
		|| s == "const"
		|| s == "var"
		|| s == "integer"
		|| s == "boolean"
		|| s == "begin"
		|| s == "end"
		|| s == "true"
		|| s == "false"
		|| s == "not"
		|| s == "mod"
		|| s == "div"
		|| s == "and"
		|| s == "or"
		|| s == "read"
		|| s == "write") 
	{
		return true;
	}
	return false;
}
bool Compiler::isSpecialSymbol(char c) const { // determines if c is a special symbol //done
	if (  c == '='
		|| c == ':'
		|| c == ','
		|| c == ';'
		|| c == '.'
		|| c == '+'
		|| c == '-'
		|| c == '*'
		|| c == '<'
		|| c == '>'
		|| c == '('
		|| c == ')') {
		return true;
	}
	return false;
}

bool Compiler::isNonKeyId(string s) const { // determines if s is a non_key_id //done
	if (isKeyword(s)){
		return false;
   }
	if (s[s.length() - 1] == '_'){
		return false;
   }
	for (uint i = 0; i < s.length(); ++i){
		if (!(islower(s[0]) && (isdigit(s[i]) || islower(s[i]) || !(s[i] == '_' && s[i + 1] == '_')))){
			return false;
      }
   }
	return true;
}

bool Compiler::isInteger(string s) const { // determines if s is an integer //done

	if (symbolTable.find(s) != symbolTable.end())
	{
		if (symbolTable.find(s)->second.getDataType() == INTEGER){
			return true;
      }
		else{
			return false;
      }
	}
   
	if (s.length() == 1 && (s == "+" || s == "-")){
		return false;
   }
	
	for (uint i = 0; i < s.length(); ++i){
		if (!(isdigit(s[i]) || s[0] == '+' || s[0] == '-')){
			return false;
      }
   }
	return true;
}

bool Compiler::isBoolean(string s) const { // determines if s is a boolean //done
	if (s == "true" || s == "false") {
		return true;
	}
	return false;
}

bool Compiler::isLiteral(string s) const { // determines if s is a literal  //done
	return isInteger(s) || isBoolean(s) || s == "+" || s == "-" || s == "not";
}

// Action routines

void Compiler::insert(string externalName, storeTypes inType, modes inMode, string inValue, allocation inAlloc, int inUnits) { //done
//create symbol table entry for each identifier in list of external names
//Multiply inserted names are illegal

	uint i = 0;
	while (i < externalName.length())
	{
		string name = "";
		while (i < externalName.length() && externalName[i] != ',')
		{
			name += externalName[i];
			i++;
		}
		i++;	// go to the next character after ','

		if (name != "")
		{
			// we can only have max of 15 chars in table
			name = name.substr(0, 15);

			//symbolTable[name] is defined
			if (symbolTable.find(name) != symbolTable.end()){
				processError("multiple name definition" + name);
         }
			else if (isKeyword(name) && name != "true" && name != "false"){
				processError("illegal use of keyword");
         }
			
			else
			{//create table entry
				if (isupper(name[0])){
					symbolTable.insert({ name, SymbolTableEntry(name, inType, inMode, inValue, inAlloc, inUnits) });
            }
				else{
					symbolTable.insert({ name, SymbolTableEntry(genInternalName(inType), inType, inMode, inValue, inAlloc, inUnits) });
            }
         }
		}
	}

	if (symbolTable.size() > 256){
		processError("symbol table overflow -- max 256 entries");
   }
}

storeTypes Compiler::whichType(string name) { //tells which data type a name has //done
   storeTypes type;
   if (isLiteral(name))
   {
      if (isBoolean(name))
      {
         type = BOOLEAN;
      }
      else if (isInteger(name))
      {
         type = INTEGER;
      }
   }
   else //name is an identifier and hopefully a constant
   {
      if (symbolTable.find(name) != symbolTable.end()) 
      {
         type = symbolTable.at(name).getDataType();
      }
       else
      {
         processError("reference to undefined variable ");
      }

   }
   return type;
}

string Compiler::whichValue(string name) { //tells which value a name has
	string value;
   if (isLiteral(name)){
      if ( name == "false"){
         value = "0";
      }else if ( name == "true") {
         value = "-1";
      }else{
         value = name;
      }
   }
   else //name is an identifier and hopefully a constant
   {
      if (symbolTable.count(name) > 0 && symbolTable.at(name).getValue() != "") 
      {
         value = symbolTable.at(name).getValue();
      }
      else
      {     
         processError("reference to undefined constant");
      }
   }
   return value;
}

void Compiler::code(string op, string operand1, string operand2) { //done
	if (op == "program"){
		emitPrologue(operand1);
   }
	else if (op == "end"){
		emitEpilogue();
   }
	else if (op == "read"){
		emitReadCode(operand1);
   }
	else if (op == "write"){
		emitWriteCode(operand1);
   }
	else if (op == "+"){ // this must be binary '+'
		emitAdditionCode(operand1, operand2);
   }
	else if (op == "-"){ // this must be binary '-'
		emitSubtractionCode(operand1, operand2);
   }
	else if (op == "neg"){ // this must be unary '-'
		emitNegationCode(operand1);
   }
	else if (op == "not"){
		emitNotCode(operand1);
   }
	else if (op == "*"){
		emitMultiplicationCode(operand1, operand2);
   }
	else if (op == "div"){
		emitDivisionCode(operand1, operand2);
   }
	else if (op == "mod"){
		emitModuloCode(operand1, operand2);
   }
	else if (op == "and"){
		emitAndCode(operand1, operand2);
   }
	else if (op == "=") {
		emitEqualityCode(operand1, operand2);
	}
	else if (op == "<>") {
		emitInequalityCode(operand1, operand2);
	}
	else if (op == "or") {
		emitOrCode(operand1, operand2);
	}
	else if (op == "<") {
		emitLessThanCode(operand1, operand2);
	}
	else if (op == ">") {
		emitGreaterThanCode(operand1, operand2);
	}
	else if (op == "<=") {
		emitLessThanOrEqualToCode(operand1, operand2);
	}
	else if (op == ">=") {
		emitGreaterThanOrEqualToCode(operand1, operand2);
	}
	else if (op == ":=") {
		emitAssignCode(operand1, operand2);
	}
	else{
		processError("compiler error since function code should not be called with illegal arguments ");
   }
}

void Compiler::pushOperator(string name) { // Push name onto opertorStk //done
	// push name onto stack;
	operatorStk.push(name);
}

string Compiler::popOperator() { // pop name from operandStk //done
   string temp;
	// if operatorStk is not empty
	// return top element removed from stack;
	// else
	// processError(compiler error; operator stack underflow)
	if (!operatorStk.empty()) {
		temp = operatorStk.top();
		operatorStk.pop();
		
	}else{
      processError("operator stack underflow");
   }
   
   return temp;
}

void Compiler::pushOperand(string operand) { // Push name onto operandStk //done
	//if name is a literal, also create a symbol table entry for it

   // if name is a literal and has no symbol table entry
   // insert symbol table entry, call whichType to determine the data type of the literal
   // push name onto stack;
	if (symbolTable.count(operand) == 0) {
		if (isInteger(operand) || operand == "true" || operand == "false") {
			insert(operand, whichType(operand), CONSTANT, whichValue(operand), YES, 1);
      }
	}
	operandStk.push(operand);
}

string Compiler::popOperand() { //pop name from operandStk //done
	string temp;
	// if operandStk is not empty
	// return top element removed from stack;
	// else
	// processError(compiler error; operand stack underflow)
	if (!operandStk.empty()) {
		temp = operandStk.top();
		operandStk.pop();	
	}
   else 
   {
	processError("operand stack underflow");
   }
   return temp;
}


void Compiler::emit(string label, string instruction, string operands, string comment) //done
{
	// Turn on left justification in objectFile
   // Output label in a field of width 8
   // Output instruction in a field of width 8
   // Output the operands in a field of width 24
   // Output the comment
   objectFile.setf(ios_base::left);
   objectFile << setw(8) << label;
   objectFile << setw(8) << instruction ;
   objectFile << setw(24) << operands;
   objectFile << comment << endl;
}

void Compiler::emitPrologue(string progName, string operand2) //done
{
	time_t now = time (NULL); 
   // Output identifying comments at beginning of objectFile
   objectFile << "; Brett Hedden & David Roberts      " << ctime(&now);
	objectFile << "%INCLUDE \"Along32.inc\"\n" "%INCLUDE \"Macros_Along.inc\"\n\n";
   // Output the %INCLUDE directives
   emit("SECTION", ".text");
   emit("global", "_start", "", "; program " + progName.substr(0, 15));
   objectFile << endl;
   emit("_start:");
}

void Compiler::emitEpilogue(string operand1, string operand2) //done
{
	emit("","Exit", "{0}");
   objectFile << endl;
   emitStorage();
}

void Compiler::emitStorage() //done
{
	emit("SECTION", ".data");
   // for those entries in the symbolTable that have
   // an allocation of YES and a storage mode of CONSTANT
   // { call emit to output a line to objectFile }
   for (auto i : symbolTable){ //use auto its shorter 
      if(i.second.getAlloc() == YES && i.second.getMode() == CONSTANT){
         emit(i.second.getInternalName(), "dd",i.second.getValue(), "; " + i.first);
         
      }     
   }
   objectFile << endl;
   emit("SECTION", ".bss");
   // for those entries in the symbolTable that have
   // an allocation of YES and a storage mode of VARIABLE
   // { call emit to output a line to objectFile }
   // if (isInSymbolTable == true && storeTypes(VARIABLE)) {
	// }
   for (auto i : symbolTable){
      if(i.second.getAlloc() == YES && i.second.getMode() == VARIABLE){
         emit(i.second.getInternalName(), "resd",i.second.getValue(), "; " + i.first);
      }
      
   }
}

void Compiler::emitReadCode(string operand, string) { //done
	string name;
	for (uint i = 0; i < operand.size(); ++i) {
	
		if (operand[i] != ',' && i < operand.size()) {
			name += operand[i];
			continue;
		}

		if (name != "") {
			if (symbolTable.count(name) == 0){
				processError("reference to undefined symbol " + name);
         }
			if (symbolTable.at(name).getDataType() != INTEGER){
				processError("can't read variables of this type");
         }
			if (symbolTable.at(name).getMode() != VARIABLE){
				processError("attempting to read to a read-only location");
         }
			emit("", "call", "ReadInt", "; read int; value placed in eax");
			emit("", "mov", "[" + symbolTable.at(name).getInternalName() + "],eax", "; store eax at " + name);
			contentsOfAReg = symbolTable.at(name).getInternalName();
		}
		name = "";
	}

	if (name != "") {
		if (symbolTable.count(name) == 0){
			processError("reference to undefined symbol " + name);
      }
		if (symbolTable.at(name).getDataType() != INTEGER){
			processError("can't read variables of this type");
      }
		if (symbolTable.at(name).getMode() != VARIABLE){
			processError("attempting to read to a read-only location");
      }
		emit("", "call", "ReadInt", "; read int; value placed in eax");
		emit("", "mov", "[" + symbolTable.at(name).getInternalName() + "],eax", "; store eax at " + name);
		contentsOfAReg = symbolTable.at(name).getInternalName();
	}
}

void Compiler::emitWriteCode(string operand, string) { //done

   string name;
	static bool definedStorage = false;

	for (uint i = 0; i < operand.size(); ++i) {

		if (operand[i] != ',' && i < operand.size()) {
			name += operand[i];
			continue;
		}

		if (name != "") {
			if (symbolTable.count(name) == 0){
				processError("reference to undefined symbol " + name);
         }
			if (symbolTable.at(name).getInternalName() != contentsOfAReg) {
				emit("", "mov", "eax,[" + symbolTable.at(name).getInternalName() + "]", "; load " + name + " in eax");
				contentsOfAReg = symbolTable.at(name).getInternalName();
			}
			if (symbolTable.at(name).getDataType() == INTEGER){
				emit("", "call", "WriteInt", "; write int in eax to standard out");
         }
			else {
				emit("", "cmp", "eax,0", "; compare to 0");
				string firstLabel = getLabel(), secondLabel = getLabel();
				emit("", "je", "." + firstLabel, "; jump if equal to print FALSE");
				emit("", "mov", "edx,TRUELIT", "; load address of TRUE literal in edx");
				emit("", "jmp", "." + secondLabel, "; unconditionally jump to ." + secondLabel);
				emit("." + firstLabel + ":");
				emit("", "mov", "edx,FALSLIT", "; load address of FALSE literal in edx");
				emit("." + secondLabel + ":");
				emit("", "call", "WriteString", "; write string to standard out");

				if (definedStorage == false) {
					definedStorage = true;
					objectFile << endl;
					emit("SECTION", ".data");
					emit("TRUELIT", "db", "'TRUE',0", "; literal string TRUE");
					emit("FALSLIT", "db", "'FALSE',0", "; literal string FALSE");
					objectFile << endl;
					emit("SECTION", ".text");
				}
			}

			emit("", "call", "Crlf", "; write \\r\\n to standard out");
		}
		name = "";
	}

	if (symbolTable.count(name) == 0){
		processError("reference to undefined symbol " + name);
   }
	if (symbolTable.at(name).getInternalName() != contentsOfAReg) {
		emit("", "mov", "eax,[" + symbolTable.at(name).getInternalName() + "]", "; load " + name + " in eax");
		contentsOfAReg = symbolTable.at(name).getInternalName();
	}
	if (symbolTable.at(name).getDataType() == INTEGER){
		emit("", "call", "WriteInt", "; write int in eax to standard out");
   }
	else { 
		emit("", "cmp", "eax,0", "; compare to 0");
		string firstLabel = getLabel(), secondLabel = getLabel();
		emit("", "je", "." + firstLabel, "; jump if equal to print FALSE");
		emit("", "mov", "edx,TRUELIT", "; load address of TRUE literal in edx");
		emit("", "jmp", "." + secondLabel, "; unconditionally jump to ." + secondLabel);
		emit("." + firstLabel + ":");
		emit("", "mov", "edx,FALSLIT", "; load address of FALSE literal in edx");
		emit("." + secondLabel + ":");
		emit("", "call", "WriteString", "; write string to standard out");

		if (definedStorage == false) {
			definedStorage = true;
			objectFile << endl;
			emit("SECTION", ".data");
			emit("TRUELIT", "db", "'TRUE',0", "; literal string TRUE");
			emit("FALSLIT", "db", "'FALSE',0", "; literal string FALSE");
			objectFile << endl;
			emit("SECTION", ".text");
		}
	}

	emit("", "call", "Crlf", "; write \\r\\n to standard out");
   
}

void Compiler::emitAssignCode(string operand1, string operand2) { // op2 = op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType()){
		processError("incompatible types for operator ':='");
   }
	if (symbolTable.at(operand2).getMode() != VARIABLE){
		processError("symbol on left-hand side of assignment must have a storage mode of VARIABLE");
   }
	if (operand1 == operand2){
      return;
   }
	if (symbolTable.at(operand1).getInternalName() != contentsOfAReg){
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
   }
	emit("", "mov", "[" + symbolTable.at(operand2).getInternalName() + "],eax", "; " + operand2 + " = AReg");

	contentsOfAReg = symbolTable.at(operand2).getInternalName();
	
	if (isTemporary(operand1)) {
		freeTemp();
	}
}

void Compiler::emitAdditionCode(string operand1, string operand2) { // op2 +  op1 //done

	if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	if (symbolTable.at(operand1).getDataType() != INTEGER 
		|| symbolTable.at(operand2).getDataType() != INTEGER){
		processError("binary '+' requires integer operands");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
	}

	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "add", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " + " + operand1);
   }
	else{
		emit("", "add", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " + " + operand2);
   }
	if (isTemporary(operand1)) {
		freeTemp();
	}
   if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitSubtractionCode(string operand1, string operand2) {    // op2 -  op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	if (symbolTable.at(operand1).getDataType() != INTEGER
		|| symbolTable.at(operand2).getDataType() != INTEGER){
		processError("binary '-' requires integer operands");
   }
   
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(allocation::YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		contentsOfAReg = "";
	}

	if (contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "sub", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " - " + operand1);
   }
	if (isTemporary(operand1)) {
		freeTemp();
	}
   if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitMultiplicationCode(string operand1, string operand2) { // op2 *  op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }

	if (symbolTable.at(operand1).getDataType() != INTEGER ||
		symbolTable.at(operand2).getDataType() != INTEGER){
		processError("binary '*' requires integer operands");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "imul", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " * " + operand1);
   }
	else {
      emit("", "imul", "dword [" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " * " + operand2);
   }
   
	if (isTemporary(operand1)) {
		freeTemp();
	}
   if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitDivisionCode(string operand1, string operand2) {        // op2 /  op1 //done
	if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
	}
	if (symbolTable.at(operand1).getDataType() != INTEGER ||
		symbolTable.at(operand2).getDataType() != INTEGER){
		processError("binary 'div' requires integer operands");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}
	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);

	if (isTemporary(operand1)) {
		freeTemp();
	}
   if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitModuloCode(string operand1, string operand2) {        // op2 %  op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
	}
	if (symbolTable.at(operand1).getDataType() != INTEGER || symbolTable.at(operand2).getDataType() != INTEGER){
		processError("binary 'mod' requires integer operands");
   } 
   
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName())
	{
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	emit("", "cdq", "", "; sign extend dividend from eax to edx:eax");
	emit("", "idiv", "dword [" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " div " + operand1);
	emit("", "xchg", "eax,edx", "; exchange quotient and remainder");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitNegationCode(string operand1, string) {           // -op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	
	if (symbolTable.at(operand1).getDataType() != INTEGER){
		processError("binary '-' requires integer operands");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
		contentsOfAReg = symbolTable.at(operand1).getInternalName();
	}

	emit("", "neg", "eax", "; AReg = -AReg");
	if (isTemporary(operand1)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(INTEGER);
	pushOperand(contentsOfAReg);
}

void Compiler::emitNotCode(string operand1, string) {                // !op1  //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	if (symbolTable.at(operand1).getDataType() != BOOLEAN){
		processError("binary 'not' requires boolean operands");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand1);
		contentsOfAReg = symbolTable.at(operand1).getInternalName();
	}

	emit("", "not", "eax", "; AReg = !AReg");
	if (isTemporary(operand1)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitAndCode(string operand1, string operand2) {       // op2 && op1 //done

	if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	
	if (symbolTable.at(operand1).getDataType() != BOOLEAN ||
		symbolTable.at(operand2).getDataType() != BOOLEAN){
		processError("binary 'and' requires boolean operands");
   }

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "and", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " and " + operand1);
	}
   else{
		emit("", "and", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " and " + operand2);
   }
	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitOrCode(string operand1, string operand2) {        // op2 || op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	
	if (symbolTable.at(operand1).getDataType() != BOOLEAN ||
		symbolTable.at(operand2).getDataType() != BOOLEAN){
		processError("binary 'or' requires boolean operands");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "or", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; AReg = " + operand2 + " or " + operand1);
   }
	else{
		emit("", "or", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand1 + " or " + operand2);
   }

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitEqualityCode(string operand1, string operand2) { // op2 == op1 //done

   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }

	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}
	
	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
   }
	else{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
   }
	string firstLabel = getLabel(), secondLabel = getLabel();


	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "je", "." + firstLabel, "; if " + operand2 + " = " + operand1 + " then jump to set eax to TRUE");
   }
	else {
		emit("", "je", "." + firstLabel, "; if " + operand1 + " = " + operand2 + " then jump to set eax to TRUE");
   }
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitInequalityCode(string operand1, string operand2) {     // op2 != op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
	}
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType()){
		processError("incompatible types for operator '<>'");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
   }
	else{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
   }

	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "jne", "." + firstLabel, "; if " + operand2 + " <> " + operand1 + " then jump to set eax to TRUE");
   }
	else{
		emit("", "jne", "." + firstLabel, "; if " + operand1 + " <> " + operand2 + " then jump to set eax to TRUE");
   }
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false",BOOLEAN,CONSTANT,"0",YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true",BOOLEAN,CONSTANT,"-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitLessThanCode(string operand1, string operand2) {       // op2 <  op1 //done
   if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType()){
		processError("incompatible types for operator '<'");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
   }
	else{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
   }
	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "jl", "." + firstLabel, "; if " + operand2 + " < " + operand1 + " then jump to set eax to TRUE");
   }
	else {
		emit("", "jl", "." + firstLabel, "; if " + operand1 + " < " + operand2 + " then jump to set eax to TRUE");
   }
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", BOOLEAN,CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}
	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitLessThanOrEqualToCode(string operand1, string operand2) { // op2 <= op1 //done

	if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
	}
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType()){
		processError("incompatible types for operator '<='");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
   }
	else{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
   }
	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "jle", "." + firstLabel, "; if " + operand2 + " <= " + operand1 + " then jump to set eax to TRUE");
   }
	else{
		emit("", "jle", "." + firstLabel, "; if " + operand1 + " <= " + operand2 + " then jump to set eax to TRUE");
   }
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false",BOOLEAN,CONSTANT,"0",YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true",BOOLEAN,CONSTANT, "-1",YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitGreaterThanCode(string operand1, string operand2) {    // op2 > op1 //done

	if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
	}
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType()){
		processError("incompatible types for operator '>'");
   }
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
   }
	else{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
   }
	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "jg", "." + firstLabel, "; if " + operand2 + " > " + operand1 + " then jump to set eax to TRUE");
   }
	else{
		emit("", "jg", "." + firstLabel, "; if " + operand1 + " > " + operand2 + " then jump to set eax to TRUE");
   }
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

void Compiler::emitGreaterThanOrEqualToCode(string operand1, string operand2) { // op2 >= op1 //done

	if (symbolTable.count(operand1) == 0){
		processError("reference to undefined symbol " + operand1);
   }
	else if (symbolTable.count(operand2) == 0){
		processError("reference to undefined symbol " + operand2);
   }
	if (symbolTable.at(operand1).getDataType() != symbolTable.at(operand2).getDataType()){
		processError("incompatible types for operator '>='");
   }
   
	if (contentsOfAReg[0] == 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName() 
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "[" + contentsOfAReg + "],eax", "; deassign AReg");
		symbolTable.at(contentsOfAReg).setAlloc(YES);
		contentsOfAReg = "";
	}

	if (!contentsOfAReg.empty() && contentsOfAReg[0] != 'T' && contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()){
		contentsOfAReg = "";
   }
	if (contentsOfAReg != symbolTable.at(operand1).getInternalName()
		&& contentsOfAReg != symbolTable.at(operand2).getInternalName()) {
		emit("", "mov", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; AReg = " + operand2);
		contentsOfAReg = symbolTable.at(operand2).getInternalName();
	}

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "cmp", "eax,[" + symbolTable.at(operand1).getInternalName() + "]", "; compare " + operand2 + " and " + operand1);
   }
	else{
		emit("", "cmp", "eax,[" + symbolTable.at(operand2).getInternalName() + "]", "; compare " + operand1 + " and " + operand2);
   }
	string firstLabel = getLabel(), secondLabel = getLabel();

	if (contentsOfAReg == symbolTable.at(operand2).getInternalName()){
		emit("", "jge", "." + firstLabel, "; if " + operand2 + " >= " + operand1 + " then jump to set eax to TRUE");
   }
	else{
		emit("", "jge", "." + firstLabel, "; if " + operand1 + " >= " + operand2 + " then jump to set eax to TRUE");
   }
	emit("", "mov", "eax,[FALSE]", "; else set eax to FALSE");

	if (symbolTable.count("false") == 0) {
		insert("false", BOOLEAN, CONSTANT, "0", YES, 1);
		symbolTable.at("false").setInternalName("FALSE");
	}
	emit("", "jmp", "." + secondLabel, "; unconditionally jump");
	emit("." + firstLabel + ":");
	emit("", "mov", "eax,[TRUE]", "; set eax to TRUE");

	if (symbolTable.count("true") == 0) {
		insert("true", BOOLEAN, CONSTANT, "-1", YES, 1);
		symbolTable.at("true").setInternalName("TRUE");
	}
	emit("." + secondLabel + ":");

	if (isTemporary(operand1)) {
		freeTemp();
	}
	if (isTemporary(operand2)) {
		freeTemp();
	}

	contentsOfAReg = getTemp();
	symbolTable.at(contentsOfAReg).setDataType(BOOLEAN);
	pushOperand(contentsOfAReg);
}

// lexical scanner

char Compiler::nextChar() //returns the next character or end of file marker //done
{
	sourceFile.get(ch);

	static char prevChar = '\n';

	if (sourceFile.eof())
	{
		ch = END_OF_FILE;
		return ch;
	}
	else
	{
		if (prevChar == '\n')
      {
			listingFile << setw(5) << ++lineNo << '|';
      }
		listingFile << ch;
	}

	prevChar = ch;
	return ch;
}



string Compiler::nextToken() //returns the next token or end of file marker //done
{
	token = "";
	
	while (token == "")
	{
		if (ch == '{')
		{
			while (nextChar() && ch != END_OF_FILE && ch != '}')
			{}	// do nothing
		
			if (ch == END_OF_FILE)
         {
				processError("unexpected end of file");
         }
			else
         {
				nextChar();
         }
		}
		
		else if (ch == '}')
      {
			processError("'}' cannot begin token");
      }
		else if (isspace(ch))
      {
			nextChar();
		}
		else if (isSpecialSymbol(ch))
		{
			token = ch;
			nextChar();
			
			if (token == ":" && ch == '=')
			{
				token += ch;
				nextChar();
			}
			else if (token == "<" && (ch == '>' || ch == '='))
			{
				token += ch;
				nextChar();
			}
			else if (token == ">" && ch == '=')
			{
				token += ch;
				nextChar();
			}
		}
		
		else if (islower(ch))
		{
			token += ch;
			
			while ((nextChar() == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
				|| (ch >= '0' && ch <= '9')) && ch != END_OF_FILE)
					token += ch;
			
			if (ch == END_OF_FILE)
         {
				processError("unexpected end of file");
         }
		}
		
		else if (isdigit(ch))
		{
			token = ch;
			
			while (isdigit(nextChar()) && ch != END_OF_FILE
				&& !isSpecialSymbol(ch))
            {
					token += ch;
            }
			if (ch == END_OF_FILE)
         {
				processError("unexpected end of file");
         }
		}
		
		else if (ch == END_OF_FILE)
      {
			token = ch;
		}
		else
      {
			processError("illegal symbol");
      }
	}
	
	return token;
}


// Other routines

string Compiler::genInternalName(storeTypes stype) const { //done
	string internalName;

	switch (stype) {
	case PROG_NAME:
   {
		internalName = "P0";
		break;
   }
	case INTEGER:
	{
		int intCount = 0;
		for (auto itr : symbolTable) {
			if (itr.second.getDataType() == INTEGER && itr.first[0] != 'T') ++intCount;
		}

		internalName = "I" + to_string(intCount);
		break;
	}
	case BOOLEAN:
	{
		int boolCount = 0;
		for (auto itr : symbolTable) {
			if (itr.second.getDataType() == BOOLEAN) ++boolCount;
		}
		internalName = "B" + to_string(boolCount);
		break;
	}
	case UNKNOWN: {}
	}

	return internalName;
}

void Compiler::processError(string err) { //done
	++errorCount;
	listingFile << endl << "Error: Line " << lineNo << ": " << err << endl;
	createListingTrailer();

	exit(1);	
}

void Compiler::freeTemp() {
	currentTempNo--;
	if (currentTempNo < -1) {
		processError("compiler error, currentTempNo should be = –1");
	}
}

string Compiler::getTemp() { //done
	string temp;
	currentTempNo++;
	temp = "T" + to_string(currentTempNo);
	if (currentTempNo > maxTempNo) {
		insert(temp, UNKNOWN, VARIABLE, "1", NO, 1);
		symbolTable.at(temp).setInternalName(temp);
		maxTempNo++;
	}

	return temp;
}

string Compiler::getLabel() { //done

	static int labelNo = -1;
	string temp;
	labelNo++;
	temp = "L" + to_string(labelNo);
	return temp;
}

bool Compiler::isTemporary(string s) const { // determines if s represents a temporary //done
	if (s[0] == 'T')
   {      
      return true;
   }
   else 
   {
      return false;
   }
}
