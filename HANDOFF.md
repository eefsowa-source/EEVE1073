# EON Audio Plugins — 작업 인계 문서

다른 채팅 세션에서 이어서 작업할 때 참고하는 현황 요약. 프로젝트 자체 사용법은
[README.md](README.md) 참고 (빌드 명령, 디렉터리 구조, Max attribute 목록 등).
이 문서는 "지금까지 뭘 했고, 뭐가 남았는지"에 집중합니다.

## 프로젝트 개요

`/Users/sungha/Desktop/EON LLM wiki/EON Audio Plugin/EEVE1073` 아래의
모노레포. 독립적인 analog 하드웨어 에뮬레이션 플러그인 두 개:

- **EE-1176** (`EE1176/`) — 1176 스타일 FET 컴프레서
- **EE-1073** (`EE1073/`) — Neve 1073 스타일 마이크 프리앰프 + 3밴드 EQ

각 플러그인은 `<Plugin>/Shared/`의 순수 C++ DSP 코어를 JUCE VST3/AU/Standalone
(`<Plugin>/Source/`)과 Max for Live 익스터널(`<Plugin>/Max/`)이 공유하는 구조.
git 저장소는 이 폴더 자체가 루트(홈 디렉터리 전체를 잡고 있던 git과는 별개로
분리해서 `git init` 했음 — 자세한 경위는 커밋 로그 초반 참고).

## 지금까지 한 일 (시간순 요약)

1. PDF 블루프린트 2개(1176 회로 분석, Neve 1073 회로 분석) 기반으로 스캐폴딩
2. JUCE + CMake로 VST3/AU/Standalone 빌드 구성, min-devkit으로 Max 익스터널 빌드 구성
3. EE-1176 DSP 코어: 피드백 토폴로지 디텍터, 레이쇼별 내부 threshold 이동,
   soft-knee, British mode(all-buttons-in) 구현
4. `EEVE1073`(원래 이름, 컴프레서였음)를 **EE-1176**으로 리네임하고,
   실제 Neve 1073 블루프린트에 맞는 **EE-1073**(프리앰프+EQ)을 신규 설계
   → 모노레포로 분리 (`EE1176/`, `EE1073/` 서브프로젝트, 최상위 CMake가 오케스트레이션)
5. EE-1073 DSP 코어: 인풋/아웃풋 트랜스포머 새추레이션(히스테리시스 근사),
   Class-A 스테이지(2kHz 배음 dip), HPF, 3밴드 EQ(가변-Q 미드밴드)
6. 두 플러그인 다 실제 JUCE 빌드 검증 (초반엔 Max만 검증했었음 — C 컴파일러
   누락, BUNDLE_ID 공백 문제 등 발견해서 고침)
7. **EE-1073 UI 디자인**: 커스텀 `LookAndFeel` (Ee1073LookAndFeel) →
   빈티지 콘솔 → **우드프레임 세로 랙스트립**(레퍼런스: UAD 1073 스킨 사진)으로
   방향 전환. 다크 EQ 패널(왼쪽) + 실버 페이더 패널(오른쪽), 세로 Output 페이더,
   HPF를 Off 포함 5단 choice 로터리로 통합, Phase invert·Power(바이패스)
   파라미터 신규 추가
8. **EE-1176 UI 디자인**: 처음엔 오리지널 1176LN(4버튼 레이쇼+GR LED바)으로
   설계 → 사용자가 준 사진이 실제로는 **UA 1176 Rack Mount**(COMP RATIO
   로터리+VERNIER, 아날로그 니들 VU미터, 블루그레이 패널)라서 재설계 →
   그다음 사진이 다시 **오리지널 1176LN Classic**(INPUT/OUTPUT 큰 노브,
   ATTACK 위에 RELEASE, 작은 RATIO 노브, VU미터)이라 또 재설계, 색은
   **민트그린 계열**로 변경. Power 바이패스, ratioTrim/attackTrimMs(Vernier용,
   현재 UI엔 노출 안 됨 — 파라미터만 존재) 추가
9. EE-1073 노브 전체 확대(더 두껍게) — 캡 비율·스트로크 두께·전체 사이즈 업
10. **4배 오버샘플링**: JUCE 플러그인 양쪽에 `juce::dsp::Oversampling`
    (half-band IIR 2단) 적용, DSP 코어는 `sampleRate*4`로 prepare
11. **인풋 레벨에 비례하는 짝수 배음**: 두 코어의 새추레이션 함수가
    `asymmetry` 인자를 받아서 Input 노브를 올릴수록 비대칭(짝수배음)이 커지게 함
12. (사용자가 직접/다른 세션에서 커밋 `f20a25c`) **Max 익스터널에도 4배
    오버샘플링 적용** — `Shared/EeFourTimesOversampler.h`라는 경량 선형보간
    기반 업/다운샘플러 신규 추가, `ee1176~`/`ee1073~` 둘 다 적용. JUCE 쪽은
    half-band IIR, Max 쪽은 선형보간 방식으로 구현 방식이 다름 (Max의
    per-sample 콜백 API 특성상 블록 단위 IIR 오버샘플링을 그대로 못 씀)

