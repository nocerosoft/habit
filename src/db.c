#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

sqlite3 *db = NULL;

int db_init(const char *db_path) {
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql_habits = 
        "CREATE TABLE IF NOT EXISTS habits ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT UNIQUE COLLATE NOCASE NOT NULL, "
        "weekly_goal INTEGER DEFAULT 0, "
        "monthly_goal INTEGER DEFAULT 0, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    const char *sql_completions = 
        "CREATE TABLE IF NOT EXISTS completions ("
        "habit_id INTEGER NOT NULL, "
        "completion_date DATE NOT NULL, "
        "FOREIGN KEY(habit_id) REFERENCES habits(id), "
        "UNIQUE(habit_id, completion_date)"
        ");";

    char *err_msg = 0;
    if (sqlite3_exec(db, sql_habits, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    if (sqlite3_exec(db, sql_completions, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return 1;
    }

    return 0;
}

void db_close(void) {
    if (db) {
        sqlite3_close(db);
        db = NULL;
    }
}

int db_record_habit(const char *habit_name, RecordResult *res) {
    memset(res, 0, sizeof(RecordResult));
    sqlite3_stmt *stmt;
    
    // Insert habit if not exists
    const char *sql_insert_habit = "INSERT OR IGNORE INTO habits (name) VALUES (?);";
    if (sqlite3_prepare_v2(db, sql_insert_habit, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        return 1;
    }

    // Insert completion for today
    const char *sql_insert_completion = 
        "INSERT OR IGNORE INTO completions (habit_id, completion_date) "
        "SELECT id, date('now', 'localtime') FROM habits WHERE name = ?;";
        
    if (sqlite3_prepare_v2(db, sql_insert_completion, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        return 1;
    }

    int changes = sqlite3_changes(db);
    res->is_duplicate = (changes == 0);
    
    if (!res->is_duplicate) {
        // Calculate streak for this newly recorded habit
        int streak = 0;
        const char *sql_streak = 
            "SELECT completion_date FROM completions "
            "WHERE habit_id = (SELECT id FROM habits WHERE name = ?) "
            "ORDER BY completion_date DESC;";
        if (sqlite3_prepare_v2(db, sql_streak, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
            
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char today[32], yesterday[32], expected_date[32] = "";
            snprintf(today, sizeof(today), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            t -= 24 * 3600;
            tm = *localtime(&t);
            snprintf(yesterday, sizeof(yesterday), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            
            int is_first = 1;
            int streak_broken = 0;
            
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const unsigned char *date_str = sqlite3_column_text(stmt, 0);
                if (date_str) {
                    int y, m, d;
                    sscanf((const char*)date_str, "%d-%d-%d", &y, &m, &d);
                    struct tm dtm = {0};
                    dtm.tm_year = y - 1900; dtm.tm_mon = m - 1; dtm.tm_mday = d;
                    time_t c_time = mktime(&dtm);
                    time_t now = time(NULL);
                    double diff_days = difftime(now, c_time) / (24 * 3600);
                    
                    if (diff_days <= 7) res->weekly_count++;
                    if (diff_days <= 30) res->monthly_count++;

                    if (!streak_broken) {
                        if (is_first) {
                            if (strcmp((const char*)date_str, today) == 0 || strcmp((const char*)date_str, yesterday) == 0) {
                                streak++;
                                time_t exp_t = c_time - 24 * 3600;
                                struct tm n_tm = *localtime(&exp_t);
                                snprintf(expected_date, sizeof(expected_date), "%04d-%02d-%02d", n_tm.tm_year + 1900, n_tm.tm_mon + 1, n_tm.tm_mday);
                            } else {
                                streak_broken = 1;
                            }
                            is_first = 0;
                        } else {
                            if (strcmp((const char*)date_str, expected_date) == 0) {
                                streak++;
                                time_t exp_t = c_time - 24 * 3600;
                                struct tm n_tm = *localtime(&exp_t);
                                snprintf(expected_date, sizeof(expected_date), "%04d-%02d-%02d", n_tm.tm_year + 1900, n_tm.tm_mon + 1, n_tm.tm_mday);
                            } else {
                                streak_broken = 1;
                            }
                        }
                    }
                }
            }
            sqlite3_finalize(stmt);
        }
        res->streak = streak;

        // Fetch goals
        const char *sql_goals = "SELECT weekly_goal, monthly_goal FROM habits WHERE name = ?;";
        if (sqlite3_prepare_v2(db, sql_goals, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                res->weekly_goal = sqlite3_column_int(stmt, 0);
                res->monthly_goal = sqlite3_column_int(stmt, 1);
            }
            sqlite3_finalize(stmt);
        }
    }

    // Query daily progress
    int total_habits = 0;
    int completed_today = 0;
    
    const char *sql_total = "SELECT COUNT(*) FROM habits;";
    if (sqlite3_prepare_v2(db, sql_total, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total_habits = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    res->total_active = total_habits;

    const char *sql_completed = 
        "SELECT COUNT(DISTINCT habit_id) FROM completions "
        "WHERE completion_date = date('now', 'localtime');";
    if (sqlite3_prepare_v2(db, sql_completed, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            completed_today = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    res->completed_today = completed_today;

    if (completed_today < total_habits) {
        const char *sql_remaining = 
            "SELECT name FROM habits "
            "WHERE id NOT IN ("
            "  SELECT habit_id FROM completions WHERE completion_date = date('now', 'localtime')"
            ");";
        if (sqlite3_prepare_v2(db, sql_remaining, -1, &stmt, NULL) == SQLITE_OK) {
            int capacity = 10;
            res->remaining = malloc(sizeof(RemainingHabit) * capacity);
            if (!res->remaining) {
                sqlite3_finalize(stmt);
                return 1;
            }
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                if (res->remaining_count >= capacity) {
                    capacity *= 2;
                    RemainingHabit *temp = realloc(res->remaining, sizeof(RemainingHabit) * capacity);
                    if (!temp) {
                        sqlite3_finalize(stmt);
                        free(res->remaining);
                        res->remaining = NULL;
                        return 1;
                    }
                    res->remaining = temp;
                }
                const unsigned char *name = sqlite3_column_text(stmt, 0);
                strncpy(res->remaining[res->remaining_count].name, (const char*)name, 255);
                res->remaining[res->remaining_count].name[255] = '\0';
                res->remaining_count++;
            }
            sqlite3_finalize(stmt);
        }
    }

    return 0;
}

int db_remove_habit(const char *habit_name) {
    sqlite3_stmt *stmt;
    
    const char *sql_delete_completions = 
        "DELETE FROM completions WHERE habit_id IN (SELECT id FROM habits WHERE name = ?);";
        
    if (sqlite3_prepare_v2(db, sql_delete_completions, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        return 1;
    }

    const char *sql_delete_habit = "DELETE FROM habits WHERE name = ?;";
    if (sqlite3_prepare_v2(db, sql_delete_habit, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    } else {
        return 1;
    }

    return 0;
}

int db_set_goal(const char *habit_name, int target, const char *period) {
    sqlite3_stmt *stmt;
    
    // Insert habit if not exists
    const char *sql_insert = "INSERT OR IGNORE INTO habits (name) VALUES (?);";
    if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, habit_name, -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    
    const char *sql_update;
    if (strcmp(period, "weekly") == 0) {
        sql_update = "UPDATE habits SET weekly_goal = ? WHERE name = ?;";
    } else if (strcmp(period, "monthly") == 0) {
        sql_update = "UPDATE habits SET monthly_goal = ? WHERE name = ?;";
    } else {
        return 1;
    }
    
    if (sqlite3_prepare_v2(db, sql_update, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, target);
        sqlite3_bind_text(stmt, 2, habit_name, -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);
    } else {
        return 1;
    }
    
    return 0;
}

int db_list_habits(HabitStats **stats_out, int *count_out) {
    sqlite3_stmt *stmt;
    *stats_out = NULL;
    *count_out = 0;
    
    // Query to get each habit and its completions sorted by date descending
    const char *sql = 
        "SELECT h.id, h.name, c.completion_date, h.weekly_goal, h.monthly_goal "
        "FROM habits h "
        "LEFT JOIN completions c ON h.id = c.habit_id "
        "ORDER BY h.id, c.completion_date DESC;";
        
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 1;
    }

    int current_id = -1;
    char current_name[256] = "";
    int current_weekly_goal = 0;
    int current_monthly_goal = 0;
    int streak = 0;
    int weekly = 0;
    int monthly = 0;
    
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today[32], yesterday[32];
    
    snprintf(today, sizeof(today), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    t -= 24 * 3600;
    tm = *localtime(&t);
    snprintf(yesterday, sizeof(yesterday), "%04d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    
    int is_first = 1;
    char expected_date[32] = "";
    int streak_broken = 0;
    
    int capacity = 10;
    HabitStats *stats = malloc(sizeof(HabitStats) * capacity);
    if (!stats) {
        sqlite3_finalize(stmt);
        return 1;
    }
    int count = 0;

    int rc = sqlite3_step(stmt);
    
    while (rc == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char *name = sqlite3_column_text(stmt, 1);
        const unsigned char *date_str = sqlite3_column_text(stmt, 2);
        
        if (id != current_id) {
            if (current_id != -1) {
                if (count >= capacity) {
                    capacity *= 2;
                    HabitStats *temp = realloc(stats, sizeof(HabitStats) * capacity);
                    if (!temp) {
                        free(stats);
                        sqlite3_finalize(stmt);
                        return 1;
                    }
                    stats = temp;
                }
                stats[count].id = current_id;
                strncpy(stats[count].name, current_name, 255);
                stats[count].name[255] = '\0';
                stats[count].weekly_goal = current_weekly_goal;
                stats[count].monthly_goal = current_monthly_goal;
                stats[count].current_streak = streak;
                stats[count].weekly_count = weekly;
                stats[count].monthly_count = monthly;
                count++;
            }
            current_id = id;
            strncpy(current_name, (const char*)name, sizeof(current_name) - 1);
            current_weekly_goal = sqlite3_column_int(stmt, 3);
            current_monthly_goal = sqlite3_column_int(stmt, 4);
            streak = 0;
            weekly = 0;
            monthly = 0;
            is_first = 1;
            streak_broken = 0;
            strcpy(expected_date, today);
        }
        
        if (date_str) {
            int year, month, day;
            sscanf((const char*)date_str, "%d-%d-%d", &year, &month, &day);
            
            struct tm dtm = {0};
            dtm.tm_year = year - 1900;
            dtm.tm_mon = month - 1;
            dtm.tm_mday = day;
            time_t c_time = mktime(&dtm);
            
            time_t now = time(NULL);
            double diff_days = difftime(now, c_time) / (24 * 3600);
            
            if (diff_days <= 7) weekly++;
            if (diff_days <= 30) monthly++;
            
            if (!streak_broken) {
                if (is_first) {
                    if (strcmp((const char*)date_str, today) == 0) {
                        streak++;
                        // Expected next is yesterday
                        t = c_time - 24 * 3600;
                        struct tm n_tm = *localtime(&t);
                        snprintf(expected_date, sizeof(expected_date), "%04d-%02d-%02d", n_tm.tm_year + 1900, n_tm.tm_mon + 1, n_tm.tm_mday);
                    } else if (strcmp((const char*)date_str, yesterday) == 0) {
                        streak++;
                        t = c_time - 24 * 3600;
                        struct tm n_tm = *localtime(&t);
                        snprintf(expected_date, sizeof(expected_date), "%04d-%02d-%02d", n_tm.tm_year + 1900, n_tm.tm_mon + 1, n_tm.tm_mday);
                    } else {
                        streak_broken = 1;
                    }
                } else {
                    if (strcmp((const char*)date_str, expected_date) == 0) {
                        streak++;
                        t = c_time - 24 * 3600;
                        struct tm n_tm = *localtime(&t);
                        snprintf(expected_date, sizeof(expected_date), "%04d-%02d-%02d", n_tm.tm_year + 1900, n_tm.tm_mon + 1, n_tm.tm_mday);
                    } else {
                        streak_broken = 1;
                    }
                }
            }
            is_first = 0;
        }
        
        rc = sqlite3_step(stmt);
    }
    
    if (current_id != -1) {
        if (count >= capacity) {
            capacity *= 2;
            HabitStats *temp = realloc(stats, sizeof(HabitStats) * capacity);
            if (!temp) {
                free(stats);
                sqlite3_finalize(stmt);
                return 1;
            }
            stats = temp;
        }
        stats[count].id = current_id;
        strncpy(stats[count].name, current_name, 255);
        stats[count].name[255] = '\0';
        stats[count].weekly_goal = current_weekly_goal;
        stats[count].monthly_goal = current_monthly_goal;
        stats[count].current_streak = streak;
        stats[count].weekly_count = weekly;
        stats[count].monthly_count = monthly;
        count++;
    }
    
    sqlite3_finalize(stmt);
    
    *stats_out = stats;
    *count_out = count;
    return 0;
}

struct DateCount {
    char date_str[32];
    int count;
};

int db_heatmap(int grid[7][25]) {
    sqlite3_stmt *stmt;
    
    struct DateCount history[168];
    time_t now = time(NULL);
    for (int i = 0; i < 168; i++) {
        time_t t = now - (167 - i) * 24 * 3600;
        struct tm *tm_info = localtime(&t);
        snprintf(history[i].date_str, sizeof(history[i].date_str), "%04d-%02d-%02d", 
                 tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
        history[i].count = 0;
    }
    
    const char *sql = 
        "SELECT completion_date, COUNT(*) FROM completions "
        "WHERE completion_date >= date('now', '-168 days', 'localtime') "
        "GROUP BY completion_date;";
        
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *date_str = sqlite3_column_text(stmt, 0);
            int count = sqlite3_column_int(stmt, 1);
            if (date_str) {
                for (int i = 0; i < 168; i++) {
                    if (strcmp((const char*)date_str, history[i].date_str) == 0) {
                        history[i].count = count;
                        break;
                    }
                }
            }
        }
        sqlite3_finalize(stmt);
    } else {
        return 1;
    }
    
    for (int r = 0; r < 7; r++) {
        for (int c = 0; c < 25; c++) {
            grid[r][c] = -1;
        }
    }
    
    int today_wday = localtime(&now)->tm_wday;
    int curr_r = today_wday;
    int curr_c = 24;
    
    for (int i = 167; i >= 0; i--) {
        grid[curr_r][curr_c] = history[i].count;
        curr_r--;
        if (curr_r < 0) {
            curr_r = 6;
            curr_c--;
        }
    }
    
    return 0;
}
