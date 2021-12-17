
#include "dmrapidpool.h"
#include <string>
#include <iostream>

class CPlayer
{
public:
    CPlayer(const std::string& name)
        : m_strName(name)
    {

    }
    const std::string& GetName()
    {
        return m_strName;
    }
private:
    std::string m_strName;
};

class CMonster
{
public:
    CMonster(const std::string& name)
        : m_strName(name)
    {

    }
    const std::string& GetName()
    {
        return m_strName;
    }
private:
    std::string m_strName;
};

int main(int argc, char* argv[]) {

    for (int i = 0; i < 10000; ++i)
    {
        CPlayer* poPlayer = DMNew<CPlayer>("name");

        DMDelete(poPlayer);
    }
    for (int i = 0; i < 10000; ++i)
    {
        CMonster* poMonster = DMNew<CMonster>("name");

        DMDelete(poMonster);
    }

    std::cout << DMPrintPool();

    return 0;
}
