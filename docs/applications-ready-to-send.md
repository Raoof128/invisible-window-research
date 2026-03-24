# Anthropic Applications — Ready to Send

**All details filled in. Copy-paste directly.**

**Author:** Mohammad Raouf Abedini
**Academic email:** mohammadraouf.abedini@students.mq.edu.au
**Console email:** raoof.r12@gmail.com
**Console Org ID:** cfe48766-59a6-4d8d-8833-1b8231864174
**Console Org name:** Raoof
**GitHub:** https://github.com/Raoof128
**Private repo:** https://github.com/Raoof128/invisible-window-research
**Website:** https://raoufabedini.dev

---

## APPLICATION 0: Responsible Disclosure (SEND FIRST)

**To:** usersafety@anthropic.com
**Subject:** Responsible Disclosure — Claude API Being Used in Commercial Proctoring Bypass Tools

---

Dear Anthropic Safety Team,

I am a computing researcher at Macquarie University, Sydney, writing to disclose that commercial products are using Claude's API as the backend intelligence for invisible screen capture evasion tools that bypass proctored assessments.

**The Finding:**

Tools including Cluely and Interview Coder embed large language models — including Claude — inside desktop windows that are programmatically invisible to all screen capture mechanisms. They exploit documented OS-level display affinity APIs (Windows `SetWindowDisplayAffinity` with `WDA_EXCLUDEFROMCAPTURE`, macOS `NSWindow.SharingType.none`) to create real-time AI assistants that proctoring software cannot detect. The test-taker sees a live AI assistant generating answers; the proctoring system sees a clean screen.

My research demonstrates:
- 100% screen capture evasion across Windows 10/11 and macOS 14–26
- A novel finding that macOS 26 remains vulnerable despite Apple's documented ScreenCaptureKit changes
- Pixel-level forensic verification (A/B comparison, 1.17M pixels, 100% match)
- Zero detectable artifacts in gaze tracking, mouse dynamics, or behavioral telemetry

**Relevance to Anthropic:**

This violates multiple sections of Anthropic's Usage Policy:
- "Bypass security controls such as authenticated systems, endpoint protection, or monitoring tools"
- "Plagiarize or submit AI-assisted work without proper permission or attribution"

Industry data indicates 35% of candidates in technical assessments showed signs of AI-assisted cheating in late 2025. Claude is one of the backend models being used in these commercial tools.

**What I'm Providing:**

I have a complete IEEE-format manuscript (8,672 words, 53 verified references), working proof-of-concept code, and detailed evaluation results. I am following coordinated vulnerability disclosure principles and am happy to share all research artifacts with your team.

This research was conducted using Claude Code powered by Claude Opus 4.6 (1M context), which I mention both for transparency and because it demonstrates the dual-use nature of the problem: the same AI capabilities that enabled this research are being exploited for academic misconduct.

I am not requesting any action beyond awareness. If your team would like the full manuscript or PoC evaluation data, I'm available at the email below.

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University, Sydney, Australia
mohammadraouf.abedini@students.mq.edu.au
https://raoufabedini.dev

---

## APPLICATION 1: External Researcher Access Program

**Form URL:** https://forms.gle/pZYC8f6qYqSKvRWn9
**Review cycle:** First Monday of each month

### Form Fields (fill in order)

**1. Email address:**
raoof.r12@gmail.com

**2. Full name:**
Mohammad Raouf Abedini

**3. Have you been recommended by an Anthropic employee?**
No (change to Yes if Saffron Huang or safety team responds to your disclosure email first)

**4. Anthropic Console Organization ID:**
cfe48766-59a6-4d8d-8833-1b8231864174

**5. Team description (max 200 words):**

I am a computing researcher at Macquarie University, Sydney, Australia, specialising in systems security and AI safety. My current research investigates how commercial AI-powered tools exploit OS-level display affinity APIs to embed invisible real-time AI assistants into proctored assessments — a form of AI-enabled misuse that directly threatens Anthropic's Usage Policy compliance. I have developed a working proof-of-concept demonstrating 100% screen capture evasion on Windows and macOS, supported by a complete IEEE-format manuscript with 53 peer-reviewed references and pixel-level forensic evaluation. My technical skills span Python, Swift, C, and systems programming, with experience in WebRTC internals, OS compositing pipelines, and browser security models. This research was conducted using Claude Code powered by Claude Opus 4.6 (1M context) as the primary research instrument, demonstrating both my familiarity with Anthropic's products and the model's capacity for rigorous academic security research.

