# EON Audio Plugins

analog 회로 에뮬레이션 플러그인 모노레포. 두 개의 독립적인 플러그인을
포함합니다.

- **EE-1176** (`EE1176/`) — 1176 스타일 FET 컴프레서. 설계 목표는
  `Engineering the 1176 ... Blueprint.pdf` 참고 (WDF/behavioral 모델링,
  "British mode"(all-buttons-in) 등).
- **EE-1073** (`EE1073/`) — Neve 1073 스타일 마이크 프리앰프 + 3밴드 EQ
  채널스트립. 설계 목표는
  `From Circuit Simulation to DAW Integration ... Neve 1073 Emulation.pdf`
  참고 (인풋/아웃풋 트랜스포머 새추레이션·히스테리시스, Class-A 게인
  스테이지의 2kHz 배음 dip, 가변-Q 인터랙티브 미드밴드 EQ 등).

두 플러그인 모두 같은 패턴으로 구성됩니다: `<Plugin>/Shared/`의 DSP
코어를 JUCE VST3/AU/Standalone 플러그인(`<Plugin>/Source/`)과 Max for
Live 익스터널(`<Plugin>/Max/`)이 공유합니다. 각 `Shared/` 헤더의 DSP는
현재 비헤이비어럴 모델이며, 블루프린트가 목표로 하는 full WDF/MNA
회로 시뮬레이션은 아직 아닙니다.

## 1. VST3/AU 플러그인 빌드

### 요구 사항
- Xcode (커맨드라인 툴 포함)
- CMake 3.22+
- JUCE (서브모듈로 추가, VST3 SDK 내장됨 — 별도 다운로드 불필요)

### 설정

```bash
git submodule add https://github.com/juce-framework/JUCE ThirdParty/JUCE
git submodule update --init --recursive
```

### 빌드 (둘 다)

```bash
cmake -B build -GXcode
cmake --build build --config Release
```

특정 플러그인만 빌드하려면 옵션으로 끕니다:

```bash
cmake -B build -GXcode -DEE1073_BUILD_PLUGIN=OFF   # EE-1176만
cmake -B build -GXcode -DEE1176_BUILD_PLUGIN=OFF   # EE-1073만
```

빌드가 끝나면 `COPY_PLUGIN_AFTER_BUILD TRUE` 설정에 의해
`~/Library/Audio/Plug-Ins/VST3/EE-1176.vst3`,
`~/Library/Audio/Plug-Ins/VST3/EE-1073.vst3` 등에 자동 설치됩니다.
Ableton Live는 VST3를 네이티브로 스캔하므로, 별도 M4L 래퍼 없이도
Live에서 바로 사용할 수 있습니다.

### 검증
VST3 SDK가 제공하는 `VSTPluginTestHost`로 포맷 준수 여부를 확인하세요
(JUCE가 SDK를 내장하므로 `ThirdParty/JUCE/modules/juce_audio_processors/format_types/VST3_SDK/`
아래에서 빌드할 수 있습니다).

### 배포 (Notarization)
Apple 배포 시에는 Developer ID 서명 후 `.vst3`/`.component` 번들을
zip으로 묶어 notarization 서비스에 제출해야 합니다 (개별 dylib는
직접 제출 불가).

## 2. Ableton용 Max (Max for Live) 익스터널 빌드

`ee1176~`, `ee1073~`는 Max/MSP tilde 오브젝트로, Max for Live 디바이스
(.amxd) 안에서 인스턴스화해 사용합니다.

### 요구 사항
- Max/MSP 개발 환경 (Max SDK는 min-devkit에 서브모듈로 포함됨)
- Xcode
- CMake 3.22+

### 설정

```bash
git submodule add https://github.com/Cycling74/min-devkit ThirdParty/min-devkit
git submodule update --init --recursive
```

### 빌드 (둘 다, 플러그인은 끄고)

