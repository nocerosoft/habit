#ifndef DB_H
#define DB_H

#include <sqlite3.h>

#include "types.h"

extern sqlite3 *db;

// Initialize the database and create tables if they don't exist
int db_init(const char *db_path);

// Close the database connection
void db_close(void);

// Record a habit for the current day
// Returns 0 on success, non-zero on error. Populates res.
int db_record_habit(const char *habit_name, RecordResult *res);

// List all habits with their streaks, weekly and monthly goals
// Allocates and populates an array of HabitStats, sets count. Returns 0 on success.
// Caller is responsible for freeing *stats_out if count > 0.
int db_list_habits(HabitStats **stats_out, int *count_out);

// Set a weekly or monthly goal for a habit
int db_set_goal(const char *habit_name, int target, const char *period);

// Retrieve heatmap counts for the last 168 days
// populates grid[7][25]. Returns 0 on success.
int db_heatmap(int grid[7][25]);

// Remove a habit and all its completions
int db_remove_habit(const char *habit_name);

#endif // DB_H
