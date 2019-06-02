#include "CommandArgsParser.h"
#include <fstream>
#include <string>
#include <cassert>

CommandArgVariable::CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const int defaultIntValue, const uint8_t flags) : m_Flags(flags) {
	assert(nType == CommandArgVariableType::Integer);
	memset(&m_Data, 0, sizeof(m_Data));
	CommandArgsMgr::GetInstance().RegisterCommandArgVariable(pVariableName, this);
	m_Type = nType;
	m_Data.m_AsInt = defaultIntValue;
}

CommandArgVariable::CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const bool defaultBoolValue, const uint8_t flags) : m_Flags(flags) {
	assert(nType == CommandArgVariableType::Boolean);
	memset(&m_Data, 0, sizeof(m_Data));
	CommandArgsMgr::GetInstance().RegisterCommandArgVariable(pVariableName, this);
	m_Type = nType;
	m_Data.m_AsBool = defaultBoolValue;
}

CommandArgVariable::CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const float defaultFloatValue, const uint8_t flags) : m_Flags(flags) {
	assert(nType == CommandArgVariableType::Float);
	memset(&m_Data, 0, sizeof(m_Data));
	CommandArgsMgr::GetInstance().RegisterCommandArgVariable(pVariableName, this);
	m_Type = nType;
	m_Data.m_AsFloat = defaultFloatValue;
}

CommandArgVariable::CommandArgVariable(const char * pVariableName, const CommandArgVariableType::Type nType, const char * defaultCStringValue, const uint8_t flags) : m_Flags(flags) {
	assert(nType == CommandArgVariableType::CString);
	memset(&m_Data, 0, sizeof(m_Data));
	CommandArgsMgr::GetInstance().RegisterCommandArgVariable(pVariableName, this);
	m_Type = nType;
	m_Data.m_AsCString = defaultCStringValue;
}

CommandArgVariable::~CommandArgVariable() {
	if (m_Type == CommandArgVariableType::CString) {
		if (m_Data.m_AsCString != nullptr) {
			if (m_Flags & CommandArgVariableFlags::OwnsCString) {
				delete[] m_Data.m_AsCString;
				m_Data.m_AsCString = nullptr;
			}
		}
	}
}

int CommandArgVariable::GetInt() const {
	if (m_Type == CommandArgVariableType::Integer) {
		return m_Data.m_AsInt;
	}
	return 0;
}

float CommandArgVariable::GetFloat() const {
	if (m_Type == CommandArgVariableType::Float) {
		return m_Data.m_AsFloat;
	}
	return 0.0f;
}

bool CommandArgVariable::GetBool() const {
	if (m_Type == CommandArgVariableType::Boolean) {
		return m_Data.m_AsBool;
	}
	return false;
}

const char * CommandArgVariable::GetCString() const {
	if (m_Type == CommandArgVariableType::CString) {
		return m_Data.m_AsCString;
	}
	return "\0"; // maybe should be nullptr?
}

void CommandArgVariable::SetInt(const int i) {
	if (m_Type == CommandArgVariableType::Integer) {
		m_Data.m_AsInt = i;
	}
}

void CommandArgVariable::SetFloat(const float f) {
	if (m_Type == CommandArgVariableType::Float) {
		m_Data.m_AsFloat = f;
	}
}

void CommandArgVariable::SetBool(const bool b) {
	if (m_Type == CommandArgVariableType::Boolean) {
		m_Data.m_AsBool = b;
	}
}

void CommandArgVariable::SetCString(const char * pString) {
	if (m_Type == CommandArgVariableType::CString) {
		// It's possible we set this c-string multiple times
		// If it's owned we need to be careful to delete the old one
		// The calling code is responsible for setting OwnsCString flag 
		// which should probably only be done in the Execute function
		const char * pCurCString = GetCString();
		if (pCurCString != nullptr) {
			if (m_Flags & CommandArgVariableFlags::OwnsCString) {
				delete[] pCurCString;
			}
		}
		m_Data.m_AsCString = pString;
	}
}

const char * CommandArgsParser::ms_DefaultDelimeters = " \t\n\v\f\r";