**6. Research description (max 300 words):**

I am researching how commercial tools weaponise Claude and other LLMs for undetectable academic misconduct through OS-level screen capture evasion. Products like Cluely and Interview Coder embed Claude via API into windows marked with SetWindowDisplayAffinity (Windows) or NSWindow.SharingType.none (macOS), creating real-time AI assistants fully visible to the test-taker but producing zero pixels in WebRTC proctoring capture output.

I request API credits for three specific research objectives that align with Anthropic's Adversarial Robustness priority:

1. Detection signature development: Test whether Claude API request patterns originating from invisible overlay tools exhibit detectable characteristics — timing cadences, prompt structures, question-answer patterns — that could flag misuse at the API level.

2. Model-level awareness: Evaluate whether Claude can detect contextual signals indicating it is being used as a cheating backend (e.g., sequential exam-style questions, screenshot-extracted text, rapid question bursts) and whether safety mechanisms can be tuned to mitigate this use case.

3. Countermeasure validation: Use the API to test proposed detection classifiers against realistic cheating scenarios, measuring false positive rates against legitimate Claude usage to ensure defenses don't degrade the experience for honest users.

My preliminary findings include a novel result: NSWindow.SharingType.none achieves 100% evasion on macOS 26.3.1, contradicting the prevailing assumption that Apple mitigated this in macOS 15. This means the attack surface is broader than previously understood. Industry data shows 35% of candidates exhibit signs of AI-assisted cheating, yet zero academic papers formally analyse this attack class. My work provides the first systematic security analysis.

This research directly serves Anthropic's interest: Claude is being commercially deployed in violation of your Usage Policy, and understanding the interaction patterns enables model-level defenses.

**7. How many credits are you requesting?**
$1,000

**8. Do you need Quality of Service (QoS) guarantees?**
No

**9. Google Scholar or GitHub profile URL:**
https://github.com/Raoof128

**10. Are you located in the United States?**
No

**11. Terms of Service agreement:**
[Check: Agree]

**12. Age confirmation (18+):**
[Check: Yes]

---

## APPLICATION 2: Email to Saffron Huang (Societal Impacts Team)

**To:** saffron@anthropic.com
**CC:** (optionally cc: mohammadraouf.abedini@students.mq.edu.au for your records)
**Subject:** Your education report's gap — technical infrastructure behind AI-assisted academic misconduct

---

Dear Saffron,

I read your team's education report ("How University Students Use Claude") with great interest — particularly the finding that 47% of student conversations are direct answer-seeking and the flagged patterns around plagiarism avoidance. Your research maps the behavioural side of AI-assisted academic misconduct comprehensively. My research maps the technical infrastructure that makes it undetectable.

**What I've found:**

Commercial tools (Cluely, Interview Coder, Lumio) embed Claude and GPT-4 inside desktop windows that are programmatically invisible to all screen capture — including WebRTC-based proctoring systems. They use documented OS APIs (Windows `SetWindowDisplayAffinity`, macOS `NSWindow.SharingType.none`) to create real-time AI assistants that generate answers during proctored exams while the proctor's screen feed shows a perfectly clean desktop.

My research demonstrates:
- 100% evasion on all tested platforms (Windows 10/11, macOS 14–26)
- A novel finding that macOS 26 remains vulnerable despite Apple's documented mitigations — contradicting developer community assumptions
- Zero detectable artifacts in screen capture, webcam gaze tracking, or mouse dynamics
- An estimated 35% cheating prevalence from industry data

No academic paper has formally analysed this attack class despite active commercial exploitation.

**Why I'm reaching out:**

Your education report identified the demand-side patterns (students using Claude for direct answers). My research documents the supply-side infrastructure (invisible overlay tools delivering those answers undetectably during exams). Together, these form a complete picture of AI-enabled academic misconduct that neither analysis covers alone.

Your August 2025 misuse report documented Claude being used for cybercrime and fraud but did not cover academic fraud — my findings fill that gap.

**What I'm proposing:**
- Share our IEEE manuscript (8,672 words, 53 verified references) and PoC evaluation data
- Explore whether Anthropic's API usage data (via Clio) could identify invisible overlay tool patterns
- Potentially collaborate on a joint publication at FAccT or COLM

