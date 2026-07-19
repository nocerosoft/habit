#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "db.h"

void normalize_name(char *str) {
    // Trim leading whitespace
    char *start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    // Trim trailing whitespace
    if (strlen(str) > 0) {
        char *end = str + strlen(str) - 1;
        while (end >= str && isspace((unsigned char)*end)) {
            *end = '\0';
            end--;
        }
    }
    
    // Convert to Title Case
    int new_word = 1;
    for (char *p = str; *p; p++) {
        if (isspace((unsigned char)*p)) {
            new_word = 1;
        } else {
            if (new_word) {
                *p = toupper((unsigned char)*p);
                new_word = 0;
            } else {
                *p = tolower((unsigned char)*p);
            }
        }
    }
}

void print_usage(const char *prog_name) {
    printf("Usage:\n");
    printf("  %s record <activity>\n", prog_name);
    printf("  %s list\n", prog_name);
    printf("  %s heatmap\n", prog_name);
    printf("  %s remove <activity>\n", prog_name);
    printf("  %s goal <activity> <target> <weekly|monthly>\n", prog_name);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (db_init("habits.db") != 0) {
        return 1;
    }

    int result = 0;
    const char *command = argv[1];

    if (strcmp(command, "record") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing activity name.\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            normalize_name(argv[2]);
            RecordResult res;
            if (db_record_habit(argv[2], &res) == 0) {
                if (res.is_duplicate) {
                    printf("You already recorded '%s' today!\n\n", argv[2]);
                } else {
                    printf("[OK] Recorded '%s' for today! (Streak: %d days)\n\n", argv[2], res.streak);
                }
                
                if (res.weekly_goal > 0) {
                    if (res.weekly_count >= res.weekly_goal) {
                        printf("\x1b[32mWeekly goal reached! (%d/%d)\x1b[0m\n", res.weekly_count, res.weekly_goal);
                    } else {
                        printf("Keep going! %d more times to reach your weekly goal.\n", res.weekly_goal - res.weekly_count);
                    }
                }
                
                if (res.monthly_goal > 0) {
                    if (res.monthly_count >= res.monthly_goal) {
                        printf("\x1b[32mMonthly goal reached! (%d/%d)\x1b[0m\n", res.monthly_count, res.monthly_goal);
                    } else {
                        printf("Keep going! %d more times to reach your monthly goal.\n", res.monthly_goal - res.monthly_count);
                    }
                }
                
                if (res.weekly_goal > 0 || res.monthly_goal > 0) {
                    printf("\n");
                }
                
                printf("You have completed %d/%d habits today.\n", res.completed_today, res.total_active);
                
                if (res.completed_today < res.total_active) {
                    printf("Still left for today:\n");
                    for (int i = 0; i < res.remaining_count; i++) {
                        printf("- %s\n", res.remaining[i].name);
                    }
                } else {
                    printf("All habits completed for today!\n");
                }
                
                if (res.remaining) {
                    free(res.remaining);
                }
            } else {
                result = 1;
            }
        }
    } else if (strcmp(command, "list") == 0) {
        HabitStats *stats = NULL;
        int count = 0;
        if (db_list_habits(&stats, &count) == 0) {
            printf("%-20s | %-6s | %-7s | %-7s\n", "Habit", "Streak", "Weekly", "Monthly");
            printf("------------------------------------------------------\n");
            
            if (count == 0) {
                printf("No habits tracked yet.\n");
            } else {
                for (int i = 0; i < count; i++) {
                    char w_str[32], m_str[32];
                    int w_done = (stats[i].weekly_goal > 0 && stats[i].weekly_count >= stats[i].weekly_goal);
                    int m_done = (stats[i].monthly_goal > 0 && stats[i].monthly_count >= stats[i].monthly_goal);
                    
                    if (stats[i].weekly_goal > 0) snprintf(w_str, sizeof(w_str), "%d/%d", stats[i].weekly_count, stats[i].weekly_goal);
                    else snprintf(w_str, sizeof(w_str), "%d", stats[i].weekly_count);
                    
                    if (stats[i].monthly_goal > 0) snprintf(m_str, sizeof(m_str), "%d/%d", stats[i].monthly_count, stats[i].monthly_goal);
                    else snprintf(m_str, sizeof(m_str), "%d", stats[i].monthly_count);
                    
                    printf("%-20s | %-6d | ", stats[i].name, stats[i].current_streak);
                    
                    if (w_done) printf("\x1b[32m%-7s\x1b[0m | ", w_str);
                    else printf("%-7s | ", w_str);
                    
                    if (m_done) printf("\x1b[32m%-7s\x1b[0m\n", m_str);
                    else printf("%-7s\n", m_str);
                }
            }
            if (stats) free(stats);
        } else {
            result = 1;
        }
    } else if (strcmp(command, "heatmap") == 0) {
        int grid[7][25];
        if (db_heatmap(grid) == 0) {
            int max_val = 1;
            for (int r = 0; r < 7; r++) {
                for (int c = 0; c < 25; c++) {
                    if (grid[r][c] > max_val) max_val = grid[r][c];
                }
            }
            if (max_val < 4) max_val = 4;
            
            int t1 = (max_val * 1) / 4;
            int t2 = (max_val * 2) / 4;
            int t3 = (max_val * 3) / 4;
            
            if (t1 < 1) t1 = 1;
            if (t2 <= t1) t2 = t1 + 1;
            if (t3 <= t2) t3 = t2 + 1;

            const char *day_names[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
            printf("\nHabit Completion Heatmap (Last 24 Weeks)\n\n");
            
            for (int r = 0; r < 7; r++) {
                printf("%s ", day_names[r]);
                for (int c = 0; c < 25; c++) {
                    int count = grid[r][c];
                    if (count == -1) {
                        printf("  ");
                    } else if (count == 0) {
                        printf("\x1b[90m■\x1b[0m ");
                    } else if (count <= t1) {
                        printf("\x1b[32m■\x1b[0m ");
                    } else if (count <= t2) {
                        printf("\x1b[33m■\x1b[0m ");
                    } else if (count <= t3) {
                        printf("\x1b[34m■\x1b[0m ");
                    } else {
                        printf("\x1b[35m■\x1b[0m ");
                    }
                }
                printf("\n");
            }
            char leg1[32], leg2[32], leg3[32], leg4[32];
            if (t1 == 1) snprintf(leg1, 32, "1"); else snprintf(leg1, 32, "1-%d", t1);
            if (t2 == t1 + 1) snprintf(leg2, 32, "%d", t2); else snprintf(leg2, 32, "%d-%d", t1 + 1, t2);
            if (t3 == t2 + 1) snprintf(leg3, 32, "%d", t3); else snprintf(leg3, 32, "%d-%d", t2 + 1, t3);
            snprintf(leg4, 32, "%d+", t3 + 1);

            printf("\nLegend: \x1b[90m■\x1b[0m 0   \x1b[32m■\x1b[0m %s   \x1b[33m■\x1b[0m %s   \x1b[34m■\x1b[0m %s   \x1b[35m■\x1b[0m %s\n\n", 
                   leg1, leg2, leg3, leg4);
        } else {
            result = 1;
        }
    } else if (strcmp(command, "remove") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing activity name.\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            normalize_name(argv[2]);
            if (db_remove_habit(argv[2]) == 0) {
                printf("Removed habit '%s'.\n", argv[2]);
            } else {
                result = 1;
            }
        }
    } else if (strcmp(command, "goal") == 0) {
        if (argc < 5) {
            fprintf(stderr, "Error: Missing arguments for goal.\n");
            print_usage(argv[0]);
            result = 1;
        } else {
            normalize_name(argv[2]);
            int target = atoi(argv[3]);
            if (db_set_goal(argv[2], target, argv[4]) == 0) {
                printf("Set %s goal for '%s' to %d.\n", argv[4], argv[2], target);
            } else {
                result = 1;
            }
        }
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        print_usage(argv[0]);
        result = 1;
    }

    db_close();
    return result;
}
