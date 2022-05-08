#include <string>
#include <iostream>

#include "dmrapidpool.h"
#include "dmthreadpool.h"

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

    dmthreadpool pool;

    for (int i = 0; i < 100; ++i) {
        pool.commit([] {

            for (int j = 0; j < 100000; ++j)
            {
                CPlayer* poPlayer = DMNew<CPlayer>("name");

                DMDelete(poPlayer);
            }
        });
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << DMGetPoolInfo();

    return 0;
}