This research was conducted using Claude Code with Opus 4.6 (1M context) — your product enabled a single researcher to produce what would typically require a multi-person team. I'd be happy to share the session as a case study of AI-augmented research.

I've also submitted a responsible disclosure to usersafety@anthropic.com regarding Claude's presence in these commercial cheating tools.

Would you be open to a brief call?

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University, Sydney
mohammadraouf.abedini@students.mq.edu.au | https://raoufabedini.dev
GitHub: https://github.com/Raoof128/invisible-window-research (private — happy to grant access)

---

## APPLICATION 3: Economic Futures Program

**To:** economicfutures@anthropic.com
**Subject:** Research Proposal — Economic Cost of AI-Enabled Credential Fraud via Invisible Overlay Tools

---

Dear Economic Futures Team,

I am a computing researcher at Macquarie University, Sydney, proposing interdisciplinary research on how AI-enabled assessment fraud threatens the economic signalling function of educational credentials.

**The Technical Problem (already solved):**

Commercial tools (Cluely, Interview Coder) embed Claude and GPT-4 inside OS windows invisible to screen capture, enabling undetectable AI-assisted cheating on proctored assessments. My research demonstrates 100% evasion across all major platforms with zero artifacts. Industry data shows 35% of candidates exhibit signs of AI-assisted cheating. No formal academic analysis exists.

**The Economic Question (proposed research):**

If AI makes credential fraud undetectable and scalable, what happens to labour market trust in educational signals?

Proposed investigation:
1. Credential devaluation modelling using Spence (1973) signalling theory — how does widespread undetectable cheating degrade the informational value of degrees?
2. Employer response survey (n=200+) — how does awareness of invisible AI cheating tools affect hiring confidence and credential reliance?
3. Institutional cost analysis — comparing the cost of the proctoring arms race versus alternative assessment models
4. Policy recommendations for universities, employers, and AI providers

**Budget:** $25,000 ($15K researcher time, $5K survey platform/participant compensation, $5K presentation/dissemination). API credits would support computational analysis.

**Timeline:** 6 months to completion with empirical paper, employer survey results, and policy brief.

**Note on methodology:** This research was conducted using Claude Code powered by Claude Opus 4.6 (1M context). The same AI capabilities that enabled this study are the capabilities being exploited for credential fraud — illustrating the dual-use challenge at the heart of the economic question.

I recognise my background is computing rather than economics. I am actively seeking an economics faculty collaborator at Macquarie University to provide the econometric rigour this programme requires, and I welcome guidance on whether a co-PI arrangement would strengthen this proposal.

I am happy to share the full IEEE manuscript and proof-of-concept evaluation data.

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University, Sydney
mohammadraouf.abedini@students.mq.edu.au | https://raoufabedini.dev

---

## APPLICATION 4: Anthropic Fellows Program (AI Security Track)

**Apply via:** https://job-boards.greenhouse.io/anthropic/jobs/5030244008
**Cohort:** July 2026 (rolling applications)

### Application Fields

**Position:** Anthropic AI Security Fellow

**Resume/CV:** [ATTACH — TODO: prepare CV]

**Cover letter / research proposal:**

Dear Fellows Selection Committee,

I am applying for the AI Security Fellow position for the July 2026 cohort. My research demonstrates how commercial AI-powered tools exploit OS-level display affinity APIs to embed invisible real-time AI assistants — including Claude — into proctored assessments, achieving 100% evasion of screen capture-based monitoring.

**Why AI Security:**

This is an AI-enabled attack at scale. Tools like Cluely and Interview Coder use Claude's API as their backend LLM, feeding exam questions through invisible overlay windows and returning AI-generated answers in real time. The invisible window itself is just plumbing — the AI is what makes it consequential. An estimated 35% of candidates already show signs of AI-assisted cheating. Anthropic's own model is being commercially weaponised for academic fraud in violation of your Usage Policy.

**What I've built:**

- Working proof-of-concept demonstrating 100% screen capture evasion on Windows 10/11 and macOS 14–26
- Novel finding: NSWindow.SharingType.none achieves full evasion on macOS 26, contradicting the prevailing assumption that Apple mitigated this vulnerability
- Pixel-level forensic verification methodology (A/B comparison across 1.17M pixels, 3 independent capture methods)
- Complete IEEE manuscript (8,672 words, 53 verified references)
- ACPR reasoning engine (MCP server, 38 tests, Python)

