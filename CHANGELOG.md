# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
<!-- markdownlint-disable MD024 -->

## [2.1.1] - 2024-10-06

### <!-- 3 --> 🗑️ Removed

- Enable building IRQ-based examples by \@2bndy5 in [#36](https://github.com/nRF24/RF24Gateway/pull/36)

### <!-- 4 --> 🛠️ Fixed

- Fix build warnings for examples by \@2bndy5 in [#37](https://github.com/nRF24/RF24Gateway/pull/37)

### <!-- 9 --> 🗨️ Changed

- Change `DEBUG_LEVEL` into `RF24GATEWAY_DEBUG_LEVEL` by \@2bndy5 in [#34](https://github.com/nRF24/RF24Gateway/pull/34)
- Update doxygen CSS and use v1.11.0 by \@2bndy5 in [#38](https://github.com/nRF24/RF24Gateway/pull/38)
- Warnings if unable to config interface by \@TMRh20 in [#39](https://github.com/nRF24/RF24Gateway/pull/39)

[2.1.1]: https://github.com/nRF24/RF24Gateway/compare/v2.1.0...v2.1.1

Full commit diff: [`v2.1.0...v2.1.1`][2.1.1]

## [2.1.0] - 2024-04-06

### <!-- 3 --> 🗑️ Removed

- Simplify interrupt handling by \@TMRh20 in [#32](https://github.com/nRF24/RF24Gateway/pull/32)

### <!-- 8 --> 📝 Documentation

- Minor doc updates by \@2bndy5 in [#33](https://github.com/nRF24/RF24Gateway/pull/33)

### <!-- 9 --> 🗨️ Changed

- Comment out setStaticAddress by \@TMRh20 in [#27](https://github.com/nRF24/RF24Gateway/pull/27)
- Update clang format by \@2bndy5 in [#30](https://github.com/nRF24/RF24Gateway/pull/30)

[2.1.0]: https://github.com/nRF24/RF24Gateway/compare/v2.0.0...v2.1.0

Full commit diff: [`v2.0.0...v2.1.0`][2.1.0]

## [2.0.0] - 2023-06-21

### <!-- 8 --> 📝 Documentation

- Manually trigger docs CI by \@2bndy5 in [`308ced6`](https://github.com/nRF24/RF24Gateway/commit/308ced6994f70b4332569508d41caff0614ed0cc)
- Doc updates and reusable CI by \@2bndy5 in [#24](https://github.com/nRF24/RF24Gateway/pull/24)
- Template layer for radio HW abstraction by \@2bndy5 in [#26](https://github.com/nRF24/RF24Gateway/pull/26)

[2.0.0]: https://github.com/nRF24/RF24Gateway/compare/v1.0.4...v2.0.0

Full commit diff: [`v1.0.4...v2.0.0`][2.0.0]

## [1.0.4] - 2022-06-24

### <!-- 1 --> 🚀 Added

- Add user packets counter by \@TMRh20 in [`6cccf88`](https://github.com/nRF24/RF24Gateway/commit/6cccf8857e368c0c719300d065b84fedfaa044ea)
- Add label by \@TMRh20 in [`28002fb`](https://github.com/nRF24/RF24Gateway/commit/28002fba36011e14350eb68d216fd9554a8618a8)
- Add to interrupt example by \@TMRh20 in [`6c11bff`](https://github.com/nRF24/RF24Gateway/commit/6c11bffd0abc97b640460d7de59f041dc0cf021f)
- Add images by \@TMRh20 in [`e656ba2`](https://github.com/nRF24/RF24Gateway/commit/e656ba23fbc74d1ca39f5efde34103c53daaeebe)
- Add better image by \@TMRh20 in [`ec0da50`](https://github.com/nRF24/RF24Gateway/commit/ec0da50283b10aa73e0bbbd419acb0e306df24d4)
- Merge pull request \#22 from nRF24/topology by \@TMRh20 in [#22](https://github.com/nRF24/RF24Gateway/pull/22)
- Add clang-format config by \@2bndy5 in [`c9ac4d9`](https://github.com/nRF24/RF24Gateway/commit/c9ac4d96cc0f94ef543501b034ca8367f3322de2)

### <!-- 9 --> 🗨️ Changed

- Adjust RF24Pad by \@TMRh20 in [`75346fc`](https://github.com/nRF24/RF24Gateway/commit/75346fcc7daf31398a1d75a7b5a12873ac3453c9)
- [cmake] link examples to pigpio if detected by \@2bndy5 in [`9bf626f`](https://github.com/nRF24/RF24Gateway/commit/9bf626f2ef1bb7e3667536e2b98d69a99d3216fb)
- Forgot about the sniffer example by \@2bndy5 in [`d8948f0`](https://github.com/nRF24/RF24Gateway/commit/d8948f00087b5f0e2f2ec1b4532b569fd3470eb5)
- [cmake] build examples w/ a specified driver by \@2bndy5 in [`ae80da8`](https://github.com/nRF24/RF24Gateway/commit/ae80da885d8b4ae9399c33af47502044a13a3e20)
- Skip build irq examples for mraa/wiringPi by \@2bndy5 in [`9561200`](https://github.com/nRF24/RF24Gateway/commit/9561200f58a7df8bb92444ec154403d17465684b)
- -DRF24_NO_IRQ when no pigpio while building lib by \@2bndy5 in [`01a6164`](https://github.com/nRF24/RF24Gateway/commit/01a616473fa674136d5772f02dc715cc55a461f6)
- Allow RF24_NO_IRQ for building examples by \@2bndy5 in [`0f8711f`](https://github.com/nRF24/RF24Gateway/commit/0f8711f0e267c0c74d0cfb73dc9aca1b3d098c77)
- Only build irq examples when able by \@2bndy5 in [`67b4f70`](https://github.com/nRF24/RF24Gateway/commit/67b4f701163a9513e935a3ff976f7b5af6c52cf5)
- Merge branch 'pigpio-support' by \@TMRh20 in [`3f06838`](https://github.com/nRF24/RF24Gateway/commit/3f068384dd7dae2c74a0932269b7b8a55e3e82e9)
- Linux CI use master branch of RF24 by \@2bndy5 in [`88fb4c1`](https://github.com/nRF24/RF24Gateway/commit/88fb4c1e1e825b642cbd395770733236c446a118)
- Draw topology in ncurses example by \@TMRh20 in [`3abe2b4`](https://github.com/nRF24/RF24Gateway/commit/3abe2b4b69eba4a5fc302eef410de5021b3de187)
- Word wrapping by \@TMRh20 in [`490d832`](https://github.com/nRF24/RF24Gateway/commit/490d832013837ee6ff86f297b1bb603f6d86a75a)
- Make CI use clang-format by \@2bndy5 in [`6fde253`](https://github.com/nRF24/RF24Gateway/commit/6fde253efde0dccc90bae67964b901167d6be92e)
- Ran clang-format on al C++ sources by \@2bndy5 in [`2445bd1`](https://github.com/nRF24/RF24Gateway/commit/2445bd1b5d77125d490e5fbb3ee762d3ef44c319)
- Ignore long inline conditions by \@2bndy5 in [`5a4e8f7`](https://github.com/nRF24/RF24Gateway/commit/5a4e8f76f7405ef5a474d01d2eef8a971736b660)
- Merge pull request \#23 from nRF24/clang-format by \@TMRh20 in [#23](https://github.com/nRF24/RF24Gateway/pull/23)

[1.0.4]: https://github.com/nRF24/RF24Gateway/compare/v1.0.3...v1.0.4

Full commit diff: [`v1.0.3...v1.0.4`][1.0.4]

## [1.0.3] - 2021-11-21

### <!-- 1 --> 🚀 Added

- Add License; CMake builds this fine locally by \@2bndy5 in [`0b4542b`](https://github.com/nRF24/RF24Gateway/commit/0b4542bb842258c64987039c9acf1768498c5258)
- Add linux_build CI by \@2bndy5 in [`0da85b4`](https://github.com/nRF24/RF24Gateway/commit/0da85b42781732e153afc14e4746164adea0e2c5)
- Merge pull request \#18 from nRF24/CMake-4-Linux by \@TMRh20 in [#18](https://github.com/nRF24/RF24Gateway/pull/18)

### <!-- 3 --> 🗑️ Removed

- That worked; remove debug prompts by \@2bndy5 in [`ee7b259`](https://github.com/nRF24/RF24Gateway/commit/ee7b259e01c71e6460050853ca2b7c1178191a27)
- [cmake] remove old STRIP cmds by \@2bndy5 in [`d0d12d7`](https://github.com/nRF24/RF24Gateway/commit/d0d12d7edd1b4b18fd2efd2ca3fa1368864ef88a)

### <!-- 4 --> 🛠️ Fixed

- Fix examples compilation by \@TMRh20 in [`1cefcdb`](https://github.com/nRF24/RF24Gateway/commit/1cefcdb1fed6af88b47fbb5cf735f7dd74a3a9bf)

### <!-- 8 --> 📝 Documentation

- [CI] use RF24* master; save doc builds as artifact by \@2bndy5 in [`f35b293`](https://github.com/nRF24/RF24Gateway/commit/f35b2930032c8351c5d28ecc5a00ec87682967ae)

### <!-- 9 --> 🗨️ Changed

- Oops! c-n-p much? by \@2bndy5 in [`8c50a95`](https://github.com/nRF24/RF24Gateway/commit/8c50a95f8c57907520e05381812d709d92fbfadd)
- [LinuxCI] forgot the "with:" line by \@2bndy5 in [`9df2a8f`](https://github.com/nRF24/RF24Gateway/commit/9df2a8ff4df681c7e418be7cb52cba7f68b89d13)
- Linux FS is case sensitive by \@2bndy5 in [`c610e09`](https://github.com/nRF24/RF24Gateway/commit/c610e095f0f9ccc052255b0e36c5e63c9a8f86f5)
- This will fail in CI; I need to debug it more by \@2bndy5 in [`5def5e4`](https://github.com/nRF24/RF24Gateway/commit/5def5e40146f64cd86c1bf73d9401761adf86a1c)
- Use regex to detect if returned url ends with .git by \@2bndy5 in [`5109a13`](https://github.com/nRF24/RF24Gateway/commit/5109a132edd0e1f9aab527f87efba0882c1110f9)
- Better regex pattern by \@2bndy5 in [`eabc7af`](https://github.com/nRF24/RF24Gateway/commit/eabc7af12cfa83a67c5100de328bad9e0c5f355f)
- Show me what `git describe` returns on GH runner by \@2bndy5 in [`6a91d3f`](https://github.com/nRF24/RF24Gateway/commit/6a91d3f8d4fd32f1b21f841adde947081e05d9d8)
- Try with fetch-depth = 0 by \@2bndy5 in [`4a05041`](https://github.com/nRF24/RF24Gateway/commit/4a05041314bcbc2853bd5da6a956dc67585f7693)
- [no ci] rename workflow assets DL file by \@2bndy5 in [`c396585`](https://github.com/nRF24/RF24Gateway/commit/c3965855f01de247c8f1868e8eeb7455470529a2)
- [no ci] code fmt; prep rename casing of READMEs by \@2bndy5 in [`029709e`](https://github.com/nRF24/RF24Gateway/commit/029709e2881f92b2862bc79cc8be786199fe43bd)
- Rename READMEs in uppercase by \@2bndy5 in [`2b56b88`](https://github.com/nRF24/RF24Gateway/commit/2b56b88fc1529fef614700a24aac201beaef107b)
- Clean up linux CI workflow by \@2bndy5 in [`4309008`](https://github.com/nRF24/RF24Gateway/commit/43090081954226875f17e1dcf9dc603ab22fc3d1)
- [no ci] add a comment by \@2bndy5 in [`26960fb`](https://github.com/nRF24/RF24Gateway/commit/26960fbab2f45a60762f19a6405a023fe4f14d69)
- Update CONTRIBUTING.md by \@2bndy5 in [`a76581d`](https://github.com/nRF24/RF24Gateway/commit/a76581d5d2b780a7b9d699b3b66e8d4075897652)
- Manually trigger CI workflows by \@2bndy5 in [`b3cb626`](https://github.com/nRF24/RF24Gateway/commit/b3cb6267bf64cd94314fecf75bc299d74f50cfe4)
- Gimme that badge by \@2bndy5 in [`461cb2c`](https://github.com/nRF24/RF24Gateway/commit/461cb2cabb1ee9351b78de55f8f3927aa52608ed)
- Use CMake CLI to change debugging level by \@2bndy5 in [`501d9ea`](https://github.com/nRF24/RF24Gateway/commit/501d9eab059d3df8c439f0eb5a1a42ce01c3b34e)
- Limit Doxygen CI trigger during open PR by \@2bndy5 in [`9ea06ee`](https://github.com/nRF24/RF24Gateway/commit/9ea06ee5a45cd7d0be1128fd0f3238fc05478e2d)
- Update mqttLogger.py by \@2bndy5 in [`fecc698`](https://github.com/nRF24/RF24Gateway/commit/fecc698e2f595d4266312a182a2df39cbe5204b1)
- Adjust mesh display by \@TMRh20 in [`49ea85e`](https://github.com/nRF24/RF24Gateway/commit/49ea85ec184ecec3c2670767ee007b25bd593f09)

[1.0.3]: https://github.com/nRF24/RF24Gateway/compare/v1.0.2...v1.0.3

Full commit diff: [`v1.0.2...v1.0.3`][1.0.3]

## [1.0.2] - 2021-03-31

### <!-- 8 --> 📝 Documentation

- Docs abstraction & code formating by \@2bndy5 in [#17](https://github.com/nRF24/RF24Gateway/pull/17)

[1.0.2]: https://github.com/nRF24/RF24Gateway/compare/v1.0.1...v1.0.2

Full commit diff: [`v1.0.1...v1.0.2`][1.0.2]

## [1.0.1] - 2020-11-29

### <!-- 4 --> 🛠️ Fixed

- Fix warnings by \@2bndy5 in [`87b4ae6`](https://github.com/nRF24/RF24Gateway/commit/87b4ae63ee585813ddef9b6f3bb066cb84a39ee3)

### <!-- 8 --> 📝 Documentation

- Redirect doc links to nRF24 org by \@2bndy5 in [`a076209`](https://github.com/nRF24/RF24Gateway/commit/a076209ba6a632c6949e1e72090c290e72cbe9c7)
- Merge pull request \#15 from 2bndy5/master by \@TMRh20 in [#15](https://github.com/nRF24/RF24Gateway/pull/15)

### <!-- 9 --> 🗨️ Changed

- Edit doxyfile OUTPUT_DIR & add workflow by \@2bndy5 in [`451a08e`](https://github.com/nRF24/RF24Gateway/commit/451a08e60c16e02a855852c3145826584c72eaf6)
- Format doxygen css file & include latest changes by \@2bndy5 in [`92a4a71`](https://github.com/nRF24/RF24Gateway/commit/92a4a719465b63332f5acb33641c74dbb218f3e0)
- Only run doxygen on master branch by \@2bndy5 in [`511fefc`](https://github.com/nRF24/RF24Gateway/commit/511fefc0db82b8d26f13fbcae16be3424c068cc2)

[1.0.1]: https://github.com/nRF24/RF24Gateway/compare/v1.0.0...v1.0.1

Full commit diff: [`v1.0.0...v1.0.1`][1.0.1]

## [1.0.0] - 2020-08-13

### <!-- 1 --> 🚀 Added

- Add missing include to example by \@TMRh20 in [`78e716a`](https://github.com/nRF24/RF24Gateway/commit/78e716a0146b2ef5f248c1c775974561413224f5)
- Add delay for latest RF24Network changes by \@TMRh20 in [`3f3caf5`](https://github.com/nRF24/RF24Gateway/commit/3f3caf5d4ec94e27e9a9360767da2934894569dc)
- Add interrupt code by \@TMRh20 in [`50a037d`](https://github.com/nRF24/RF24Gateway/commit/50a037db9d95ba496e482888a5d6e3522773e6b1)
- Address renewal improvements by \@TMRh20 in [`21373ca`](https://github.com/nRF24/RF24Gateway/commit/21373ca0408e9e3891bbbe4b90d1da06026a7adb)
- Add initial code of simple python logger by \@TMRh20 in [`684cd1a`](https://github.com/nRF24/RF24Gateway/commit/684cd1ad2eee0cdab4bc1688cb98d2bb29836c8c)
- Add RF24Network sniffer by \@TMRh20 in [`625aebd`](https://github.com/nRF24/RF24Gateway/commit/625aebdb43d3e977eeb3a75c6d199788586fffcb)
- Add routing functionality by \@TMRh20 in [`70644f0`](https://github.com/nRF24/RF24Gateway/commit/70644f058eb8e30b6c585c3ca7f7177493f9e554)
- Add failure recovery to examples by \@TMRh20 in [`d4455dd`](https://github.com/nRF24/RF24Gateway/commit/d4455dd43af660bbd4ba78332ceedddf783ac2cb)

### <!-- 3 --> 🗑️ Removed

- Update interrupt code by \@TMRh20 in [`68477bc`](https://github.com/nRF24/RF24Gateway/commit/68477bc4286816ca144503c15dfeddeb31a85299)
- Remove unneeded delay, fix non-interrupt version by \@TMRh20 in [`ed44440`](https://github.com/nRF24/RF24Gateway/commit/ed4444052c86ae59219b017f23309d964a5f4d07)
- Update RF24Gateway.cpp by \@TMRh20 in [`874f771`](https://github.com/nRF24/RF24Gateway/commit/874f771047694bb23c6185e3066d6f3efffe1340)
- Use radio masking of IRQs by \@TMRh20 in [`1d0686e`](https://github.com/nRF24/RF24Gateway/commit/1d0686e634bccf571f82db5f32343a33645caba8)
- Interrupt fix: Examples by \@TMRh20 in [`85d4591`](https://github.com/nRF24/RF24Gateway/commit/85d4591697f408441f038ce402ebf643bb1a952e)
- Reliabiity fixes by \@TMRh20 in [`3508adc`](https://github.com/nRF24/RF24Gateway/commit/3508adc46a6f1c0c23305c9679370a03118d6bc9)

### <!-- 4 --> 🛠️ Fixed

- Fix ncurses example, channel setting with RF24Mesh by \@TMRh20 in [`fd3e767`](https://github.com/nRF24/RF24Gateway/commit/fd3e767f10fd7fc06941d7d724d73a8085a4eb4c)
- Broken comms for nodes w/16-bit addresses by \@TMRh20 in [`68dc95d`](https://github.com/nRF24/RF24Gateway/commit/68dc95d2fa4988832a47e74a532fa35541f4014b)
- Fix ncurses keyboard controls/inputs with SPIDEV by \@TMRh20 in [`afe76e0`](https://github.com/nRF24/RF24Gateway/commit/afe76e0ef0a826f49c62e9891bc97f054f70fe9a)
- Fix/Update address renewal for ncurses interfaces by \@TMRh20 in [`9a38f5d`](https://github.com/nRF24/RF24Gateway/commit/9a38f5de9cffbf9fd85c4fbd94522b2ce7ea6bff)
- Fix broken ncurses example by \@TMRh20 in [`7757e07`](https://github.com/nRF24/RF24Gateway/commit/7757e071700a84afa8c67303dea80cbfaa7b2e30)
- Fix for last update by \@TMRh20 in [`5530c4e`](https://github.com/nRF24/RF24Gateway/commit/5530c4ef3cd04e063e701d240abf561737d7b02a)
- Fix stability probs using interrupts by \@TMRh20 in [`2116017`](https://github.com/nRF24/RF24Gateway/commit/21160173a05c981b72ad1f02b7a70917c157e7ee)
- Fix node list printout by \@TMRh20 in [`aeee05b`](https://github.com/nRF24/RF24Gateway/commit/aeee05b82cd9d3f5b6ed7c1e5594e11968e99ee2)
- Bad memory leak by \@TMRh20 in [`6e0506c`](https://github.com/nRF24/RF24Gateway/commit/6e0506c5656530df01d2428b21dfaa2f455256ce)
- Missing address requests by \@TMRh20 in [`6505d3c`](https://github.com/nRF24/RF24Gateway/commit/6505d3cb821f09bea1e79a912928290dfc110572)

### <!-- 8 --> 📝 Documentation

- Update .h file (doc changes) by \@TMRh20 in [`0b8c5b2`](https://github.com/nRF24/RF24Gateway/commit/0b8c5b2d25673d7b86ada909b02d3d110a3fab33)
- Routing files & doc by \@TMRh20 in [`307db39`](https://github.com/nRF24/RF24Gateway/commit/307db392d52dbb8641f4e1a5972e75c199692365)
- Minor doc fix by \@TMRh20 in [`dff739c`](https://github.com/nRF24/RF24Gateway/commit/dff739c63462ad0992def687c3096d7476fb796b)
- Minor fix & doc update by \@TMRh20 in [`207c960`](https://github.com/nRF24/RF24Gateway/commit/207c96097970a5168330b41437a02e1333c0df5c)

### <!-- 9 --> 🗨️ Changed

- :confetti_ball: Added .gitattributes by \@TMRh20 in [`077eb33`](https://github.com/nRF24/RF24Gateway/commit/077eb338ecc8e3454973804e8d9b957ebf081868)
- Creation of RF24Gateway library by \@TMRh20 in [`2ebf603`](https://github.com/nRF24/RF24Gateway/commit/2ebf603a5fef8cd9d47a176b57b3c9f64c75079d)
- Drop packets if writing to network interface fails by \@TMRh20 in [`23cc843`](https://github.com/nRF24/RF24Gateway/commit/23cc843536716a8e2301c7f8247531a10d746b34)
- Update for latest RF24Network_Dev changes by \@TMRh20 in [`47f8963`](https://github.com/nRF24/RF24Gateway/commit/47f8963268f81899457ce1174a35c84c0a23952d)
- Enable IP config internally, add NCurses example by \@TMRh20 in [`a3235a8`](https://github.com/nRF24/RF24Gateway/commit/a3235a8daffafb32dcb6d433ee0a4ee80820431e)
- Minor changes, Add client examples, Update examples by \@TMRh20 in [`342203e`](https://github.com/nRF24/RF24Gateway/commit/342203ebb8cad22ccc4133df6160fe54ac34dac0)
- Rem int handling code from non-interrupt example by \@TMRh20 in [`b7ccb68`](https://github.com/nRF24/RF24Gateway/commit/b7ccb68c9d88ac1d5fdc46a691af672d49172b06)
- Mask radio tx irqs for interrupt examples by \@TMRh20 in [`d097b70`](https://github.com/nRF24/RF24Gateway/commit/d097b704f38aa51777a1e097ed85975563b6fd45)
- Further modifications for interrupt testing by \@TMRh20 in [`159c098`](https://github.com/nRF24/RF24Gateway/commit/159c09841543f7382b48de184b0e384c073aa74b)
- Merge remote-tracking branch 'refs/remotes/origin/Interrupts' by \@TMRh20 in [`bde1eda`](https://github.com/nRF24/RF24Gateway/commit/bde1eda131b9ba278f207bacd2f86df2dcc00339)
- RPI2 autodetection by \@yozik04 in [`4b6f6fd`](https://github.com/nRF24/RF24Gateway/commit/4b6f6fd1845889e7510871ef30d25444ef5e449f)
- Merge pull request \#6 from yozik04/master by \@TMRh20 in [`82b89b3`](https://github.com/nRF24/RF24Gateway/commit/82b89b36034091c59a8b64319da245588c54b2f1)
- Lower cpu usage on non-interrupt examples by \@TMRh20 in [`31f6603`](https://github.com/nRF24/RF24Gateway/commit/31f6603ae0b2e92ec91817ac3017dca58dc02523)
- Revert change: breaks application by \@TMRh20 in [`fb36bd2`](https://github.com/nRF24/RF24Gateway/commit/fb36bd2741316becd1ad8e035059a5e9bb481044)
- RPi Bugfix: Interrupts & ncursesInt example update by \@TMRh20 in [`bc6b671`](https://github.com/nRF24/RF24Gateway/commit/bc6b671f0e31097888dfab369b5215d32c53bef0)
- NcursesInt revert pins, adjust gw.poll placement by \@TMRh20 in [`7db01a1`](https://github.com/nRF24/RF24Gateway/commit/7db01a1be321af25afd176c996e1e5aa87eb76cf)
- Output of data via UDP + create WireShark dissector by \@TMRh20 in [`32c70bf`](https://github.com/nRF24/RF24Gateway/commit/32c70bf75be77bea5f3304208fcc0cbc84ebe92a)
- Minor Fixes & Cleanup by \@TMRh20 in [`c0a57ed`](https://github.com/nRF24/RF24Gateway/commit/c0a57ed66993e54175fff196dc8ceca2549e213d)
- Err, count var needs to be 16-bit by \@TMRh20 in [`1acd057`](https://github.com/nRF24/RF24Gateway/commit/1acd057549af17fa88984a5d8fa06a26303d2cd9)
- Forgot the channel again... by \@TMRh20 in [`81c7627`](https://github.com/nRF24/RF24Gateway/commit/81c7627dc5bd68015f3bdb032b6e91cfb309ca72)
- Forgot to re-enable interrupts by \@TMRh20 in [`930fac3`](https://github.com/nRF24/RF24Gateway/commit/930fac32d377f0233aec692b186786018e900ee9)

[1.0.0]: https://github.com/nRF24/RF24Gateway/compare/077eb338ecc8e3454973804e8d9b957ebf081868...v1.0.0

Full commit diff: [`077eb33...v1.0.0`][1.0.0]

## New Contributors
* \@yozik04 made their first contribution
<!-- generated by git-cliff -->
