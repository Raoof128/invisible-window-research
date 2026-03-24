# Deep Research Reasoning Agent — Design Document

**Date:** 2026-03-24
**Status:** Approved
**Author:** Raouf + Claude

---

## 1. Overview

A difficulty-adaptive deep research agent that implements the full Actor-Critic-Planner-Reflexion reasoning pipeline from the AI reasoning architecture research. Runs as a Claude Code skill backed by a Python MCP server, powered by the user's Claude Max subscription.

**Primary use case:** Deep research and synthesis — multi-source research, literature review, synthesizing findings across papers and documents.

**Key properties:**
- No separate API key required — leverages Claude Code agent spawning on Max account
- Integrates with Crawl4AI for adaptive web research (pre-crawled data + live crawling)
- Claude-as-Judge for step-wise scoring (ThinkPRM-style generative verification)
- Difficulty-adaptive: single-pass for easy queries, full Forest-of-Thought for hard ones

---

## 2. Architecture

Two components working together:

```
User -> /deep-research "query"
         |
         v
+-----------------------------+
|  CLAUDE CODE SKILL          |  <- The orchestrator (LLM-powered)
|  - Spawns parallel agents   |
|  - Actor: generates paths   |
|  - Critic: scores quality   |
|  - Reflexion: self-corrects |
|  - Calls Crawl4AI for data  |
|  - Presents final synthesis |
+----------+------------------+
           | MCP tool calls
           v
+-----------------------------+
|  MCP SERVER (Python)        |  <- The algorithmic brain (deterministic)
|  - Difficulty estimation    |
|  - DORA budget allocation   |
|  - UCB branch selection     |
|  - PRM score tracking       |
|  - Tree state management    |
|  - Episodic memory (SQLite) |
|  - Source/citation tracking |
|  - Content sanitization     |
+-----------------------------+
```

**Design principle:** Claude Code agents handle all LLM work (generation, critique, synthesis) using the Max account. The MCP server handles all algorithmic work (scoring math, search algorithms, state persistence) — no API key needed.

---

## 3. MCP Server — The Reasoning Backend

**Name:** `reasoning-engine`
**Stack:** Python + FastMCP + SQLite + NetworkX (knowledge graph)

### 3.1 MCP Tools

| Tool | Input | Output | Purpose |
|------|-------|--------|---------|
| `init_research_session` | query, context? | session_id, difficulty, budget, strategy | Creates session, estimates difficulty, returns budget allocation |
| `register_branch` | session_id, branch_data | branch_id | Registers a new reasoning branch in the tree |
| `score_branch` | session_id, branch_id, q_score, advantage, critique_text, confidence | updated_branch | Records Critic's dual-signal score for a branch |
| `select_next_branches` | session_id | continue[], reflect[], prune[], allocation, kappa, budget_remaining | Runs UCB + DORA, returns branch decisions |
| `record_reflection` | session_id, branch_id, critique, revision | reflection_id | Stores Reflexion feedback in episodic memory |
| `get_session_state` | session_id | full tree state, scores, budget | Returns current session state |
| `check_termination` | session_id | should_terminate, reason | Checks termination conditions |
| `consensus_candidates` | session_id | top_k_branches with traces and scores | Returns best branches for final synthesis |
| `save_to_memory` | session_id, learnings | memory_id | Persists episodic memory for future sessions |
| `recall_memory` | query | relevant_memories[] | Retrieves past research and Reflexion learnings |
| `sanitize_content` | raw_text | cleaned_text | Strips injection patterns, scripts, suspicious prompts |

### 3.2 Internal Algorithms

**Difficulty Estimator:**
- Heuristic based on query length, keyword complexity, domain signals
- Returns score 0.0-1.0
- Maps to strategy and budget allocation

**DORA Budget Allocator:**
- Maps difficulty -> budget (branches, depth, max steps, tokens per branch)
- Computes score variance (kappa) at each planning step
- kappa > 0.15: breadth-first exploration (keep all branches, allocate evenly)
- kappa < 0.15: depth-first exploitation (focus on top 1-2 branches, prune rest)
- Recalculates every loop iteration

**UCB Selection:**
- Formula: Q(node) + c * sqrt(ln(parent_visits) / node_visits)
- Balances promising branches vs unexplored ones
- c (exploration constant) tunable per domain

**Content Sanitizer:**
- Strips scripts, HTML tags, suspicious prompt patterns
- Detects prompt injection attempts in crawled content
- Returns clean text safe for reasoning pipeline

---

## 4. Claude Code Skill — The Orchestrator

**Name:** `/deep-research`
**Invocation:** `/deep-research "How does X relate to Y across domains Z?"`

### 4.1 Phase 1: INITIALIZE