All research conducted using Claude Code with Opus 4.6 (1M context).

**Proposed fellowship research (4 months):**

Month 1 — Attack landscape: Systematic evaluation against top 5 commercial proctoring systems. Taxonomy of invisible overlay tools and which AI models they use.

Month 2 — Detection development: OS-level agent detecting display affinity flags. Browser-based JavaScript detection framework. API request pattern analysis for cheating tool signatures.

Month 3 — Model-level mitigations: With Anthropic mentorship, explore whether Claude can detect when invoked as a cheating backend. Build a classifier for "likely assessment fraud" request patterns. Evaluate false positive rates.

Month 4 — Policy and dissemination: Technical report for Anthropic Safety. Policy recommendations for proctoring vendors and universities. Submit extended paper to AI safety venue.

**Location:** I am based in Sydney, Australia. I am an Australian citizen eligible for a UK Tier 5 Youth Mobility Visa or Canada IEC Working Holiday, either of which provides work authorisation. I am prepared to relocate to London, UK or Ontario, Canada for the duration of the fellowship and will secure work authorisation before the programme start date.

**About me:**
- Computing student, Macquarie University, Sydney
- Proficient in Python, Swift, C, systems programming
- Security research: working PoC, pixel-level forensic evaluation, IEEE manuscript
- Power user of Anthropic products (Claude Code + Opus 4.6 for all research)
- No PhD — but the listing says that's fine, and my PoC speaks for itself

Website: https://raoufabedini.dev
GitHub: https://github.com/Raoof128

Best regards,
Mohammad Raouf Abedini
mohammadraouf.abedini@students.mq.edu.au

---

## APPLICATION 5: Misuse Report (backup channel)

**To:** usersafety@anthropic.com (can be sent alongside or after Application 0)
**Subject:** Claude API Misuse — Integration in Commercial Invisible Overlay Cheating Tools

This is a shorter, more focused misuse report format if the disclosure email (App 0) feels too long:

---

**Misuse type:** Claude API integrated into commercial tools that bypass proctoring monitoring

**Products identified:**
- Cluely (https://cluely.com) — uses Claude as backend LLM for real-time answer generation
- Interview Coder (https://interviewcoder.co) — uses LLMs in capture-invisible overlay windows
- Lumio / NotchGPT (https://notchgpt.com) — invisible AI overlay marketed against proctoring

**Mechanism:** These tools embed Claude API calls inside desktop windows marked with OS-level display affinity flags (Windows WDA_EXCLUDEFROMCAPTURE, macOS NSWindow.SharingType.none) that are invisible to all screen capture including WebRTC getDisplayMedia(). The test-taker sees a live AI assistant; the proctor sees nothing.

**Usage Policy violations:**
- "Bypass security controls such as monitoring tools"
- "Submit AI-assisted work without proper permission or attribution"

**Scale:** Industry data indicates 35% of candidates showed signs of AI-assisted cheating (FabricHQ, Jan 2026)

**My research:** I have a complete IEEE manuscript, working PoC, and pixel-level evaluation data available upon request.

**Researcher:** Mohammad Raouf Abedini, Macquarie University
**Contact:** mohammadraouf.abedini@students.mq.edu.au

---

## SENDING ORDER

```
1. Application 0 (Responsible Disclosure)     → usersafety@anthropic.com
   Wait 2-3 days for any acknowledgment

2. Application 1 (External Researcher Access)  → Google Form
   Same week

3. Application 2 (Saffron Huang email)         → saffron@anthropic.com
   Same week or week 2

4. Application 3 (Economic Futures)             → economicfutures@anthropic.com
   Week 2 (after finding economics co-author, or send anyway)

5. Application 4 (Fellows Program)              → Greenhouse portal
   Week 3-4 (after preparing CV, once visa research is done)
```

---

## BEFORE SENDING CHECKLIST

- [ ] Anthropic Console account created (raoof.r12@gmail.com) — DONE
- [ ] Organization ID noted (cfe48766-59a6-4d8d-8833-1b8231864174) — DONE
- [ ] GitHub repo created (Raoof128/invisible-window-research, private) — DONE
- [ ] CV / resume prepared — **TODO**
- [ ] raoufabedini.dev updated with research page — **TODO**
- [ ] Proofread all emails one more time — **TODO**
- [ ] Check Cluely is still using Claude (verify before claiming) — **TODO**
