# Academic References for "The Invisible Window"

**Paper:** "The Invisible Window: Exploiting OS-Level Display Affinity to Bypass WebRTC Proctoring Systems"
**Format:** Harvard (Australian) Referencing Style
**Date Compiled:** 24 March 2026
**Coverage:** 2019-2026 | Peer-reviewed and credible sources only

---

## CATEGORY 1 — WebRTC Screen Capture Limitations

### C1-1. W3C Screen Capture Specification

**Citation:** Bruaroey, J-I & Alon, E 2025, *Screen Capture*, W3C Working Draft, World Wide Web Consortium (W3C), viewed 24 March 2026, <https://www.w3.org/TR/screen-capture/>.

**DOI/URL:** https://www.w3.org/TR/screen-capture/

**Relevance:** The authoritative W3C specification for `getDisplayMedia()`. Explicitly documents that the API captures "a user's display, or parts thereof" by trusting the OS compositing pipeline. Section 5 (Security and Privacy Considerations) identifies the core risk: "Display capture presents...risk to the cross site request forgery protections offered by the browser sandbox." The spec acknowledges that "information that is not currently rendered to the screen" should be obscured unless elevated authorization is granted — confirming the trust assumption that what the OS reports as visible is treated as ground truth. The spec mandates HTTPS origins and transient activation but delegates actual pixel composition entirely to the operating system.

---

### C1-2. Threat Models Over Space and Time (End-to-End Encrypted Messaging)

**Citation:** Das Chowdhury, P, Sameen, M, Blessing, J, Boucher, N, Gardiner, J, Burrows, T, Anderson, R & Rashid, A 2024, 'Threat models over space and time: A case study of end-to-end encrypted messaging applications', *Software: Practice and Experience*, vol. 54, no. 12, pp. 2316-2335.

**DOI:** https://doi.org/10.1002/spe.3341

**Relevance:** Demonstrates that system designers often fail to update threat models when moving from one platform to another (e.g., mobile to desktop). Directly analogous to proctoring systems that assume browser-level screen capture is equivalent to physical display capture. Their STRIDE/LINDDUN threat analysis framework is applicable to modelling the display affinity attack surface.

---

### C1-3. Browser Extension Security and Trust Model

**Citation:** Lyu, T, Liu, L, Zhu, F, Hu, S, Ye, R & Dalle, J 2022, 'BEFP: An extension recognition system based on behavioral and environmental fingerprinting', *Security and Communication Networks*, vol. 2022, no. 1, article 7896571.

**DOI:** https://doi.org/10.1155/2022/7896571

**Relevance:** Demonstrates that browser extensions operate with elevated privileges across three levels (background, action, injected script) and can "drastically change the behaviour of web pages." Relevant to understanding how browser-based proctoring extensions interact with the display pipeline and the trust boundaries they assume.

---

### C1-4. MDN Web Docs — Screen Capture API

**Citation:** Mozilla Developer Network 2025, *Using the Screen Capture API*, MDN Web Docs, viewed 24 March 2026, <https://developer.mozilla.org/en-US/docs/Web/API/Screen_Capture_API/Using_Screen_Capture>.

**URL:** https://developer.mozilla.org/en-US/docs/Web/API/Screen_Capture_API/Using_Screen_Capture

**Relevance:** Authoritative technical documentation confirming that `getDisplayMedia()` relies on user consent and OS-level display surface enumeration. Documents that the API cannot distinguish between what is physically rendered on the monitor and what the OS compositing layer reports — the fundamental gap exploited by display affinity attacks.

---

## CATEGORY 2 — OS-Level Display Affinity / Window Hiding

### C2-1. Microsoft SetWindowDisplayAffinity Documentation

**Citation:** Microsoft 2025, *SetWindowDisplayAffinity function (winuser.h)*, Microsoft Learn, viewed 24 March 2026, <https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity>.

**URL:** https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setwindowdisplayaffinity

**Relevance:** The primary technical reference for `WDA_EXCLUDEFROMCAPTURE` (value `0x00000011`), introduced in Windows 10 Version 2004. The documentation explicitly states: "The window is displayed only on a monitor. Everywhere else, the window does not appear at all." Critically, Microsoft's own documentation acknowledges: "Unlike a security feature or an implementation of Digital Rights Management (DRM), there is no guarantee that using SetWindowDisplayAffinity...will strictly protect windowed content." This confirms the API was designed for content protection, not security — yet it creates a complete blind spot for screen capture-based proctoring.

