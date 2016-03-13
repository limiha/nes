#pragma once

class Util
{
public:
    static void WriteBytes(bool val, std::ofstream& ofs);
    static void WriteBytes(u8 val, std::ofstream& ofs);
    static void WriteBytes(u16 val, std::ofstream& ofs);
    static void WriteBytes(u32 val, std::ofstream& ofs);
    static void WriteBytes(i32 val, std::ofstream& ofs);

    static void ReadBytes(bool& val, std::ifstream& ifs);
    static void ReadBytes(u8& val, std::ifstream& ifs);
    static void ReadBytes(u16& val, std::ifstream& ifs);
    static void ReadBytes(u32& val, std::ifstream& ifs);
    static void ReadBytes(i32& val, std::ifstream& ifs);
};