#pragma once

#include <string>

class LogFile 
{
public:
    LogFile();
    ~LogFile();
    void Print(std::string s);
    void Print(char c[]);
    std::ofstream& operator<<(const char *s);
    std::ofstream& operator<<(char c[]);
    std::ofstream& operator<<(std::string s);
    std::ofstream& operator<<(double d);
    std::ofstream& operator<<(float f);
    std::ofstream& operator<<(int i);
    std::ofstream& operator<<(bool b);
};


static LogFile debug;