---

### C2-2. Apple NSWindow.SharingType Documentation

**Citation:** Apple Inc. 2025, *NSWindow.SharingType*, Apple Developer Documentation, viewed 24 March 2026, <https://developer.apple.com/documentation/appkit/nswindow/sharingtype>.

**URL:** https://developer.apple.com/documentation/appkit/nswindow/sharingtype

**Relevance:** Documents macOS `NSWindow.SharingType.none` which prevents legacy CoreGraphics APIs from reading window content. Critical for understanding the macOS side of the display affinity attack. However, as of macOS 15+, ScreenCaptureKit ignores `sharingType = .none`, creating asymmetric behaviour across OS versions that complicates both attack and detection strategies.

---

### C2-3. Apple ScreenCaptureKit Framework

**Citation:** Apple Inc. 2025, *ScreenCaptureKit*, Apple Developer Documentation, viewed 24 March 2026, <https://developer.apple.com/documentation/screencapturekit/>.

**URL:** https://developer.apple.com/documentation/screencapturekit/

**Relevance:** Documents `SCContentFilter` which supports both display-independent window capture and display-dependent filters with include/exclude capabilities. The framework can exclude specific windows and applications from capture. On macOS 15+, ScreenCaptureKit captures all visible content and `setContentProtection`/`sharingType = .none` only blocks legacy CoreGraphics APIs — a deliberate Apple design change that has security implications for proctoring systems relying on older capture methods.

---

### C2-4. DRM-Based Mobile Digital Rights Management

**Citation:** Dharminder, D 2020, 'LWEDM: Learning with error based secure mobile digital rights management system', *Transactions on Emerging Telecommunications Technologies*, vol. 32, no. 2, article e4199.

**DOI:** https://doi.org/10.1002/ett.4199

**Relevance:** Describes a post-quantum DRM authentication protocol for mobile content protection. Provides context for understanding DRM-adjacent window hiding mechanisms and the broader ecosystem of content protection technologies that display affinity APIs were originally designed to support.

---

### C2-5. macOS 15+ ScreenCaptureKit Behaviour Change (Community-Documented)

**Citation:** columbusux 2025, *macOS 15+: ScreenCaptureKit ignores setContentProtection / NSWindow.sharingType*, GitHub Issue #14200, tauri-apps/tauri, 20 September, viewed 24 March 2026, <https://github.com/tauri-apps/tauri/issues/14200>.

**URL:** https://github.com/tauri-apps/tauri/issues/14200

**Relevance:** Community-documented evidence that Apple's macOS 15+ deliberately changed ScreenCaptureKit behaviour: all window contents are composited into a single framebuffer before display, and ScreenCaptureKit captures this framebuffer directly, bypassing legacy protection flags entirely. `setContentProtection(true)` and `NSWindow.sharingType = .none` no longer prevent screen capture. This affects Tauri, Electron (Issue #48258), and all frameworks relying on legacy CoreGraphics protection. This documents the evolving arms race between OS-level content protection and capture frameworks — directly relevant to the paper's threat model.

> **Note:** This is a GitHub issue, not a peer-reviewed source. Included as primary technical evidence documenting a confirmed upstream behaviour change with no known workaround.

---

## CATEGORY 3 — Remote Proctoring Systems & Their Weaknesses

### C3-1. Community-Developed Proctoring Evasion Techniques (ACM CCS 2024)