```bash
cmake -B build-max -GXcode \
  -DEE1176_BUILD_PLUGIN=OFF -DEE1073_BUILD_PLUGIN=OFF \
  -DEE1176_BUILD_MAX_EXTERNAL=ON -DEE1073_BUILD_MAX_EXTERNAL=ON
cmake --build build-max --config Release
```

빌드 결과물은 각각 `EE1176/Max/externals/ee1176~.mxo`,
`EE1073/Max/externals/ee1073~.mxo`에 생성됩니다 (x86_64 + arm64
universal binary로 검증됨). 이 폴더를 Max의 검색 경로에 추가하거나
(Max 환경설정 > File Preferences), `~/Documents/Max 9/Library/`로
복사한 뒤, Max for Live 디바이스 편집기에서 해당 오브젝트로 불러와
UI(다이얼/노브)를 attribute에 연결하고 `.amxd`로 저장하면 Ableton
Live에서 사용할 수 있습니다.

- `ee1176~` attribute: `input`, `output`, `attack`, `release`, `ratio`
- `ee1073~` attribute: `input`, `hpf_on`, `hpf_freq`, `low_freq`,
  `low_gain`, `mid_freq`, `mid_gain`, `high_gain`, `eq_on`, `output`,
  `phase_invert`

> 참고 1: JUCE 플러그인 쪽 UI에만 있는 **Power**(전체 바이패스) 스위치는
> `ee1073~`에는 없습니다 — JUCE `AudioProcessor::processBlock`에서만
> 처리되는 플러그인 레벨 기능이라, 공유 DSP 코어(`Ee1073ChannelStripCore.h`)
> 밖에 있습니다. Max 쪽에서 바이패스가 필요하면 패치에서 `ee1073~` 앞뒤로
> 신호를 직접 분기해서 처리하세요.
>
> 참고 2: JUCE 플러그인 UI의 HPF 노브는 `hpf_on`/`hpf_freq`를
> "Off/50/80/160/300Hz" 5단 choice 파라미터 하나로 합쳐서 보여주지만,
> 이는 UI 표현 방식일 뿐입니다. `ee1073~`와 공유 DSP 코어는 여전히
> `hpf_on`(bool)/`hpf_freq`(Hz) 두 값을 그대로 사용합니다.

프로젝트 구조는 min-devkit의 관례를 따릅니다: `Max/<object>_tilde/`
폴더 이름 자체가 오브젝트 이름이 되고, 그 안의
`<object>_tilde.cpp`가 `${PROJECT_NAME}.cpp`로 빌드됩니다. 새 Max
오브젝트를 추가하려면 같은 방식으로 `<Plugin>/Max/<object_name>/`
폴더를 만들고 `<Plugin>/Max/CMakeLists.txt`에
`add_subdirectory(<object_name>)`을 추가하세요.

## 디렉터리 구조

```
CMakeLists.txt                          최상위 빌드 (JUCE/min-devkit 셋업, 두 플러그인 서브디렉터리 진입)
EE1176/CMakeLists.txt                   EE-1176 플러그인/Max 타겟 정의
EE1176/Shared/Ee1176CompressorCore.h    EE-1176 DSP 코어 (플러그인·Max 익스터널 공용)
EE1176/Source/                          EE-1176 JUCE VST3/AU/Standalone 소스
EE1176/Max/ee1176_tilde/                ee1176~ 오브젝트 (min-devkit 관례)
EE1073/CMakeLists.txt                   EE-1073 플러그인/Max 타겟 정의
EE1073/Shared/Ee1073ChannelStripCore.h  EE-1073 DSP 코어 (플러그인·Max 익스터널 공용)
EE1073/Source/                          EE-1073 JUCE VST3/AU/Standalone 소스
EE1073/Max/ee1073_tilde/                ee1073~ 오브젝트 (min-devkit 관례)
ThirdParty/                             서브모듈 (JUCE, min-devkit) — git에는 미포함
```
