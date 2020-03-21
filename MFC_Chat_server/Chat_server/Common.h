#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <vector>

#define BUF_SIZE 1024

using namespace std;

inline void ErrMsg(const char* message, bool bExit = true)
{
	fputs(message, stderr);
	fputc('\n', stderr);

	if (bExit)
		exit(1);
}

inline void SplitIDPW(bool bslash, const char* str, string* ID, string* PW)
{
	int i = 0;

	while (str[i] != ',')
	{
		*ID += str[i];
		++i;
	}
	++i;

	if (PW)
	{
		if (bslash)
		{
			while (str[i] != '/')
			{
				*PW += str[i];
				++i;
			}
		}
		else
		{
			while (str[i] != '\0')
			{
				*PW += str[i];
				++i;
			}
		}
	}
}
