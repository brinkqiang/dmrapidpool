# dmrapidpool

Copyright (c) 2013-2018 brinkqiang (brink.qiang@gmail.com)

[dmrapidpool GitHub](https://github.com/brinkqiang/dmrapidpool)

## Build status
| [Linux][lin-link] | [Mac][mac-link] | [Windows][win-link] |
| :---------------: | :----------------: | :-----------------: |
| ![lin-badge]      | ![mac-badge]       | ![win-badge]        |

[lin-badge]: https://github.com/brinkqiang/dmrapidpool/workflows/linux/badge.svg "linux build status"
[lin-link]:  https://github.com/brinkqiang/dmrapidpool/actions/workflows/linux.yml "linux build status"
[mac-badge]: https://github.com/brinkqiang/dmrapidpool/workflows/mac/badge.svg "mac build status"
[mac-link]:  https://github.com/brinkqiang/dmrapidpool/actions/workflows/mac.yml "mac build status"
[win-badge]: https://github.com/brinkqiang/dmrapidpool/workflows/win/badge.svg "win build status"
[win-link]:  https://github.com/brinkqiang/dmrapidpool/actions/workflows/win.yml "win build status"

## Intro
dmrapidpool
```cpp
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
    virtual ~CPlayer()
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
    virtual ~CMonster()
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
        std::unique_ptr<CPlayer, DMPoolDeleter<CPlayer>> player(DMNew<CPlayer>("name"));
    }

    for (int i = 0; i < 10000; ++i)
    {
        std::unique_ptr<CPlayer, DMPoolDeleter<CPlayer>> player;
        player.reset(DMNew<CPlayer>("name"));
    }

    dmthreadpool pool;

    for (int i = 0; i < 100; ++i) {
        pool.commit([] {

            for (int j = 0; j < 100000; ++j)
            {
                std::unique_ptr<CPlayer, DMPoolDeleter<CPlayer>> player;
                player.reset(DMNew<CPlayer>("name"));
            }
        });
    }

    pool.wait_idle();
    std::cout << DMGetPoolInfo();

    return 0;
}

```
## Contacts

## Thanks