```
User invokes /deep-research "query"
  |
  +-- Call MCP: recall_memory(query)            -> retrieve past learnings
  +-- Call MCP: init_research_session(query)     -> get difficulty, budget
  |     Returns: { session_id, difficulty: 0.73, budget: {
  |       total_branches: 8, max_depth: 10, max_steps: 40,
  |       tokens_per_branch: 4000,
  |       strategy: "beam_search + reflexion" } }
  |
  +-- If pre-crawled data exists -> load it
      If not -> spawn Research Agent to crawl initial sources via Crawl4AI
               -> Call MCP: sanitize_content(raw_data) before use
```

### 4.2 Phase 2: GENERATE (Actor)

```
Based on budget.total_branches, spawn N parallel agents:

  Agent-1: "Research path: focus on neuroscience perspective"
  Agent-2: "Research path: focus on computational models"
  Agent-3: "Research path: focus on empirical studies"
  ...

Each agent:
  - Gets: query + sanitized pre-crawled data + episodic memory
  - Gets: explicit token budget (budget.tokens_per_branch)
  - Gets: instruction to be concise, advance research, not restate known info
  - Can call Crawl4AI to gather more sources mid-research
  - All crawled data passes through MCP: sanitize_content() first
  - Returns: reasoning trace + intermediate findings + sources

  Call MCP: register_branch(session_id, branch_data) for each
```

### 4.3 Phase 3: EVALUATE (Critic)

```
Spawn a Critic agent for each branch (parallel):

  Critic prompt:
  "You are evaluating a research reasoning path.
   Score on two dimensions:
   - Promise (0-1): How likely is this path to produce a
     comprehensive, accurate synthesis?
   - Progress (0-1): How much has this step advanced beyond
     what was already known?

   Also provide a textual critique: what's strong, what's
   weak, what's missing, what claims are unverified?"

  Returns: { q_score, advantage, critique_text, confidence }

  Call MCP: score_branch(session_id, branch_id, q_score,
            advantage, critique_text, confidence) for each
```

### 4.4 Phase 4: PLAN (Planner)

```
Call MCP: select_next_branches(session_id)

  MCP server internally:
    - Computes kappa (variance of scores)
    - If kappa > 0.15 -> explore (keep multiple branches)
    - If kappa < 0.15 -> exploit (focus on best branch)
    - Flags low-scoring branches for Reflexion
    - Decrements budget

  Returns: {
    continue: [branch_ids to expand],
    reflect: [branch_ids needing critique injection],
    prune: [branch_ids to drop],
    allocation: "breadth-first",
    kappa: 0.34,
    budget_remaining: { steps: 28 }
  }
```

### 4.5 Phase 5: REFLECT (Reflexion)

```
For each branch in "reflect" list:

  Spawn Reflexion agent:
  "Here is a research path and its critique:
   PATH: {full_trace}
   CRITIQUE: {critique_text}

   Acknowledge the weaknesses. Revise your approach.
   Address unverified claims by calling Crawl4AI.
   Produce an improved reasoning trace."

  Returns: revised trace + new sources

  Call MCP: record_reflection(session_id, branch_id,
            original_critique, revision_summary)
```

### 4.6 Phase 6: LOOP or TERMINATE

```
Call MCP: check_termination(session_id)

  Termination conditions:
    - Budget exhausted (hard stop -> force consensus)
    - Top branch q_score > 0.85 AND confidence > 0.8
    - Score delta between iterations < 0.02 (convergence)

  If NOT terminated -> go back to Phase 2 (expand continuing branches)
  If terminated -> Phase 7
```

### 4.7 Phase 7: SYNTHESIZE (Consensus + Output)

```
Call MCP: consensus_candidates(session_id)
  -> Returns top-K branches with full traces and scores

Spawn Synthesis agent:
  "You have K research paths on {query}.
   Synthesize into a comprehensive research report.
   Resolve contradictions via evidence weight.
   Cite all sources. Flag remaining uncertainties."

  Returns: final markdown report

Call MCP: save_to_memory(session_id, learnings)
  -> Persists key findings and Reflexion learnings for future sessions

Output report to user.
```

### 4.8 Full Loop Visualization

```
INIT -> GENERATE -> EVALUATE -> PLAN --+
          ^                            |
          |    +-- REFLECT <-----------+
          |    |                       |
          +----+            TERMINATE? +
                                 |     |
                                 No ---+
                                 |
                                 Yes
                                 |
                                 v
                             SYNTHESIZE -> Output
```

---

## 5. Data Structures

### 5.1 Research Session

```python
ResearchSession:
  id: str                    # uuid
  query: str                 # original research query
  difficulty: float          # 0.0 - 1.0
  strategy: str              # "single_pass" | "best_of_n" | "beam_search" | "forest"
  budget:
    total_branches: int
    max_depth: int
    max_steps: int
    tokens_per_branch: int
    remaining_steps: int
  status: str                # "active" | "converged" | "budget_exhausted" | "completed"
  created_at: datetime
```

