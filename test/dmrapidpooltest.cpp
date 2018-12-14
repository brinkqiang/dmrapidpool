
#include "dmrapidpool.h"
#include <string>

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
    CPlayer* poPlayer = oPool.FetchObj("name");
    oPool.ReleaseObj(poPlayer);
    return 0;
}
