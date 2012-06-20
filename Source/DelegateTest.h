#pragma once
class DelegateTest
{
public:
    DelegateTest(void) {}
    ~DelegateTest(void) {}

    void Test1(std::string str) { OutputDebugStringA(("1: " + str).c_str()); }
};

class DelegateTest2
{
public:
    DelegateTest2(void) {}
    ~DelegateTest2(void) {}

    void Test2(std::string str) { OutputDebugStringA(("2: " + str).c_str()); }
};