### 5.2 Branch

```python
Branch:
  id: str                    # uuid
  session_id: str
  parent_id: str | null      # null for root branches
  depth: int
  trace: list[str]           # full reasoning steps (text-based handoff)
  sources: list[Source]       # crawled URLs + sanitized excerpts
  q_score: float             # Promise — expected success probability
  advantage: float           # Progress — improvement over baseline
  critique: str              # textual critique from Critic
  confidence: float          # Critic's self-assessed confidence
  visits: int                # for UCB calculation
  status: str                # "active" | "pruned" | "reflecting" | "completed"
```

### 5.3 Reflection

```python
Reflection:
  id: str
  branch_id: str
  session_id: str
  original_critique: str
  revision_summary: str
  score_before: float
  score_after: float
```

### 5.4 Episodic Memory

```python
EpisodicMemory:
  id: str
  session_id: str
  query: str
  key_learnings: list[str]   # what worked, what didn't
  domain_tags: list[str]
  created_at: datetime
```

### 5.5 Source

```python
Source:
  url: str
  title: str
  excerpt: str               # sanitized extract
  relevance_score: float
  crawled_at: datetime
```

---

## 6. Difficulty-Adaptive Behavior

### 6.1 Difficulty -> Strategy Mapping

| Difficulty | Strategy | Branches | Depth | Steps | Reflexion Rounds |
|------------|----------|----------|-------|-------|------------------|
| 0.0 - 0.3 | single_pass | 1 | 3 | 5 | 0 |
| 0.3 - 0.5 | best_of_n | 3 | 5 | 15 | 1 |
| 0.5 - 0.7 | beam_search | 5 | 8 | 30 | 2 |
| 0.7 - 1.0 | forest | 8 | 12 | 50 | 3 |

### 6.2 DORA In-Session Switching

After each EVALUATE phase:
- kappa = variance(q_scores across active branches)
- kappa > 0.15 (uncertain) -> EXPLORE: keep all branches, allocate evenly
- kappa < 0.15 (confident) -> EXPLOIT: focus budget on top 1-2 branches, prune rest
- Recalculates every loop iteration

### 6.3 Early Termination Triggers

- Top branch q_score > 0.85 AND confidence > 0.8
- Score delta between iterations < 0.02 (convergence)
- Budget exhausted (hard stop -> force consensus via Phase 7)

---

## 7. Security Boundaries

### 7.1 Content Sanitization

All Crawl4AI output passes through `sanitize_content()` before entering the reasoning pipeline:
- Strips scripts, HTML tags, suspicious patterns
- Detects prompt injection attempts
- Returns clean text only

### 7.2 Critic Isolation

The Critic agent evaluates **reasoning logic and claims**, never raw crawled content. Raw web data stays sandboxed in the Actor phase behind the sanitizer.

### 7.3 Budget Hard Limits

Each spawned agent receives explicit token budgets from DORA allocation. Prevents overthinking and runaway token usage.

---

## 8. Crawl4AI Integration

**Adaptive collaboration model:**

1. **Pre-crawl mode:** User provides existing crawled data or the INIT phase does a broad initial crawl. Data is sanitized and distributed to Actor agents.

2. **Live crawl mode:** Actor agents can trigger new Crawl4AI calls mid-reasoning when they identify knowledge gaps or need to verify claims. All results pass through sanitization.

3. **Verification crawl:** Reflexion agents can crawl to verify claims flagged as unsubstantiated by the Critic.

---

## 9. Technology Stack

| Component | Technology |
|-----------|-----------|
| MCP Server | Python + FastMCP |
| Database | SQLite (sessions, branches, memory) |
| Knowledge Graph | NetworkX (optional, for source relationships) |
| Skill | Claude Code skill (prompt-based) |
| LLM | Claude via Claude Code Max account (agent spawning) |
| Web Research | Crawl4AI (existing MCP server) |
| Content Safety | Custom sanitizer in MCP server |

---

## 10. Design Decisions Log

| Decision | Chosen | Rejected | Rationale |
|----------|--------|----------|-----------|
| Runtime | Claude Code skill + MCP | Standalone Python CLI, LangGraph | Max account = free compute; no API key needed |
| PRM/Critic | Claude-as-Judge | Separate trained model, self-critique only | No training needed, immediate, dual-signal scoring |
| Search complexity | Difficulty-adaptive | Fixed strategy, simple-first | Full vision from day one per user preference |
| Crawl4AI integration | Adaptive (both modes) | Pre-crawl only, live only | Maximum flexibility for different research scenarios |
| State handoff | Full text traces | Encrypted tokens | Claude has no encrypted_content equivalent; text traces work |
| Orchestration | Claude Code agent spawning | LangGraph, Agent SDK | Avoids API key requirement; leverages Max subscription |