bool CommandArgsParser::Parse_Bool(const char * pString, bool & rInOutBool) {
	// Do case insensitive compare first
	// It's valid to use 0 and 1 as well
	if (_strcmpi(pString, "true") == 0) {
		rInOutBool = true;
	}
	else if (_strcmpi(pString, "false") == 0) {
		rInOutBool = false;
	}
	else {
		int num = atoi(pString);
		rInOutBool = (num != 0);
	}
	return true;
}

bool CommandArgsParser::Parse_Integer(const char * pString, int & rInOutInt) {
	rInOutInt = atoi(pString);
	return true;
}

bool CommandArgsParser::Parse_Float(const char * pString, float & rInOutFloat) {
	double d = atof(pString);
	rInOutFloat = static_cast<float>(d);
	return true;
}

CommandArgsParser::CommandArgsParser() : m_pCurrentToken(nullptr), m_pInputString(nullptr), m_pInputStringTokenize(nullptr), m_pNextToken(nullptr), m_bHasProcessedFirstToken(false) {

}

CommandArgsParser::~CommandArgsParser() {
#if COMMANDS_ARGS_PARSER_MAKES_COPY_OF_INPUT_STRING
	if (m_pInputStringTokenize != nullptr) {
		delete[] m_pInputStringTokenize;
		m_pInputStringTokenize = nullptr;
	}
#endif //
}

void CommandArgsParser::InitWithArgs(char * pFullString) {
	Reset();
	m_pInputString = pFullString;
#if COMMANDS_ARGS_PARSER_MAKES_COPY_OF_INPUT_STRING
	size_t stringLen = strlen(pFullString);
	size_t stringSize = stringLen + 1;
	m_pInputStringTokenize = new char[stringSize];
	strcpy_s(m_pInputStringTokenize, stringSize, pFullString);
#else
	m_pInputStringTokenize = pFullString;
#endif
}

char * CommandArgsParser::IncrementToken(const char * pDelimeters /*= ms_DefaultDelimeters*/) {
	char * pCurToken = nullptr;
	if (!m_bHasProcessedFirstToken) {
		pCurToken = strtok_s(m_pInputStringTokenize, pDelimeters, &m_pNextToken);
	}
	else {
		pCurToken = strtok_s(nullptr, pDelimeters, &m_pNextToken);
	}
	m_pCurrentToken = pCurToken;
	m_bHasProcessedFirstToken = true;
	return pCurToken;
}

bool CommandArgsParser::CompareToken(const char * pCurToken, const char * pToCompareTo) const {
	const int compareResult = _strcmpi(pCurToken, pToCompareTo);
	return (compareResult == 0);
}

bool CommandArgsParser::IncrementTokenAndParseInt(int & rInOutInt, const char * pDelimeters /*= ms_DefaultDelimeters*/) {
	char * pCurToken = IncrementToken(pDelimeters);
	if (!pCurToken) {
		return false;
	}
	return Parse_Integer(pCurToken, rInOutInt);
}

bool CommandArgsParser::IncrementTokenAndParseFloat(float & rInOutFloat, const char * pDelimeters /*= ms_DefaultDelimeters*/) {
	char * pCurToken = IncrementToken(pDelimeters);
	if (!pCurToken) {
		return false;
	}
	return Parse_Float(pCurToken, rInOutFloat);
}

bool CommandArgsParser::IncrementTokenAndParseVector2(float & fx, float & fy, const char * pDelimeters /*= ms_DefaultDelimeters*/) {
	for (int v = 0; v < 2; ++v) {
		if (v == 0) {
			if (!IncrementTokenAndParseFloat(fx, pDelimeters)) {
				return false;
			}
		}
		else if (v == 1) {
			if (!IncrementTokenAndParseFloat(fy, pDelimeters)) {
				return false;
			}
		}
	}
	return true;
}

bool CommandArgsParser::IncrementTokenAndParseVector3(float & fx, float & fy, float & fz, const char * pDelimeters /*= ms_DefaultDelimeters*/) {
	for (int v = 0; v < 3; ++v) {
		if (v == 0) {
			if (!IncrementTokenAndParseFloat(fx, pDelimeters)) {
				return false;
			}
		}
		else if (v == 1) {
			if (!IncrementTokenAndParseFloat(fy, pDelimeters)) {
				return false;
			}
		}
		else {
			if (!IncrementTokenAndParseFloat(fz, pDelimeters)) {
				return false;
			}
		}
	}
	return true;
}

