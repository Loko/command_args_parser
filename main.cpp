#include "CommandArgsParser.h"
#include <iostream>
#include <cstring>

// These are the actual variables users can create easily in static memory
// Then they just need to use the GetX() function on it and they're done!
CommandArgVariable g_TestInteger("g_TestInteger", CommandArgVariableType::Integer, 0);
CommandArgVariable g_EnableExtraLogging("g_EnableExtraLogging", CommandArgVariableType::Boolean, false);
CommandArgVariable g_TestFloat("g_TestFloat", CommandArgVariableType::Float, 0.0f);
CommandArgVariable g_UserStringPrefix("g_UserStringPrefix", CommandArgVariableType::CString, "user");

// SetPlayerPosition x y z
// e.g. SetPlayerPosition 3.0 6.0 -1.0
CONSOLE_COMMAND_FUNCTION(SetPlayerPosition)(CommandArgsParser & args) {
	float fx = 0.0f, fy = 0.0f, fz = 0.0f;
	args.IncrementTokenAndParseVector3(fx, fy, fz);

	std::cout << "SetPlayerPosition Command Invoked pArgs = " << args.GetInputString() << " x = " << fx <<
		" y = " << fy << " z = " << fz << std::endl;
	return 1;
}

// A more complicated command
// SetPerformanceTestPosition [-pos x y z] [-a] [-file fileName]
// In this instance we can take the modifiers in any order and handle 
// a vector3, a flag, and a c-string output file
CONSOLE_COMMAND_FUNCTION(SetPerformanceTestPosition)(CommandArgsParser & args) {
	// If COMMANDS_ARGS_PARSER_MAKES_COPY_OF_INPUT_STRING is disabled we have to do this before incrementing any tokens
	// Unfortunately strtok_s modifies the original string for us
	std::cout << "SetPerformanceTestPosition Command Invoked pArgs = " << args.GetInputString() << std::endl;
	bool bSomeFlag = false;
	float fx = 0.0f, fy = 0.0f, fz = 0.0f;
	const char * pDesiredFile = "";
	while (char * pCurToken = args.IncrementToken())
	{
		if (args.CompareToken(pCurToken, "-pos")) {
			args.IncrementTokenAndParseVector3(fx, fy, fz);
		} else if (args.CompareToken(pCurToken, "-a")) {
			bSomeFlag = true;
		} else if (args.CompareToken(pCurToken, "-file")) {
			pDesiredFile = args.IncrementToken();
		}
	}

	std::cout << "SetPerformanceTestPosition Command " << "-pos x = " << fx <<
		" y = " << fy << " z = " << fz << " -a " << bSomeFlag << " -file " << pDesiredFile << std::endl;

	return 1;
}

void PrintCurrentCommandVariables() {
	std::cout << "g_TestInteger = " << g_TestInteger.GetInt() << std::endl;
	std::cout << "g_EnableExtraLogging = " << g_EnableExtraLogging.GetBool() << std::endl;
	std::cout << "g_TestFloat  = " << g_TestFloat.GetFloat() << std::endl;
	std::cout << "g_UserStringPrefix = " << g_UserStringPrefix.GetCString() << std::endl;
}

int main(int argc, char * argv[]) {

	std::cout << "Print Command Variables Before Args File..." << std::endl;
	PrintCurrentCommandVariables();

	std::cout << "SetupAllCommandArgs..." << std::endl;
	CommandArgsMgr::GetInstance().SetupAllCommandArgs(argc, argv);

	std::cout << "Print Command Variables After Args File..." << std::endl;
	PrintCurrentCommandVariables();

	char buffer[256];
	snprintf(buffer, 256, "SetPlayerPosition %.3f %.3f %.3f", 2.0f, 5.0f, 7.0f);
	CommandArgsMgr::GetInstance().Execute(buffer);
	return 0;
}