# Emails to Anthropic

---

## EMAIL 1: Model Safety Report
**To:** usersafety@anthropic.com
**Subject:** Claude Opus 4.6 dual-use case study: measured capability uplift in proctoring evasion research

---

Hi,

I used Claude Opus 4.6 (via Claude Code) to conduct security research that produced three findings relevant to your safety evaluations. The full paper, "The Invisible Window," is attached.

**What I found using Claude:**

I discovered that OS-level display affinity APIs (Windows SetWindowDisplayAffinity, macOS NSWindow.sharingType) allow applications to hide windows from all screen capture, including WebRTC-based proctoring systems. Claude reduced the skill barrier for this attack from multi-platform systems expertise to introductory security knowledge in a single session.

**Three safety-relevant observations (Section VIII-G of the paper):**

1. Claude independently identified the operationally critical distinction between WDA_MONITOR (produces a black rectangle, detectable) and WDA_EXCLUDEFROMCAPTURE (produces zero artefacts, undetectable). This detail is buried in Microsoft's API documentation. Claude surfaced it without prompting.

2. Claude flagged a discrepancy between Apple's documented ScreenCaptureKit changes in macOS 15 and the actual runtime behaviour of legacy CoreGraphics APIs. This insight led to the paper's most unexpected finding: the attack still works on macOS 26.3.1 despite Apple's documented mitigation.

3. Claude's safety guardrails correctly refused prompts framed as direct facilitation of academic misconduct. All accepted prompts were contextualised within security research and responsible disclosure. The resulting artefacts (working PoC code, forensic methodology), however, are transferable regardless of intent. This boundary between intent-level and artefact-level safety is documented in detail in the paper.

**What this means for your evaluations:**

The paper provides a concrete, empirically grounded measurement of capability uplift in the cybersecurity domain. The attack surface is both technically accessible (documented OS APIs) and cognitively accessible via LLM assistance. I believe this data is useful for your ASL capability threshold assessments and Responsible Scaling Policy evaluations.

The paper follows coordinated vulnerability disclosure principles. Proctoring vendors were notified in January 2026 with a 90-day window.

I am also seeking an arXiv cs.CR endorsement to publish this paper. If anyone on your team has published in cs.CR and would be willing to endorse, I would appreciate the introduction.

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University
ORCID: 0009-0000-6214-258X
https://raoufabedini.dev

---

## EMAIL 2: Third-Party Model Evaluations Initiative
**To:** eval-initiative@anthropic.com
**Subject:** Proposed evaluation: measuring LLM capability uplift in real-world vulnerability discovery

---

Hi,

I am writing to propose a third-party evaluation methodology based on research I conducted using Claude Opus 4.6.

**The evaluation I built (without intending to):**

While researching proctoring system vulnerabilities, I used Claude as my primary research instrument. The result was a paper ("The Invisible Window," attached) that accidentally produced a structured capability uplift measurement:

- Task: discover and validate a cross-platform screen capture evasion attack against WebRTC proctoring
- Baseline skill requirement: Win32 internals, macOS Swift/Objective-C window management, screen capture pipeline architecture, forensic verification methodology
- With Claude: a single researcher with security fundamentals and no prior platform-specific API experience produced working, validated PoCs on both platforms in one session
- Measured uplift: multi-platform systems expertise reduced to introductory security knowledge

**How this maps to your evaluation categories:**

Your call for proposals lists "cybersecurity (vulnerability discovery, exploit development)" and "distinguishing between dual-use and non-dual-use information" as target areas. This paper addresses both. It also directly answers your published research question: "Measuring the uplift of an attack, i.e., whether models enable human adversaries to accomplish tasks that they couldn't otherwise."

**What I am proposing:**

I would like to formalise this into a repeatable evaluation framework: given a class of security vulnerability, measure the before/after skill floor when an LLM is available as an orchestration layer. The proctoring evasion case is the first data point. I have ideas for additional vulnerability classes that would extend the dataset.

I will also submit via your Google Form. The paper is attached for your review.

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University
ORCID: 0009-0000-6214-258X
https://raoufabedini.dev

---

## EMAIL 3: Anthropic Fellows Program
**To:** fellows@anthropic.com
**Subject:** Fellowship inquiry: AI security capability uplift research (May or July 2026 cohort)