void CommandArgsParser::Reset() {
	m_pInputString = m_pCurrentToken = m_pNextToken = nullptr;
	m_bHasProcessedFirstToken = false;
#if COMMANDS_ARGS_PARSER_MAKES_COPY_OF_INPUT_STRING
	if (m_pInputStringTokenize != nullptr) {
		delete[] m_pInputStringTokenize;
		m_pInputStringTokenize = nullptr;
	}
#endif //
}

/// RAII class to just setup the function from a single macro
RegisterCommandArgFunctionAuto::RegisterCommandArgFunctionAuto(const char * pCommandName, const ConsoleCommandFunc pFunc) {
	CommandArgsMgr::GetInstance().RegisterCommandArgFunction(pCommandName, pFunc);
}

CommandArgEntry::CommandArgEntry() : m_Type(CommandArgEntryType::Variable) {
	memset(&m_Data, 0, sizeof(m_Data));
}

const ConsoleCommandFunc CommandArgEntry::GetFunction() const {
	if (m_Type == CommandArgEntryType::Function) {
		return m_Data.m_pFunction;
	}
	return nullptr;
}

CommandArgVariable * CommandArgEntry::GetVariable() const {
	if (m_Type == CommandArgEntryType::Variable) {
		return m_Data.m_pVariable;
	}
	return nullptr;
}

void CommandArgEntry::SetFunction(const ConsoleCommandFunc pFunc) {
	m_Data.m_pFunction = pFunc;
}

void CommandArgEntry::SetVariable(CommandArgVariable * pVariable) {
	m_Data.m_pVariable = pVariable;
}

void CommandArgEntry::SetType(const CommandArgEntryType::Type nType) {
	m_Type = nType;
}

CommandArgsMgr CommandArgsMgr::ms_Instance;

