#ifndef COMMAND_ARGS_PARSER_H
#define COMMAND_ARGS_PARSER_H

#include <unordered_map>
#include <cstdint>

/// Tagged variant variable type
namespace CommandArgVariableType {
	enum Type {
		None,
		Integer,
		Float,
		Boolean,
		CString
	};
}

/// Flags for the CommandArgVariable class 
namespace CommandArgVariableFlags {
	enum Flags {
		OwnsCString = 1
	};
}

/// Tagged variant class used for command line variables
/// Might own the cstring if OwnsCString flag is set
/// Must be placed in static memory - will register the command on construction
class CommandArgVariable {
public:

	CommandArgVariable() = delete;
	~CommandArgVariable();
	CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const int defaultIntValue, const uint8_t flags = 0);
	CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const bool defaultBoolValue, const uint8_t flags = 0);
	CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const float defaultFloatValue, const uint8_t flags = 0);
	CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const char * defaultCStringValue, const uint8_t flags = 0);

	int GetInt() const;
	float GetFloat() const;
	bool GetBool() const;
	const char * GetCString() const;
	void SetInt(const int i);
	void SetFloat(const float f);
	void SetBool(const bool b);
	void SetCString(const char * pString);
	CommandArgVariableType::Type GetType() const { return static_cast<CommandArgVariableType::Type>(m_Type); }
	void SetFlags(const uint8_t flags) { m_Flags = flags; }
	uint8_t GetFlags() const { return m_Flags; }

private:
	union {
		int	  m_AsInt;
		float m_AsFloat;
		bool  m_AsBool;
		const char * m_AsCString;
	} m_Data;			// 8 bytes for char * ptr.  Use memset to be safe.
	int8_t m_Type;		// CommandArgType::Type
	uint8_t m_Flags;	// CommandArgVariableFlags::Flags
};

// If enabled, we make a deep copy of the input string
// The advantage of doing this is that after making IncrementToken 
// calls m_pInputString is still unmodified
#define COMMANDS_ARGS_PARSER_MAKES_COPY_OF_INPUT_STRING (0)

class CommandArgsParser {
public:
	static const char * ms_DefaultDelimeters;

	static bool Parse_Bool(const char * pString, bool & rInOutBool);
	static bool Parse_Integer(const char * pString, int & rInOutInt);
	static bool Parse_Float(const char * pString, float & rInOutFloat);

	CommandArgsParser();
	~CommandArgsParser();
	char * GetInputString() const { return m_pInputString; }
	char * GetCurrentToken() const { return m_pCurrentToken; }
	void InitWithArgs(char * pFullString);
	char * IncrementToken(const char * pDelimeters = ms_DefaultDelimeters);
	bool CompareToken(const char * pCurToken, const char * pToCompareTo) const;
	bool IncrementTokenAndParseInt(int & rInOutInt, const char * pDelimeters = ms_DefaultDelimeters);
	bool IncrementTokenAndParseFloat(float & rInOutFloat, const char * pDelimeters = ms_DefaultDelimeters);
	bool IncrementTokenAndParseVector2(float & fx, float & fy, const char * pDelimeters = ms_DefaultDelimeters);
	bool IncrementTokenAndParseVector3(float & fx, float & fy, float & fz, const char * pDelimeters = ms_DefaultDelimeters);
	void Reset();

private:

	char * m_pInputString;
	char * m_pInputStringTokenize;
	char * m_pCurrentToken;
	char * m_pNextToken;
	bool   m_bHasProcessedFirstToken;
};

typedef int(*ConsoleCommandFunc)(CommandArgsParser & args);

/// Helper class that allows REGISTER_CONSOLE_COMMAND_FUNCTION to effectively be called 
/// as part of the CONSOLE_COMMAND_FUNCTION macro
class RegisterCommandArgFunctionAuto {
public:
	RegisterCommandArgFunctionAuto() = delete;
	RegisterCommandArgFunctionAuto(const char * pCommandName, const ConsoleCommandFunc pFunc);
};

#define CONSOLE_COMMAND_FUNCTION_MANUAL_REGISTRATION( commandName ) int Command_##commandName
#define REGISTER_CONSOLE_COMMAND_FUNCTION( commandName ) CommandLineArgs::RegisterCommandArgFunction( #commandName, &Command_##commandName )

#define CONSOLE_COMMAND_FUNCTION( commandName )	int Command_##commandName(CommandArgsParser & ); \
												RegisterCommandArgFunctionAuto s_auto##commandName( #commandName, &Command_##commandName); \
												int Command_##commandName

namespace CommandArgEntryType {
	enum Type {
		Variable,
		Function
	};
}

/// Each entry is either a command or a variable this tagged variant holds 
/// one or the other
class CommandArgEntry {
public:

	CommandArgEntry();
	const ConsoleCommandFunc GetFunction() const;
	CommandArgVariable * GetVariable() const;
	void SetFunction(const ConsoleCommandFunc pFunc);
	void SetVariable(CommandArgVariable * pVariable);
	void SetType(const CommandArgEntryType::Type nType);
	unsigned int GetType() const { return m_Type; }

private:
	union {
		CommandArgVariable * m_pVariable;
		ConsoleCommandFunc m_pFunction;
	} m_Data;
	uint8_t m_Type;  // CommandArgEntryType::Type
};

/// Singleton interface for command arg functions and variables
/// Initialize with SetupAllCommandArgs()
/// Invoke with Execute()
class CommandArgsMgr {
public:

	static CommandArgsMgr & GetInstance() { return ms_Instance; }
	static uint32_t HashCommandLineArg(const char * pArgName);
	static uint32_t HashCommandLineArg_StartEnd(const char * pStart, const char * pEnd);
	static char * FindFirstNonWhitespaceCharacter(const char * pString, const char * pWhitespaceCharacters = CommandArgsParser::ms_DefaultDelimeters);
	static char * FindFirstWhitespaceCharacterAfterFirstToken(const char * pString, const char * pWhitespaceCharacters = CommandArgsParser::ms_DefaultDelimeters);

	uint32_t RegisterCommandArgVariable(const char * pArgName, CommandArgVariable * ptr);
	void RegisterCommandArgFunction(const char * pArgName, const ConsoleCommandFunc pFunc);

	int GetIntegerForKey(const uint32_t key);
	float GetFloatForKey(const uint32_t key);
	bool GetBoolForKey(const uint32_t key);
	const char * GetCStringForKey(const uint32_t key);
	void SetupAllCommandArgs(const int argc, char * argv[]);
	int Execute(const char * pCommand);

	bool FindCommandArgEntry(std::unordered_map<uint32_t, CommandArgEntry>::iterator & inOutIterator, const uint32_t key);

private:
	std::unordered_map<uint32_t, CommandArgEntry> m_CommandArgsMap;

	static CommandArgsMgr ms_Instance;
};

#endif // COMMAND_ARGS_PARSER_H
