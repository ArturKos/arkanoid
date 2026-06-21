# Autonomous multi-agent implementation workflow (bricktron)

A supervisor loop that implements `BACKLOG.md` autonomously using six role-agents,
each on its own model. Ported/adapted from the SoundShelf workflow for bricktron
(C++11 + Allegro 5, CMake, **Linux-only**).

## Pipeline

```
ARCHITECT ─► PROGRAMMER ─► REVIEWER ─► CRITIC ─► TESTER ─► COMMITTER ─► (loop)
 (opus)       (sonnet)      (sonnet)   (opus)    (haiku)   (haiku)
```

Each role is a headless `claude -p` call with a role prompt
(`scripts/workflow_roles/<role>.md`) and a per-role model; every role ends its
reply with a machine-readable `===VERDICT=== {json}` line the supervisor parses.
reviewer `needs_changes` / critic `<7` / tester `fail` send the task back; the
committer pushes once all gates pass; the architect stops at `all_implemented`.

**Definition of Done:** Doxygen on new public API · clean Linux build
(`-Wall -Wextra -pedantic`, no new warnings) · tests where a target exists
(bricktron has none yet) · bricktron conventions (see `CLAUDE.md`).

**GUI can't be loop-verified:** the tester is headless — no display, no audio. So
gameplay/visual/audio items are marked **`🔬`** (code-complete, needs the human's
play-test), never `✅`. Flip `🔬`→`✅` only after you've played it.

## Use

1. **Add tasks to `BACKLOG.md`** (the loop does nothing useful with an empty
   backlog — the architect would just report `all_implemented`).
2. Run:
   ```bash
   python3 scripts/agent_workflow.py               # full loop, resumes from checkpoint
   python3 scripts/agent_workflow.py --once        # one stage then stop
   python3 scripts/agent_workflow.py --status      # current state
   python3 scripts/agent_workflow.py --reset       # clear state
   python3 scripts/agent_workflow.py --dry-run     # skip the committer (no push)
   ```
   Detached:
   ```bash
   setsid nohup python3 scripts/agent_workflow.py > .workflow/run.out 2>&1 < /dev/null &
   tail -f .workflow/status        # which agent is active + the task
   tail -f .workflow/progress.log  # timeline + verdicts
   ```
   Agents run with `--permission-mode bypassPermissions` (edit / build / push
   without prompts). The committer pushes to `github-arturkos:ArturKos/bricktron.git`.

## Resilience

State is checkpointed to `.workflow/state.json` (gitignored). On a token/usage
limit the supervisor waits for the reset and resumes the same stage; Ctrl-C +
re-launch also resumes. Models / timeouts / `CRITIC_PASS` / `MAX_ATTEMPTS` are
constants at the top of `scripts/agent_workflow.py`; role behaviour lives in
`scripts/workflow_roles/*.md`.