// Jenkins One At A Time for these hash functions
uint32_t CommandArgsMgr::HashCommandLineArg(const char * pArgName) {
	if (!pArgName) { return 0; }
	uint32_t hash = 0;
	while (uint8_t cCur = tolower(*pArgName)) {
		hash += cCur;
		hash += hash << 10;
		hash ^= hash >> 6;
		++pArgName;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

uint32_t CommandArgsMgr::HashCommandLineArg_StartEnd(const char * pStart, const char * pEnd) {
	if (!pStart || !pEnd) { return 0; }
	uint32_t hash = 0;
	for (const char * pCur = pStart; pCur < pEnd; ++pCur) {
		const uint8_t cCur = tolower(*pCur);
		if (cCur == 0) {
			break;
		}
		hash += cCur;
		hash += hash << 10;
		hash ^= hash >> 6;
	}
	hash += hash << 3;
	hash ^= hash >> 11;
	hash += hash << 15;
	return hash;
}

char * CommandArgsMgr::FindFirstNonWhitespaceCharacter(const char * pString, const char * pWhitespaceCharacters /*= CommandArgsParser::ms_DefaultDelimeters*/) {
	if (!pString) { return nullptr; }
	char * pCurCharPtr = const_cast<char *>(pString);
	for (; *pCurCharPtr; ++pCurCharPtr) {
		const char cCurChar = *pCurCharPtr;
		bool isWhiteSpace = strchr(pWhitespaceCharacters, cCurChar) != nullptr;
		if (!isWhiteSpace) {
			break;
		}
	}
	return pCurCharPtr;
}

char * CommandArgsMgr::FindFirstWhitespaceCharacterAfterFirstToken(const char * pString, const char * pWhitespaceCharacters /*= CommandArgsParser::ms_DefaultDelimeters*/) {
	if (!pString) { return nullptr; }
	char * pCurCharPtr = const_cast<char *>(pString);
	for (; *pCurCharPtr; ++pCurCharPtr) {
		const char cCurChar = *pCurCharPtr;
		bool isWhiteSpace = strchr(pWhitespaceCharacters, cCurChar) != nullptr;
		if (isWhiteSpace) {
			break;
		}
	}
	return pCurCharPtr;
}

uint32_t CommandArgsMgr::RegisterCommandArgVariable(const char * pArgName, CommandArgVariable * ptr) {
	if (!ptr) { return 0; }
	const uint32_t hashValue = HashCommandLineArg(pArgName);
	if (hashValue == 0) {
		return 0;
	}
	std::unordered_map<uint32_t, CommandArgEntry>::const_iterator cit = m_CommandArgsMap.find(hashValue);
	if (cit == m_CommandArgsMap.cend()) {
		CommandArgEntry sNewEntry;
		sNewEntry.SetType(CommandArgEntryType::Variable);
		sNewEntry.SetVariable(ptr);
		m_CommandArgsMap.insert(std::make_pair(hashValue, sNewEntry));
		return hashValue;
	}
	return 0;
}

void CommandArgsMgr::RegisterCommandArgFunction(const char * pArgName, const ConsoleCommandFunc pFunc) {
	if (!pFunc) { return; }
	const uint32_t hashValue = HashCommandLineArg(pArgName);
	if (hashValue == 0) {
		return;
	}
	std::unordered_map<uint32_t, CommandArgEntry>::const_iterator cit = m_CommandArgsMap.find(hashValue);
	if (cit == m_CommandArgsMap.cend()) {
		CommandArgEntry sEntry;
		sEntry.SetType(CommandArgEntryType::Function);
		sEntry.SetFunction(pFunc);
		m_CommandArgsMap.insert(std::make_pair(hashValue, sEntry));
	}
}

int CommandArgsMgr::GetIntegerForKey(const uint32_t key) {
	std::unordered_map<uint32_t, CommandArgEntry>::const_iterator cit = m_CommandArgsMap.find(key);
	if (cit != m_CommandArgsMap.cend()) {
		const CommandArgEntry & rEntry = cit->second;
		if (rEntry.GetType() == CommandArgEntryType::Variable) {
			const CommandArgVariable * pVariable = rEntry.GetVariable();
			if (pVariable != nullptr) {
				if (pVariable->GetType() == CommandArgVariableType::Integer) {
					return pVariable->GetInt();
				}
			}
		}
	}
	return 0;
}

float CommandArgsMgr::GetFloatForKey(const uint32_t key) {
	std::unordered_map<uint32_t, CommandArgEntry>::const_iterator cit = m_CommandArgsMap.find(key);
	if (cit != m_CommandArgsMap.cend()) {
		const CommandArgEntry & rEntry = cit->second;
		if (rEntry.GetType() == CommandArgEntryType::Variable) {
			const CommandArgVariable * pVariable = rEntry.GetVariable();
			if (pVariable != nullptr) {
				if (pVariable->GetType() == CommandArgVariableType::Float) {
					return pVariable->GetFloat();
				}
			}
		}
	}
	return 0.0f;
}

bool CommandArgsMgr::GetBoolForKey(const uint32_t key) {
	std::unordered_map<uint32_t, CommandArgEntry>::const_iterator cit = m_CommandArgsMap.find(key);
	if (cit != m_CommandArgsMap.cend()) {
		const CommandArgEntry & rEntry = cit->second;
		if (rEntry.GetType() == CommandArgEntryType::Variable) {
			const CommandArgVariable * pVariable = rEntry.GetVariable();
			if (pVariable != nullptr) {
				if (pVariable->GetType() == CommandArgVariableType::Boolean) {
					return pVariable->GetBool();
				}
			}
		}
	}
	return false;
}

const char * CommandArgsMgr::GetCStringForKey(const uint32_t key) {
	std::unordered_map<uint32_t, CommandArgEntry>::const_iterator cit = m_CommandArgsMap.find(key);
	if (cit != m_CommandArgsMap.cend()) {
		const CommandArgEntry & rEntry = cit->second;
		if (rEntry.GetType() == CommandArgEntryType::Variable) {
			const CommandArgVariable * pVariable = rEntry.GetVariable();
			if (pVariable != nullptr) {
				if (pVariable->GetType() == CommandArgVariableType::CString) {
					return pVariable->GetCString();
				}
			}
		}
	}
	return "\0";
}

void CommandArgsMgr::SetupAllCommandArgs(const int argc, char * argv[]) {
	if (argc >= 2) {
		const char * pFileToReadFrom = argv[1];
		std::ifstream inputFile(pFileToReadFrom);
		std::string curLine;
		while (std::getline(inputFile, curLine)) {
			const char * pCurArgFull = curLine.c_str();
			Execute(pCurArgFull);
		}
	}
}

int CommandArgsMgr::Execute(const char * pCommand) {
	if (!pCommand) {
		return 0;
	}
	// A valid argument is just giving the name of a flag which implies turning it on
	// so for instance an args file with:
	// g_enableVerboseLogging
	// would be the same as 
	// g_enableVerboseLogging 1
	const char * pSpaceCharPtr = FindFirstWhitespaceCharacterAfterFirstToken(pCommand);
	uint32_t key = 0;
	bool expectsFlag = false;
	if (!pSpaceCharPtr || !(*pSpaceCharPtr)) {
		key = HashCommandLineArg(pCommand);
		expectsFlag = true;
	}
	else {
		key = HashCommandLineArg_StartEnd(pCommand, pSpaceCharPtr);
	}
	// Expect variables/commands to be initliazed already
	std::unordered_map<uint32_t, CommandArgEntry>::iterator it;
	if (!FindCommandArgEntry(it, key)) {
		return 0;
	}
	char * pArgRHSString = FindFirstNonWhitespaceCharacter(pSpaceCharPtr);
	const CommandArgEntry & rEntry = it->second;
	const unsigned int entryType = rEntry.GetType();
	if (entryType == CommandArgEntryType::Function) {
		// Invoke the function pointer
		// CommandArgsParser is passed to make things easier
		const ConsoleCommandFunc pFunc = rEntry.GetFunction();
		if (!pFunc) {
			return 0;
		}
		CommandArgsParser argsParser;
		argsParser.InitWithArgs(pArgRHSString);
		return (*pFunc)(argsParser);
	}
	else if (entryType == CommandArgEntryType::Variable) {
		// Parse the variable and set the tagged variant appropriately
		CommandArgVariable * pCommandArgVariable = rEntry.GetVariable();
		if (!pCommandArgVariable) {
			return 0;
		}
		const CommandArgVariableType::Type nType = pCommandArgVariable->GetType();
		if (expectsFlag) {
			if (nType != CommandArgVariableType::Boolean) {
				return 0;
			}
			pCommandArgVariable->SetBool(true);
			return 1;
		} else {
			if (nType == CommandArgVariableType::Boolean) {
				bool boolToSet = false;
				if (CommandArgsParser::Parse_Bool(pArgRHSString, boolToSet)) {
					pCommandArgVariable->SetBool(boolToSet);
					return 1;
				}
			} else if (nType == CommandArgVariableType::Integer) {
				int intToSet = 0;
				if (CommandArgsParser::Parse_Integer(pArgRHSString, intToSet)) {
					pCommandArgVariable->SetInt(intToSet);
					return 1;
				}
			} else if (nType == CommandArgVariableType::Float) {
				float floatToSet = 0.0f;
				if (CommandArgsParser::Parse_Float(pArgRHSString, floatToSet)) {
					pCommandArgVariable->SetFloat(floatToSet);
					return 1;
				}
			} else if (nType == CommandArgVariableType::CString) {
				size_t stringLen = strlen(pArgRHSString);
				size_t stringSize = stringLen + 1;
				char * deepStringCopy = new char[stringSize];
				strcpy_s(deepStringCopy, stringSize, pArgRHSString);
				pCommandArgVariable->SetCString(deepStringCopy);
				pCommandArgVariable->SetFlags(pCommandArgVariable->GetFlags() | CommandArgVariableFlags::OwnsCString);
				return 1;
			}
		}
	}
	return 0;
}

bool CommandArgsMgr::FindCommandArgEntry(std::unordered_map<uint32_t, CommandArgEntry>::iterator & inOutIterator, const uint32_t key) {
	inOutIterator = m_CommandArgsMap.find(key);
	return inOutIterator != m_CommandArgsMap.end();
}