## 현재 파라미터 목록

### EE-1176
`input`, `output`, `attack`, `release`, `ratio`(5단: 4:1/8:1/12:1/20:1/All),
`ratioTrim`(Vernier, UI 노출 안 됨), `attackTrim`(Vernier, UI 노출 안 됨), `power`

### EE-1073
`input`, `hpfFreq`(5단: Off/50/80/160/300Hz choice), `lowFreq`(4단), `lowGain`,
`midFreq`(6단), `midGain`, `highGain`(12kHz 고정), `eqOn`, `output`,
`phaseInvert`, `power`

Max 익스터널(`ee1073~`)은 `hpf_on`(bool)+`hpf_freq`(Hz)를 따로 받음 — JUCE
쪽만 UI 편의상 Off 포함 choice로 합쳤을 뿐, 내부 `Parameters` 구조체는 둘 다
그대로 `hpfEnabled`+`hpfFreqHz`. `ee1073~`엔 `power`(바이패스)가 없음 —
JUCE `AudioProcessor::processBlock` 레벨 기능이라 공유 DSP 코어 밖에 있음.

## 알려진 갭 / 미해결

- **둘 다 여전히 비헤이비어럴 모델** — 블루프린트가 목표로 한 full WDF/MNA
  회로 시뮬레이션은 아님. 가장 큰 남은 작업.
- **실제 시각 확인을 한 번도 못 함** — 이 세션이 작업한 환경(코딩 에이전트
  샌드박스)은 헤드리스라 Standalone 앱을 띄워도 창이 실제로 렌더링되지
  않아서(System Events로 확인함) 스크린샷 검증이 불가능했음. **다음 세션에서
  꼭 실제 데스크톱에서 열어서 눈으로 확인 필요** — 레이아웃 겹침, 색상,
  텍스트 잘림 등.
- **실제 청음 테스트 안 함** — 지금까지 검증은 전부 "컴파일 됨 / NaN·Inf
  없음 / 짝수배음이 입력에 비례해서 커짐(수학적으로)" 수준. EQ 커브가
  실제로 Neve 1073 느낌이 나는지, 컴프레션이 1176답게 느껴지는지는 안 들어봄.
- **프리셋 없음**
- **배포용 코드사이닝(Developer ID)·notarization 안 함** — 지금은 ad-hoc
  서명이라 로컬 개발용으로만 문제없이 동작. 남에게 배포하려면 README의
  notarization 절차 진행 필요.
- **자동 테스트가 저장소에 정식으로 없음** — 이 세션 내내 `/tmp`에 즉석
  스크립트로 DSP sanity 테스트를 짜서 돌렸는데(NaN/Inf 체크, 무음 DC 체크,
  배음 스케일링 체크 등), 세션이 끝나면 다 날아감. 재사용하려면 저장소
  안(`tests/` 같은 곳)에 정식으로 옮겨야 함.
- **CI 없음** (GitHub Actions 등)
- **Max 익스터널 오버샘플러는 JUCE 쪽과 다른(더 단순한) 구현** —
  `EeFourTimesOversampler`는 선형보간 업샘플 + 박스필터 다운샘플이라
  진짜 half-band 필터 대비 앨리어싱 억제력이 약함. Max의 per-sample
  API 제약 때문에 나온 타협이라 문서화는 해뒀지만, 더 정확하게 하려면
  Max 쪽에서도 진짜 halfband FIR/IIR 오버샘플러로 교체 필요.
- **`.mxo`/`.vst3`는 ad-hoc 서명만 되어 있고 Gatekeeper 배포용 서명 아님**

## 빌드 관련 주의사항 (이 환경에서 겪은 것)

- 이 세션에서 `cmake -B build -G Xcode` / `cmake --build`가 종종
  `accessing build database ".../XCBuildData/build.db": disk I/O error`로
  실패했음 — **코드 문제 아니고 환경 쪽 일시적 문제**. 그냥 같은 명령
  재시도하면 대부분 해결됨. 안 되면 `xcodebuild -project
  ThirdParty/JUCE/tools/JUCE.xcodeproj -target juceaide -configuration Custom`로
  juceaide를 먼저 직접 빌드한 다음 cmake configure를 재시도하면 됨.
- `ThirdParty/JUCE`, `ThirdParty/min-devkit`는 git submodule이라
  `git submodule update --init --recursive` 필요 (README 참고).

## 다음에 뭘 하면 좋을지

우선순위 추천 순서:
1. 실제 데스크톱에서 두 플러그인 UI 열어서 육안 확인 + 리사이즈/겹침 확인
2. Ableton Live 또는 다른 DAW에서 실제로 오디오 걸어서 들어보기 (EQ 커브,
   컴프레션 느낌, 오버샘플링 레이턴시로 인한 PDC 이상 유무)
3. 필요하면 Max 쪽 오버샘플러를 진짜 halfband 필터로 업그레이드
4. 프리셋 시스템
5. 배포 준비 (코드사이닝/notarization)