**Citation:** Simko, L, Hutchinson, A, Isaac, A, Fries, E, Sherr, M & Aviv, AJ 2024, '"Modern problems require modern solutions": Community-developed techniques for online exam proctoring evasion', in *Proceedings of the 2024 ACM SIGSAC Conference on Computer and Communications Security (CCS '24)*, ACM, Salt Lake City, UT, pp. 1-18.

**DOI:** https://doi.org/10.1145/3658644.3691638

> **Note:** Page range "pp. 1-18" is from the ACM proceedings early access. Verify final pagination against the published proceedings volume before submission.

**Relevance:** The most directly relevant peer-reviewed paper. Qualitative analysis of 137 social media videos and 4,297 comments documenting both non-technical (sticky notes) and deeply technical (custom virtual machines) methods of evading online proctoring. Published at CCS, a top-tier security venue. Establishes that proctoring evasion is a community-driven, openly documented phenomenon — directly supporting the paper's argument that OS-level display affinity represents a sophisticated escalation of known evasion techniques.

---

### C3-2. Educators' Perspectives on Proctoring (USENIX Security 2023)

**Citation:** Balash, DG, Fainchtein, RA, Korkes, E, Grant, M, Sherr, M & Aviv, AJ 2023, 'Educators' perspectives of using (or not using) online exam proctoring', in *Proceedings of the 32nd USENIX Security Symposium*, USENIX Association, Anaheim, CA, pp. 5091-5108.

**DOI/URL:** https://www.usenix.org/conference/usenixsecurity23/presentation/balash
**ISBN:** 978-1-939133-37-3

**Relevance:** Survey of 3,400+ instructors revealing that many educators developed alternatives to proctoring and weighed privacy/utility tradeoffs. Documents the ProctorU data breach (444,000 users' PII leaked) and a Proctorio vulnerability allowing remote software activation. Published at USENIX Security — establishes that even educators question proctoring system reliability, and that commercial systems have documented security failures.

---

### C3-3. Students' Privacy and Security Perceptions (USENIX SOUPS 2021)

**Citation:** Balash, DG, Kim, D, Shaibekova, D, Fainchtein, RA, Sherr, M & Aviv, AJ 2021, 'Examining the examiners: Students' privacy and security perceptions of online proctoring services', in *Proceedings of the Seventeenth Symposium on Usable Privacy and Security (SOUPS 2021)*, USENIX Association, pp. 633-652.

**DOI/URL:** https://www.usenix.org/conference/soups2021/presentation/balash

**Relevance:** Analysis of user reviews of proctoring browser extensions and survey (n=102) documenting students' security/privacy concerns. Identifies that proctoring services employ "lockdown" browser modes, video/screen monitoring, local network traffic analysis, and eye tracking — the full attack surface relevant to the paper. Establishes the adversarial dynamic between students and proctoring systems.

---

### C3-4. Security Requirements for Proctoring (IEEE EDUCON 2024)

**Citation:** Luijben, R, van den Broek, F & Alpar, G 2024, 'Security requirements for proctoring in higher education', in *2024 IEEE Global Engineering Education Conference (EDUCON)*, IEEE, Kos Island, Greece, pp. 1-8.

**DOI:** https://doi.org/10.1109/EDUCON60312.2024.10578664

**Relevance:** Identifies five pivotal security requirements for online proctoring: student authentication, verification of work authenticity, prevention of prior access to exam materials, protection of personal data, and exam availability. Uses threat analysis and software inspection methodology. Directly relevant as a framework for evaluating which security requirements the display affinity attack violates (primarily #2 — verification of work authenticity).

---

### C3-5. Online Exam Proctoring Technologies: Innovation or Deterioration?

**Citation:** Lee, K & Fanguy, M 2022, 'Online exam proctoring technologies: Educational innovation or deterioration?', *British Journal of Educational Technology*, vol. 53, no. 3, pp. 475-490.

**DOI:** https://doi.org/10.1111/bjet.13182

**Relevance:** Critical analysis of the effectiveness and drawbacks of proctoring technologies adopted during COVID-19. Provides a balanced academic critique of whether proctoring systems actually improve academic integrity or merely create an illusion of security while introducing new problems.

---

### C3-6. Proctoring at the Nexus of Integrity and Surveillance

**Citation:** Khalil, M, Prinsloo, P & Slade, S 2022, 'In the nexus of integrity and surveillance: Proctoring (re)considered', *Journal of Computer Assisted Learning*, vol. 38, no. 6, pp. 1589-1602.

**DOI:** https://doi.org/10.1111/jcal.12713

**Relevance:** Positions proctoring at the intersection of academic integrity enforcement and student surveillance. Analyses how the COVID-19 pivot to Emergency Remote Online Teaching amplified reliance on proctoring tools. Provides the ethical tension framework relevant to the paper's discussion of why display affinity exploits are significant — they expose the gap between surveillance capability and actual security.

---

### C3-7. The Fear of Big Brother: Negative Side-Effects of Proctoring

**Citation:** Conijn, R, Kleingeld, A, Matzat, U & Snijders, C 2022, 'The fear of big brother: The potential negative side-effects of proctored exams', *Journal of Computer Assisted Learning*, vol. 38, no. 6, pp. 1521-1534.

**DOI:** https://doi.org/10.1111/jcal.12651

**Relevance:** Empirical study of the psychological impact of proctoring on student performance and anxiety. Demonstrates that proctoring systems impose costs (student anxiety, performance degradation) even when they function as intended — making the security-versus-cost tradeoff analysis more damning when the security itself is shown to be bypassable.

---

### C3-8. On the Necessity (or Lack Thereof) of Digital Proctoring

**Citation:** Duncan, A & Joyner, D 2022, 'On the necessity (or lack thereof) of digital proctoring: Drawbacks, perceptions, and alternatives', *Journal of Computer Assisted Learning*, vol. 38, no. 5, pp. 1482-1496.

**DOI:** https://doi.org/10.1111/jcal.12700

**Relevance:** Evaluates whether digital proctoring is actually necessary by examining drawbacks, user perceptions, and alternative assessment strategies. Directly supports the paper's argument that if proctoring can be technically bypassed via display affinity, institutions should invest in alternative integrity measures rather than an arms race.

---

### C3-9. Sins of Omission: Privacy in E-Learning Systems

**Citation:** Paris, B, Reynolds, R & McGowan, C 2021, 'Sins of omission: Critical informatics perspectives on privacy in e-learning systems in higher education', *Journal of the Association for Information Science and Technology*, vol. 73, no. 5, pp. 708-725.

**DOI:** https://doi.org/10.1002/asi.24575

**Relevance:** Critical informatics analysis of seven online learning platforms at Rutgers University, uncovering serious privacy and intellectual property violations. Demonstrates that e-learning platforms (including proctoring systems) were "already committing serious privacy...violations" before the pandemic accelerated adoption.

---

### C3-10. Students' Technological Ambivalence Toward Proctoring

**Citation:** Johri, A & Hingle, A 2023, 'Students' technological ambivalence toward online proctoring and the need for responsible use of educational technologies', *Journal of Engineering Education*, vol. 112, no. 1, pp. 221-242.

**DOI:** https://doi.org/10.1002/jee.20504

**Relevance:** Documents the paradox of students who recognise the need for academic integrity but simultaneously resist invasive proctoring technologies. Provides the sociotechnical context for why students might seek and share bypass techniques, including OS-level evasion methods.

---

### C3-11. Cryptographic Approaches to Online Assessment Security

**Citation:** Mehrishi, AA, Sarmah, DK & Daneva, M 2025, 'How can cryptography secure online assessments against academic dishonesty?', *Security and Privacy*, vol. 8, no. 4, article e70065.

**DOI:** https://doi.org/10.1002/spy2.70065

**Relevance:** Identifies security gaps in Canvas LMS, Moodle, and Google Forms that facilitate cheating, and proposes cryptographic solutions. Demonstrates that the vulnerability surface extends beyond proctoring software to the underlying platform architecture — supporting the argument that display affinity exploits are one of many systemic weaknesses.

---

### C3-12. Student Experience of Remote Proctoring: Scoping Review

**Citation:** Marano, E, Newton, PM, Birch, Z, Croombs, M, Gilbert, C & Draper, MJ 2024, 'What is the student experience of remote proctoring? A pragmatic scoping review', *Higher Education Quarterly*, vol. 78, no. 3, pp. 1031-1047.

**DOI:** https://doi.org/10.1111/hequ.12506

**Relevance:** Comprehensive scoping review of student experiences with remote proctoring, documenting the rapid adoption during COVID-19 and its impacts on testing validity and student wellbeing.

---

### C3-13. Visual Data Obfuscation in Remote Proctoring (EuroUSEC 2024)

**Citation:** Mukherjee, S, Distler, V, Lenzini, G & Cardoso-Leite, P 2024, 'Balancing the perception of cheating detection, privacy and fairness: A mixed-methods study of visual data obfuscation in remote proctoring', in *Proceedings of the 2024 European Symposium on Usable Security (EuroUSEC '24)*, ACM, Karlstad, Sweden.

**DOI:** https://doi.org/10.1145/3688459.3688474

**Relevance:** Explores how selectively obfuscating information in video recordings can protect test-taker privacy while ensuring effective cheating detection. Relevant to the paper's discussion of what proctoring systems actually need to capture versus what they currently capture, and how display affinity creates selective obfuscation from the attacker's side.

---

## CATEGORY 4 — Behavioral & Artifact-Based Detection

### C4-1. Computational Intelligence for Cheating Detection (Systematic Review)

**Citation:** Kaddoura, S, Vincent, S, Hemanth, DJ & Ashraf, I 2023, 'Computational intelligence and soft computing paradigm for cheating detection in online examinations', *Applied Computational Intelligence and Soft Computing*, vol. 2023, no. 1, article 3739975.

**DOI:** https://doi.org/10.1155/2023/3739975

**Relevance:** Comprehensive systematic literature review of soft computing techniques for online exam cheating detection, covering face recognition, facial expression recognition, head posture analysis, eye gaze tracking, network data traffic analysis, and IP spoofing detection. Maps the full landscape of detection mechanisms that display affinity attacks must evade or remain invisible to.

---

### C4-2. Behavioral Pattern Modeling for Online Exam Proctoring

**Citation:** Ferdosi, BJ, Rahman, M, Sakib, AM, Helaly, T & Chakraborty, P 2023, 'Modeling and classification of the behavioral patterns of students participating in online examination', *Human Behavior and Emerging Technologies*, vol. 2023, no. 1, article 2613802.

**DOI:** https://doi.org/10.1155/2023/2613802

**Relevance:** Proposes an automated proctoring solution using MediaPipe for facial landmark detection, K-NN classification for head/eye/lip orientation, and coordinated movement pattern analysis. Achieved 87.5% cheating detection accuracy. Documents that cheating increased from 29.9% pre-COVID to 54.7% during COVID. Critical for understanding the behavioral signals that display affinity attacks leave undisturbed (gaze patterns remain normal) versus those they might disrupt (screen interaction patterns).

---

### C4-3. Eye Gaze Tracking for Cheating Detection (IEEE ICITR 2021)

**Citation:** Dilini, N, Senaratne, A, Yasarathna, T, Warnajith, N & Seneviratne, L 2021, 'Cheating detection in browser-based online exams through eye gaze tracking', in *2021 6th International Conference on Information Technology Research (ICITR)*, IEEE, pp. 1-6.

**DOI:** https://doi.org/10.1109/ICITR54349.2021.9657277

**Relevance:** Implements browser-based eye gaze tracking using webcams and One-Class SVM to detect anomalous visual attention patterns during exams. Directly relevant because display affinity attacks would not trigger gaze anomalies — the student's eyes remain on the screen — making this detection method ineffective against the described attack vector.

---

### C4-4. Continuous Authentication via Multimodal Behavioral Fusion

**Citation:** Guan, J, Li, X, Zhang, Y & Andersson, K 2021, 'Design and implementation of continuous authentication mechanism based on multimodal fusion mechanism', *Security and Communication Networks*, vol. 2021, no. 1, article 6669429.

**DOI:** https://doi.org/10.1155/2021/6669429

**Relevance:** Proposes a Multimodal Fusion-based Continuous Authentication (MFCA) scheme combining keystroke dynamics, mouse movement patterns, and application usage for continuous identity verification. Relevant because display affinity attacks could potentially be detected through anomalous mouse/keyboard patterns (switching to a hidden window) if such behavioral biometrics were integrated into proctoring systems.

---

### C4-5. Mouse Dynamics-Based Insider Threat Detection via Deep Learning

**Citation:** Hu, T, Niu, W, Zhang, X, Liu, X, Lu, J, Liu, Y & Chen, J 2019, 'An insider threat detection approach based on mouse dynamics and deep learning', *Security and Communication Networks*, vol. 2019, no. 1, article 3898951.

**DOI:** https://doi.org/10.1155/2019/3898951

**Relevance:** Proposes continuous authentication using mouse biobehavioral characteristics and CNN-based deep learning, achieving identity verification every 7 seconds with 2.94% FAR and 2.28% FRR. Demonstrates that mouse dynamics can detect user identity changes — a potential countermeasure if proctoring systems monitored for mouse movement anomalies consistent with interacting with a hidden window.

---

### C4-6. Keystroke and Mouse Behavioral Feature Fusion Authentication

**Citation:** Wang, X, Zheng, Q, Zheng, K, Wu, T & Perez, GM 2020, 'User authentication method based on MKL for keystroke and mouse behavioral feature fusion', *Security and Communication Networks*, vol. 2020, no. 1, article 9282380.

**DOI:** https://doi.org/10.1155/2020/9282380

**Relevance:** Proposes dual-index authentication fusing keystroke and mouse features via Multiple Kernel Learning, achieving 89.6% accuracy. Addresses heterogeneity between keystroke and mouse features — relevant to understanding what behavioral signals a hidden-window user might produce that differ from normal exam-taking patterns.

---

### C4-7. Online vs Face-to-Face Cheating Prevalence

**Citation:** Yazici, S, Yildiz Durak, H, Aksu Dünya, B & Şentürk, B 2022, 'Online versus face-to-face cheating: The prevalence of cheating behaviours during the pandemic compared to the pre-pandemic among Turkish university students', *Journal of Computer Assisted Learning*, vol. 39, no. 1, pp. 231-254.

**DOI:** https://doi.org/10.1111/jcal.12743

**Relevance:** Comparative study showing significant increase in cheating during online assessments compared to face-to-face settings. Both students and instructors reported higher perceived cheating rates online. Provides empirical evidence for the magnitude of the problem that display affinity exploits exacerbate.

---

### C4-8. Cheating Behaviour: Needs, Conceptions and Reasons

**Citation:** Rüth, M, Jansen, M & Kaspar, K 2024, 'Cheating behaviour in online exams: On the role of needs, conceptions and reasons of university students', *Journal of Computer Assisted Learning*, vol. 40, no. 5, pp. 1987-2008.

**DOI:** https://doi.org/10.1111/jcal.12994

**Relevance:** Examines the psychological motivations behind online exam cheating, including students' needs, conceptions of academic integrity, and situational reasons. Provides context for why students would seek and adopt technically sophisticated bypass methods like display affinity exploitation.

---

## CATEGORY 5 — Responsible Disclosure & Ethical Framing

### C5-1. Ethical Hacking Motivations and Career Development

**Citation:** Noordegraaf, JE & Weulen Kranenbarg, M 2023, 'Why do young people start and continue with ethical hacking? A qualitative study on individual and social aspects in the lives of ethical hackers', *Criminology & Public Policy*, vol. 22, no. 4, pp. 803-824.

**DOI:** https://doi.org/10.1111/1745-9133.12650

**Relevance:** Qualitative study of 15 ethical hackers who started before age 18, examining what motivated them to pursue responsible disclosure. Finds that "thinking of reporting vulnerabilities as a moral duty" is a key motivator, and that positive reactions from system owners stimulate continued ethical hacking. Directly supports framing the display affinity disclosure as responsible security research.

---

### C5-2. Design Ethics and Student Privacy in Higher Education

**Citation:** Lachheb, A, Abramenka-Lachheb, V, Moore, S & Gray, C 2023, 'The role of design ethics in maintaining students' privacy: A call to action to learning designers in higher education', *British Journal of Educational Technology*, vol. 54, no. 6, pp. 1653-1670.

**DOI:** https://doi.org/10.1111/bjet.13382

**Relevance:** Argues that maintaining student privacy is "not only a matter of policy and law but also a matter of design ethics." Provides an ethical framework for evaluating proctoring systems that prioritise surveillance over student privacy — directly relevant to the paper's responsible disclosure section.

---

### C5-3. ACM Code of Ethics and Professional Conduct

**Citation:** Association for Computing Machinery 2018, *ACM Code of Ethics and Professional Conduct*, ACM, viewed 24 March 2026, <https://www.acm.org/code-of-ethics>.

**URL:** https://www.acm.org/code-of-ethics

**Relevance:** The authoritative ACM ethics code requiring computing professionals to "protect confidentiality except in cases where there is evidence of a violation of law" and mandating "full disclosure of all pertinent system limitations and problems." Section 1.2 (Avoid harm) and 2.9 (Design systems that are robust and usable) directly apply to responsible vulnerability disclosure in proctoring systems.

---

### C5-4. IEEE Code of Ethics

**Citation:** Institute of Electrical and Electronics Engineers 2024, *IEEE Code of Ethics*, IEEE, viewed 24 March 2026, <https://www.ieee.org/about/corporate/governance/p7-8>.

**URL:** https://www.ieee.org/about/corporate/governance/p7-8

**Relevance:** IEEE members commit to "disclose promptly factors that might endanger the public" and "uphold the highest standards of integrity, responsible behaviour, and ethical conduct." Provides the ethical mandate for disclosing the display affinity vulnerability to proctoring vendors and the academic community.

---

### C5-5. OWASP Vulnerability Disclosure Guidelines

**Citation:** OWASP Foundation n.d., *Vulnerability Disclosure Cheat Sheet*, OWASP Cheat Sheet Series, viewed 24 March 2026, <https://cheatsheetseries.owasp.org/cheatsheets/Vulnerability_Disclosure_Cheat_Sheet.html>.

**URL:** https://cheatsheetseries.owasp.org/cheatsheets/Vulnerability_Disclosure_Cheat_Sheet.html

**Relevance:** Industry-standard guidelines for coordinated vulnerability disclosure, defining the process where "a vulnerability or an issue is disclosed to the public only after the responsible parties have been allowed sufficient time to patch or remedy the vulnerability." Provides the procedural framework for responsible disclosure of the display affinity bypass.

---

### C5-6. FIRST Guidelines for Multi-Party Vulnerability Coordination

**Citation:** Forum of Incident Response and Security Teams (FIRST) 2020, *Guidelines and Practices for Multi-Party Vulnerability Coordination and Disclosure*, version 1.1, FIRST, viewed 24 March 2026, <https://www.first.org/global/sigs/vulnerability-coordination/multiparty/guidelines-v1-1>.

**URL:** https://www.first.org/global/sigs/vulnerability-coordination/multiparty/guidelines-v1-1

**Relevance:** Establishes that security researchers should "cooperate with stakeholders to remediate or mitigate the security vulnerability and minimize harm associated with disclosure." The multi-party coordination aspect is particularly relevant since the display affinity vulnerability affects multiple proctoring vendors simultaneously and requires coordination with both OS vendors (Microsoft, Apple) and proctoring software companies.

---

### C5-7. CISA Coordinated Vulnerability Disclosure Program

**Citation:** Cybersecurity and Infrastructure Security Agency 2025, *Coordinated Vulnerability Disclosure Program*, CISA, viewed 24 March 2026, <https://www.cisa.gov/resources-tools/programs/coordinated-vulnerability-disclosure-program>.

**URL:** https://www.cisa.gov/resources-tools/programs/coordinated-vulnerability-disclosure-program

**Relevance:** The U.S. government's official CVD framework, aligning with ISO/IEC 29147 and ISO/IEC 30111 standards. Provides institutional backing for the responsible disclosure approach and documents the expected timeline and communication protocols for vulnerability reporting.

---

### C5-8. Operationalizing Cybersecurity Research Ethics (NDSS EthiCS 2023)

**Citation:** Reidsma, D, van der Ham, J & Continella, A 2023, 'Operationalizing cybersecurity research ethics review: From principles and guidelines to practice', in *Proceedings of the 2nd International Workshop on Ethics in Computer Security (EthiCS 2023)*, co-located with NDSS Symposium, Internet Society, San Diego, CA, 27 February.

**URL:** https://www.ndss-symposium.org/wp-content/uploads/2023/02/ethics2023-237352-paper.pdf

**Relevance:** Operationalizes existing guidance on cybersecurity research ethics into a directly implementable Ethics Review Board proposal, including self-assessment questions for probing research ethics, a Coordinated Vulnerability Disclosure procedure for research discoveries, and a university policy outline for institutional adoption. Directly applicable to framing the display affinity research within an institutional ethics framework.

---

### C5-9. Vulnerable Student Digital Well-Being in AI-Powered Education

**Citation:** Prinsloo, P, Khalil, M & Slade, S 2024, 'Vulnerable student digital well-being in AI-powered educational decision support systems (AI-EDSS) in higher education', *British Journal of Educational Technology*, vol. 55, no. 5, pp. 2075-2092.

**DOI:** https://doi.org/10.1111/bjet.13508

**Relevance:** Examines the entanglement of students' physical and digital well-being in AI-powered educational systems. Provides ethical context for why security vulnerabilities in proctoring systems (which are increasingly AI-powered) have real-world welfare implications for vulnerable student populations.

---

## SUPPLEMENTARY REFERENCES

### S-1. Instructors' Perceptions of Proctoring Value

**Citation:** Gribbins, M & Bonk, CJ 2023, 'An exploration of instructors' perceptions about online proctoring and its value in ensuring academic integrity', *British Journal of Educational Technology*, vol. 54, no. 6, pp. 1693-1714.

**DOI:** https://doi.org/10.1111/bjet.13389

**Relevance:** Explores instructors' decision-making about proctoring adoption during COVID-19, finding mixed confidence in its effectiveness.

---

### S-2. Scaling Anti-Plagiarism Efforts

**Citation:** Adkins, KL & Joyner, DA 2022, 'Scaling anti-plagiarism efforts to meet the needs of large online computer science classes: Challenges, solutions, and recommendations', *Journal of Computer Assisted Learning*, vol. 38, no. 6, pp. 1603-1619.

**DOI:** https://doi.org/10.1111/jcal.12710

**Relevance:** Documents the practical challenges of enforcing academic integrity at scale in online CS courses — the exact context where sophisticated technical bypasses like display affinity exploitation are most likely to emerge.

---

### S-3. Automated Online Exam Proctoring (IEEE TST 2017)

**Citation:** Atoum, Y, Chen, L, Liu, AX, Hsu, SDH & Liu, X 2017, 'Automated online exam proctoring', *IEEE Transactions on Multimedia*, vol. 19, no. 7, pp. 1609-1624.

**DOI:** https://doi.org/10.1109/TMM.2017.2656064

**URL:** https://ieeexplore.ieee.org/document/7828141

**Relevance:** Foundational paper on automated online exam proctoring using multimedia analytics with six behavior cue components: user verification, text detection, voice detection, active window detection, gaze estimation, and phone detection. Achieved 88.6% cheating detection accuracy. Relevant as baseline for understanding what existing proctoring systems can and cannot detect.

> **Note:** Published 2017 — outside the stated 2019-2026 range. Included as a foundational/seminal reference that subsequent proctoring research builds upon.

---

## REFERENCE COUNT SUMMARY

| Category | Count |
|----------|-------|
| 1 — WebRTC Screen Capture Limitations | 4 |
| 2 — OS-Level Display Affinity / Window Hiding | 5 |
| 3 — Remote Proctoring Systems & Weaknesses | 13 |
| 4 — Behavioral & Artifact-Based Detection | 8 |
| 5 — Responsible Disclosure & Ethical Framing | 9 |
| Supplementary | 3 |
| **Total** | **42** |

---

## SEARCH METHODOLOGY

References were gathered using:
- **Semantic Scholar API** — 12 parallel semantic searches with 2019-2026 date filtering
- **Google Web Search** — 11 targeted searches across IEEE Xplore, ACM DL, USENIX, NDSS
- **Direct document retrieval** — W3C specs, Microsoft Learn, Apple Developer Documentation
- **Citation chain following** — cross-referencing cited works from key papers

**Exclusions applied:** Blog posts, Medium articles, Reddit threads, and non-peer-reviewed opinions were excluded per the stated criteria. Technical documentation from W3C, Microsoft, Apple, OWASP, FIRST, and CISA was included as authoritative primary sources.

**Note on Category 2:** OS-level display affinity is a relatively under-researched area in the academic literature. The primary documentation exists in vendor technical references (Microsoft Learn, Apple Developer Documentation) rather than peer-reviewed papers. This gap itself supports the paper's contribution — the security implications of these APIs have not been formally studied in the proctoring context.
