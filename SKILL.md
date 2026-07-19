---
name: Habit Tracker
description: A command-line habit tracker to record and review daily habits, streaks, weekly and monthly goals.
version: 0.1.1
---

# Habit Tracker Agent Skill

This skill allows you to track and manage user habits using a compiled C CLI application backed by SQLite. 
The executable is installed globally to the user's system path (`~/.local/bin/habit`). You can run it simply by executing `habit <command>`.

## Important Application Behaviors & State
- **State File & CWD**: The application state is stored globally in a single SQLite database located at `~/.habits.db`. You do **not** need to change directories (CWD) to run the `habit` command. It will work identically from anywhere.
- **Auto-Normalization**: The app automatically Title Cases all habit names (e.g., `drink water` becomes `Drink Water`).
- **Idempotency**: Recording the same habit multiple times in a single day is completely safe. It simply counts as a single daily completion.
- **Negative Cases**: Attempting to remove a habit that does not exist will still succeed silently.

---

## 🚫 Do / Do Not Guidelines

- **DO** always use the actual stats provided by the CLI.
- **DO** reformat CLI tables into clean, readable bullet points if you are responding to the user on a mobile interface (like Telegram), as raw tables will wrap and break on small phone screens.
- **DO** wrap the `heatmap` output in a strict ` ```text ` code block to preserve its grid alignment, or verbally summarize their recent hot/cold streaks instead of pasting the wide grid on mobile.
- **DO** use the `habit list` command to scan for incomplete weekly/monthly goals before answering questions like "What should I do today?".
- **DO NOT** fabricate or hallucinate fields (e.g., do not tell the user they spent "30 minutes" on a habit, as the CLI only tracks binary completions).
- **DO NOT** ask for confirmation after recording a habit, simply report success.

---

## Command Signatures & Expected Outputs

### 1. Record a Habit
**Command**: `habit record "<activity_name>"`
**Success Output**:
```text
[OK] Recorded 'Swim' for today! (Streak: 1 days)

Keep going! 4 more times to reach your weekly goal.

You have completed 1/4 habits today.
```

### 2. List Habits
**Command**: `habit list`
**Success Output**:
```text
Habit                | Streak | Weekly  | Monthly
------------------------------------------------------
Swim                 | 1      | 1/5     | 1/0
```

### 3. Set a Goal
**Command**: `habit goal "<activity_name>" <target> <weekly|monthly>`
**Success Output**:
```text
Set weekly goal for 'Swim' to 5.
```

### 4. View Heatmap
**Command**: `habit heatmap`
**Success Output**: (Returns a 24-week GitHub-style ASCII heatmap with standard ANSI colors)

### 5. Remove a Habit
**Command**: `habit remove "<activity_name>"`
**Success Output**:
```text
Removed habit 'Swim'.
```

---

## Natural Language (NL) Mapping Table

Use this table to translate varied user phrasing into the exact command and flag format.

| User Phrasing | Action / Command | Example Execution |
| --- | --- | --- |
| "i swim today", "did I swim?", "swim again" | Record the habit | `habit record "Swim"` |
| "undo swim", "stop tracking running", "delete my swim habit" | Remove the habit | `habit remove "Swim"` |
| "weekly goal for run?", "i want to run 3 times a week" | Set a goal | `habit goal "Run" 3 weekly` |
| "what to do this week", "what do you think i should do today", "what is my progress" | List all habits to review progress | `habit list` |
| "show me my heatmap", "how active was I this year?" | Show the heatmap | `habit heatmap` |