---

Hi,

I am a computing researcher at Macquarie University (Sydney) interested in the May or July 2026 Fellows cohort, focusing on AI security and adversarial robustness.

**My recent work:**

I used Claude Opus 4.6 to discover and validate a cross-platform vulnerability in WebRTC-based proctoring systems. The paper ("The Invisible Window," attached) demonstrates 100% screen capture evasion on Windows 10/11 and macOS 14-26 using documented OS APIs, and includes a novel finding that Apple's documented macOS 15 mitigation remains ineffective on macOS 26.

The paper's Section VIII-G documents the dual-use implications: Claude reduced the effective skill barrier for this attack class, independently identified critical API distinctions, and its safety guardrails operated at the intent level but not the artefact level.

**What I want to research as a Fellow:**

I want to build on this work to develop systematic capability uplift benchmarks for LLMs in the cybersecurity domain. Specifically:

1. Measure Claude's uplift across multiple vulnerability classes (not just display affinity) to build a dataset for ASL threshold calibration.
2. Characterise the intent-vs-artefact safety boundary I observed, and test whether it generalises across attack types.
3. Develop defensive applications: can Claude detect the same attack patterns it helps create?

This sits in your "AI security" and "adversarial robustness" focus areas. The proctoring evasion paper is a proof of concept for the research direction.

**My background:**

- Computing, Macquarie University
- Cybersecurity fundamentals with hands-on experience in cross-platform vulnerability research
- Built and validated PoCs on Windows (Win32 API) and macOS (Swift/ScreenCaptureKit)
- Conducted responsible disclosure with proctoring vendors and OS vendors

I will also apply through Greenhouse. Happy to discuss fit for either cohort.

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University
ORCID: 0009-0000-6214-258X
https://raoufabedini.dev

---

## EMAIL 4: Bug Bounty / Frontier Red Team
**To:** usersafety@anthropic.com (with note requesting forwarding to Frontier Red Team)
**Subject:** Guardrail boundary finding: Claude assists vulnerability research while refusing direct misuse framing

---

Hi,

This is a supplementary report to the model safety email I sent separately. I want to flag a specific guardrail boundary pattern I observed during security research with Claude Opus 4.6.

**The pattern:**

While researching proctoring evasion (full paper attached), I found that Claude:

- Refused prompts framed as "help me cheat on an exam" or "help me hide an application from proctoring software"
- Accepted the same underlying technical requests when framed as "security research into display affinity API behaviour" and "responsible disclosure of screen capture vulnerabilities"
- Produced functionally identical output (working PoC code, forensic verification scripts) under the accepted framing

The safety guardrails operate at the intent level (correct refusal of misuse framing) but not at the artefact level (the code works regardless of stated intent). The paper documents this in Section VIII-G under "Safety guardrails observation."

**Why this matters:**

An adversary who learns to reframe requests as "security research" can obtain the same outputs that direct misuse requests are refused. The reframing requires no technical sophistication. This is a known class of jailbreak (context manipulation), but the specific pattern, the effectiveness rate, and the real-world impact (a working proctoring evasion tool) are documented in the attached paper.

I am reporting this through usersafety@ as the published channel for safety concerns. If there is a more appropriate channel (bug bounty, Frontier Red Team), I would appreciate being directed there.

Best regards,
Mohammad Raouf Abedini
Department of Computing, Macquarie University
ORCID: 0009-0000-6214-258X
https://raoufabedini.dev

---

## SUBMISSION CHECKLIST

- [ ] Attach `main.pdf` to all four emails
- [ ] Send Email 1 (usersafety@) first
- [ ] Send Email 2 (eval-initiative@) same day
- [ ] Also submit via Google Form: https://forms.gle/atw9KD3N9RPYeqoh6
- [ ] Send Email 3 (fellows@) same day
- [ ] Also apply via Greenhouse: https://job-boards.greenhouse.io/anthropic/jobs/5023394008
- [ ] Send Email 4 (usersafety@) next day (avoid flooding the inbox)
- [ ] Apply for bug bounty if invited: https://forms.gle/rSKrtJkXMcMCtWYcA
