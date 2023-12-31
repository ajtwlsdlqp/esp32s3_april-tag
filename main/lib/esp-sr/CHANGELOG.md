# Change log for esp-sr

## Unreleased

## 1.1.0
- Support esp32c3 for Chinese TTS
- Update document of ESP-SR
- Add ESP-SR into Espressif component manager

## 1.0.0
- Add wakenet8 & wakenet9
- Add multinet5 to support English and Chinese speech command recognition
- Remove wakenet7
- Add AFE pipeline for voice communication

## 0.8.0
- support ESP32S3 chip
- add wakenet7 & update wakenet5 to support multi-channel detection
- remove wakenet6
- add AFE pipeline for speech recognition

## 0.7.0
- add chinese tts
- update noise suppression v2
- update AEC v3

## 0.6.0
- update multinet_cn_1.4 and add CONTINUOUS RECOGNITION mode
- improve hilexin wakeNet5X3 model(v5)
- support IDFv4.0 build system
- replace MAP algorithm with MASE(Mic Array Speech Enhancement) algorithm v1.0

## 0.5.0
- add multinet1 English model v1.0
- update multinet1 Chinese model v2.0
- add Mic Array Processing(MAP) algorithm
- Fix the bug of parsing speech command
- fix the bug of decoder

## 0.3.0
- add wakenet6
- support cmake
- add free wake word: hi jeson
- update wakenet5X3 wake word model(v2)

## 0.2.0
- add acoustic algorithm, include AEC, AGC, VAD ,NS
- add wakenet5X2 and wakenet5X3

## 0.1.0
- Initial commit, include wakenet4,wakenet5 and multinet1_cni
