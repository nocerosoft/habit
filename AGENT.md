# AI Developer Guidelines

If you are an AI Coding Agent assigned to maintain or expand this repository, you **MUST** adhere to the following architectural rules and constraints:

## 1. Architectural Separation of Concerns
This project uses a strict 3-layer architecture. **Do not mix these layers.**
- `src/types.h`: Contains all shared C `struct` definitions (e.g., `RecordResult`, `HabitStats`).
- `src/db.c` & `src/db.h`: The **Data Layer**. This layer interacts exclusively with SQLite. It must **NEVER** contain `printf` statements or handle UI formatting. It returns data back to the caller via pointers and structs. 
- `src/habit.c`: The **Presentation/CLI Layer**. This handles `argc/argv` parsing, calling `db.c` functions, and formatting the output to `stdout`. All `printf` logic belongs here.

## 2. Memory Safety
- You must check the return values of all dynamic memory allocations (`malloc`, `realloc`) for `NULL` to prevent Out-Of-Memory (OOM) Segmentation Faults. 
- Always use safe string operations (`snprintf`, `strncpy`). Buffer sizes for dates (like `YYYY-MM-DD`) should be at least `32` bytes to satisfy GCC `-Wformat-truncation` warnings.

## 3. UI and Terminal Compatibility
- **No Emojis**: Do not use emojis in the terminal output. They cause alignment issues on older terminals.
- **ANSI Colors & ASCII**: Use standard ANSI escape codes for coloring (e.g., `\x1b[32m`) and standard ASCII shapes (e.g., `■` `U+25A0`) for visual grids like the heatmap. Ensure color resets (`\x1b[0m`) are always applied.

## 4. Idempotency & Normalization
- `habit.c` uses `normalize_name()` to convert inputs to Title Case (e.g., `drink water` -> `Drink Water`).
- Database insertions for daily habit completions use `INSERT OR IGNORE`. It is completely valid for a user to record a habit multiple times a day; it simply counts as one daily completion. Do not change this logic.

## 5. Build System
- The project is compiled using `make`. Do not introduce complex build systems like CMake unless explicitly requested by the user. 
- Ensure any new `.c` files are added to the `Makefile` and the GitHub Actions `build.yml` workflow continues to output to `habit-skill/bin`.
