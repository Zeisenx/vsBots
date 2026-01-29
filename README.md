# vsBots

vsBots는 CS2Fixes에서 포크되어 기반으로 제작되는 CS2 vsBots 서버 플러그인입니다.

이 플러그인은 개인 서버 내의 사용 목적으로 제작되고 있기 때문에, 다른 서버 개발자나 운영자에게 적합하지 않습니다. 허나, 이 리포지토리가 퍼블릭인 이유는 이 플러그인과 연관된 다른 플러그인들을 제작하는 개발자들을 위해 개방했습니다.

이 플러그인은 봇의 기능 향상 및 버그 수정 등 봇과 관련된 개선 사항 패치가 많습니다. 패치 부분을 요약하면 다음과 같습니다.
- CT 봇이 M4를 구매하지 않는 버그가 수정되어 있습니다.
- BotProfileManager를 이용하여 botprofile.db 수정 없이 동적으로 즉시 봇 프로필을 추가할수 있습니다.
- 동적으로 서버 내에 있는 봇의 프로필을 수정할 수 있습니다.
- 워크샵 맵을 사용할 수 있는 맵 순환 시스템이 탑재되어 있습니다.
- 봇의 에임 트래킹이 CS:S와 유사하게 작동하도록 패치되어 있습니다.
- 봇이 에임이 적의 사타구니만 노리지 않습니다.

초기에는 리눅스 서버에서 동작하는것을 기준으로 플러그인이 작성되었지만, CS2에서 리눅스 서버가 플레이가 불가능할정도로 불안정한 부분이 많아 윈도우를 기준으로 제작됩니다. 따라서 동봉된 게임 데이터의 시그니처나 오프셋이 리눅스의 경우 누락되어 있을 수 있습니다.

<<<<<<< HEAD
기능 개선, 최적화 등의 PR은 환영하지만 개인 서버 내의 사용이 목적이기 때문에 밸런스 조정 등의 값 변경과 관련된 PR은 불허하며 기능/컨텐츠 추가는 불필요하다고 판단되는경우 해당 PR을 병합하지 않습니다.
=======
## Why is this all one plugin? Why "CS2Fixes"?

Reimplementing all these features as standalone plugins would duplicate quite a lot of code between each. Metamod is not much more than a loader & hook manager, so many common modding features need a fair bit of boilerplate to work with. And since our primary goal is developing CS2Fixes for all zombie escape servers, there is not necessarily a drawback to distributing our work in this form at the moment.

The CS2Fixes name comes from the CSSFixes and CSGOFixes projects, which were primarily aimed at low-level bug fixes and improvements for their respective games. Long term, we see this plugin slimming down and becoming more similar to them. Since as the CS2 modding scene matures, common things like an admin system and RTV become more feasible in source2mod or a similar modding platform.

## Compilation

```
git clone https://github.com/Source2ZE/CS2Fixes/ && cd CS2Fixes
git submodule update --init --recursive
```
### Docker (easiest)

Requires Docker to be installed. Produces Linux builds only.

```
docker compose up
```

Copy the contents of `dockerbuild/package/cs2/` to your server's `game/csgo/` directory.

### Manual

#### Requirements
- [Metamod:Source](https://github.com/alliedmodders/metamod-source)
- [AMBuild](https://wiki.alliedmods.net/Ambuild)

#### Linux
```bash
export MMSOURCE_DEV=/path/to/metamod
export HL2SDKCS2=/path/to/sdk/submodule

mkdir build && cd build
python3 ../configure.py --enable-optimize --sdks cs2
ambuild
```

#### Windows

Make sure to run in "x64 Native Tools Command Prompt for VS". Doing an initial build here is also required to setup the protobuf headers for Visual Studio to reference.

```bash
set MMSOURCE_DEV=\path\to\metamod
set HL2SDKCS2=\path\to\sdk\submodule

mkdir build && cd build
py ../configure.py --enable-optimize --sdks cs2
ambuild
```

Copy the contents of `build/package/cs2/` to your server's `game/csgo/` directory.
>>>>>>> upstream
