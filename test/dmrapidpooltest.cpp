
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

int main( int argc, char* argv[] ) {
    CDynamicRapidPool<CPlayer, 1000, 1000> oPool;

    for ( int i = 0; i < 10000; ++i)
    {
        CPlayer* poPlayer = oPool.FetchObj("name");       
    }

    std::cout << CDMRapidFactory::Instance()->Print();

    return 0;
}
