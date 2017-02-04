/*Copyright 2016 Darren Chan*/
#include "stdafx.h"
#include <stdlib.h>
#include <windows.h>
#include <Lmcons.h>
#include <iostream>
#include <string>

enum Action {
	EVAL,
	CHDIR,
	EXIT
};

enum Color {
	BLACK = 0,
	DARKBLUE = FOREGROUND_BLUE,
	DARKGREEN = FOREGROUND_GREEN,
	DARKCYAN = FOREGROUND_GREEN | FOREGROUND_BLUE,
	DARKRED = FOREGROUND_RED,
	DARKMAGENTA = FOREGROUND_RED | FOREGROUND_BLUE,
	DARKYELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
	DARKGRAY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	GRAY = FOREGROUND_INTENSITY,
	BLUE = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
	GREEN = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
	CYAN = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
	RED = FOREGROUND_INTENSITY | FOREGROUND_RED,
	MAGENTA = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
	YELLOW = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
	WHITE = FOREGROUND_INTENSITY |
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
};

struct Computer {
	std::string userName;
	std::string name;
};

class Terminal {
	HANDLE stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO scrn;
public:
	void setColor(Color color) {
		SetConsoleTextAttribute(stdoutHandle, color);
	}
	void resetColor() {
		SetConsoleTextAttribute(stdoutHandle, scrn.wAttributes);
	}
	Terminal() {
		GetConsoleScreenBufferInfo(stdoutHandle, &scrn);
	}
};

Computer computer;
Terminal terminal;

std::string toString(TCHAR* str) {
	#ifndef UNICODE
		return str;
	#else
		std::wstring tmp = str;
		return std::string(tmp.begin(), tmp.end());
	#endif
}

bool preventBreak(DWORD keyType) {
	switch (keyType) {
	case CTRL_C_EVENT: {
		std::cin.clear();
		return true;
	} break;
	case CTRL_BREAK_EVENT: {
		std::cin.clear();
		return true;
	} break;
	default: {
		return false;
	}
	}
}

void init() {
	TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size1 = static_cast<DWORD>(MAX_COMPUTERNAME_LENGTH + 1);
	GetComputerName(computerName, &size1);
	computer.name = toString(computerName);
	TCHAR userName[UNLEN + 1];
	DWORD size2 = static_cast<DWORD>(UNLEN + 1);
	GetUserName(userName, &size2);
	computer.userName = toString(userName);
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)preventBreak, TRUE);
	std::cout << std::endl;
}

void getDir(bool internalCmd) {
	TCHAR buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	std::string dir = toString(buffer);
	for (int i = 0, ii = dir.size(); i != ii; i++) {
		if (dir[i] == '\\') {
			dir[i] = '/';
		}
	}
	if (internalCmd) {
		std::cout << dir << std::endl;
	} else {
		terminal.setColor(DARKCYAN);
		std::cout << dir << " $ " << std::flush;
		terminal.resetColor();
	}
}

void prefix() {
	terminal.setColor(GREEN);
	std::cout << computer.userName + "@" + computer.name + " "
		<< std::flush;
	terminal.resetColor();
}

std::string getCommand() {
	prefix();
	getDir(false);
	std::string cmd;
	std::getline(std::cin, cmd);
	int offset = 0;
	for (int ii = cmd.size(); offset != ii; offset++) {
		if (cmd[offset] != ' ') {
			break;
		}
	}
	cmd = cmd.substr(offset, cmd.size() - offset);
	return cmd;
}

Action checkInput(std::string cmd) {
	Action action = EVAL;
	if (cmd == "exit") {
		action = EXIT;
	}
	else if (cmd.size() > 2 && cmd.substr(0, 3) == "cd " ||
		cmd == "cd") {
		action = CHDIR;
	}
	return action;
}

int cmd() {
	std::string _cmd = getCommand();
	char* cmd = const_cast<char*>(_cmd.c_str());
	Action action = checkInput(_cmd);
	switch (action) {
	case EVAL: {
		std::system(cmd);
		std::cout << std::endl;
	} break;
	case CHDIR: {
		std::string change = _cmd.substr(2, _cmd.size() - 2);
		int offset = 0;
		for (int ii = change.size(); offset != ii; offset++) {
			if (change[offset] != ' ' &&
				change[offset] != '\'' &&
				change[offset] != '"') {
				break;
			}
		}
		change = change.substr(offset, change.size() - offset);
		if (change.size() == 0) {
			getDir(true);
		} else {
			if (change.size() > 1 &&
				change.substr(0, 2) == "/d" || change.substr(0, 2) == "-d") {
				change = change.substr(2, change.size() - 2);
			}
			while (change.size() != 0 && (
				change.front() == '"' ||
				change.front() == ' ' ||
				change.front() == '\'')) {
				change = change.erase(0, 1);
			}
			while (change.size() != 0 && (
				change.back() == '"' ||
				change.back() == ' ' ||
				change.back() == '\'')) {
				change.pop_back();
			}
			if (change.size() == 2 && change[1] == ':') {
				change.push_back('/');
			}
			std::wstring nextDir = std::wstring(change.begin(), change.end());
			BOOL success = SetCurrentDirectory(nextDir.c_str());
			if (!success) {
				std::cout << "Failed to change directory: error "
					<< GetLastError() << std::endl;
			}
		}
		std::cout << std::endl;
	} break;
	case EXIT: {
		return 0;
	} break;
	}
	return 1;
}

int main() {
	init();
	int result = 1;
	do {
		result = cmd();
	} while (result != 0);
	return 0;
}
