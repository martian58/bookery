void displayRent() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** List of Rents **************\n");

    const char *sql = "SELECT title, Name, Phone, quantity_rented, rented_for_days, rent_date, return_date FROM rents;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_name_width = 0;
    int max_phone_width = 0;
    int max_qty_rented = 0;
    int max_rfd_width = 0;
    int max_rentdate_width = 0;
    int max_returndate_width = 0;


    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_name_width = fmax(max_name_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_phone_width = fmax(max_phone_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 3));
        max_rfd_width = fmax(max_rfd_width, (int)strlen((const char *)sqlite3_column_text(stmt, 4)));
        max_rentdate_width = fmax(max_rentdate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 5)));
        max_returndate_width = fmax(max_returndate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 6)));
    }

    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s |%s\n",
           BLUE,
           max_title_width, "Title",
           max_name_width, "Name",
           max_phone_width, "Phone"
           "Quantity Rented",
           "Rented for Days",
           "Rent Date",
           "Return Date",
            RESET);


    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print rent data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-*s | %-*s | %-*s | %d | %-18d | %-18d | %-13d |\n",
               max_title_width, (const char *)sqlite3_column_text(stmt, 0),
               max_name_width, (const char *)sqlite3_column_text(stmt, 1),
               max_phone_width, (const char *)sqlite3_column_text(stmt, 2),
               sqlite3_column_int(stmt, 3),
               sqlite3_column_text(stmt, 4),
               sqlite3_column_text(stmt, 5),
               sqlite3_column_text(stmt, 6));
        for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 75);i++){
             printf("-");
        }
        printf("\n");
        
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}