# EEVE1073

1176 스타일 FET 컴프레서 회로 에뮬레이션 프로젝트. 설계 목표는
`Engineering the 1176 ... Blueprint.pdf`를 참고하세요 (WDF/behavioral
모델링, "British mode"(all-buttons-in) 등).

이 저장소는 두 개의 빌드 타겟으로 구성됩니다.

- **VST3 / AU / Standalone 플러그인** — JUCE + CMake (`Source/`)
- **Max for Live 익스터널** — Cycling '74 min-devkit (`Max/`)

두 타겟은 `Shared/EeveCompressorCore.h`의 DSP 코어를 공유합니다.
`Shared/EeveCompressorCore.h`의 `processSample()`은 현재 자리표시용
비헤이비어럴 모델이며, 블루프린트에서 설명하는 FET 게인 리덕션·
program-dependent attack/release·British mode 회로 모델로 교체해야
합니다.

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

### 빌드

```bash
cmake -B build -GXcode -DEEVE_BUILD_PLUGIN=ON -DEEVE_BUILD_MAX_EXTERNAL=OFF
cmake --build build --config Release
```

빌드가 끝나면 `COPY_PLUGIN_AFTER_BUILD TRUE` 설정에 의해
`~/Library/Audio/Plug-Ins/VST3/EEVE1073.vst3` 및
`~/Library/Audio/Plug-Ins/Components/EEVE1073.component`에 자동 설치됩니다.
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

`eeve1073~`은 Max/MSP tilde 오브젝트로, Max for Live 디바이스
(.amxd) 안에서 `eeve1073~` 오브젝트로 인스턴스화해 사용합니다.

### 요구 사항
- Max/MSP 개발 환경 (Max SDK는 min-devkit에 서브모듈로 포함됨)
- Xcode
- CMake 3.22+

### 설정

```bash
git submodule add --recursive https://github.com/Cycling74/min-devkit ThirdParty/min-devkit
git submodule update --init --recursive
```

### 빌드

```bash
cmake -B build-max -GXcode -DEEVE_BUILD_PLUGIN=OFF -DEEVE_BUILD_MAX_EXTERNAL=ON
cmake --build build-max --config Release
```

빌드 결과물 `eeve1073~.mxo`를 Max의 검색 경로(예:
`~/Documents/Max 9/Library/`)에 복사한 뒤, Max for Live 디바이스
편집기에서 `eeve1073~ [signal in] [signal out]` 오브젝트로 불러와
UI(다이얼/노브)를 attribute(`input`, `output`, `attack`, `release`,
`ratio`)에 연결하고 `.amxd`로 저장하면 Ableton Live에서 사용할 수
있습니다.

> 참고: min-devkit은 자체 프로젝트 레이아웃(`source/projects/...`)을
> 기준으로 패키징 스크립트가 동작합니다. `Max/CMakeLists.txt`는
> 최소 스캐폴드이며, min-devkit을 실제로 추가한 뒤 그 저장소의
> `CMakeLists.txt` 관례에 맞춰 include 경로/패키징 단계를 검증·조정하세요.

## 디렉터리 구조

```
CMakeLists.txt              최상위 빌드 (플러그인/Max 타겟 스위치)
Shared/EeveCompressorCore.h 공유 DSP 코어 (플러그인·Max 익스터널 공용)
Source/                     JUCE VST3/AU/Standalone 플러그인 소스
Max/                        Max for Live 익스터널 (min-devkit)
ThirdParty/                 서브모듈 (JUCE, min-devkit) — git에는 미포함
```
