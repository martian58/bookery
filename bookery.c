#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>

#include "lib/user.h"

void friendlyCLI();

/**
 * @brief Initializes the database tables if they do not exist.
 * 
 * @details This function initializes the books, users, and rents tables in the database if they do not already exist.
 * 
 * @param None.
 * 
 * @return An integer representing the status of the operation:
 *         - 0: If the operation was successful.
 *         - Non-zero: If an error occurred during initialization.
 */
int initializeDatabase() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    // Open database connection.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return return_code;
    }

    // SQL statement to create books table.
    const char *sql_books = "CREATE TABLE IF NOT EXISTS books ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "title TEXT NOT NULL,"
                            "author TEXT NOT NULL,"
                            "genre TEXT,"
                            "price REAL,"
                            "quantity_available INTEGER,"   
                            "quantity_rented INTEGER,"
                            "quantity_sold INTEGER,"
                            "quantity_rented_all INTEGER,"
                            "quantity_rented_days INTEGER"
                            ");";

    // Execute SQL statement to create books table.
    return_code = sqlite3_exec(db, sql_books, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        // return return_code;
    }

    // SQL statement to create users table.
    const char *sql_users = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "username TEXT NOT NULL,"
                            "password TEXT NOT NULL,"
                            "email TEXT NOT NULL,"
                            "role INTEGER NOT NULL" 
                            ");";

    // Execute SQL statement to create users table.
    return_code = sqlite3_exec(db, sql_users, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL Error: %s\n", errMsg);
        sqlite3_free(errMsg);
        // return return_code;
    }

    // SQL statement to create rents table.
    const char *sql_rents = "CREATE TABLE IF NOT EXISTS rents ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "title TEXT NOT NULL,"  
                            "Name TEXT NOT NULL,"
                            "Phone TEXT NOT NULL,"
                            "quantity_rented INTEGER,"
                            "rented_for_days INTEGER,"
                            "rent_date TEXT NOT NULL,"
                            "return_date TEXT"
                            ");";

    // Execute SQL statement to create rents table.
    return_code = sqlite3_exec(db, sql_rents, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        return return_code;
    }

    // Close database connection.
    sqlite3_close(db);
    return 0;
}


//***********************************************************************************************************************************

/**
 *@brief Generate a sales report including top 5 books and total revenue.
 *@param None.
 *@return void.
*/
void generateSalesReport() {
    sqlite3 *db;  // SQLite database object.
    sqlite3_stmt *stmt;  // SQLite statement object.
    int return_code;  // Return code for SQLite functions.

    // Open the database connection.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Print header for the sales report.
    printf("\n%s************ Sales Report ************%s\n\n",PINK,RESET);
    printf("\n%s********* Top 5 Books *********%s\n",YELLOW,RESET);

    // SQL query to retrieve top 5 books based on quantity sold.
    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books ORDER BY quantity_sold DESC LIMIT 5;";
    const char *sql2 = "SELECT price, quantity_sold FROM books;";  // SQL query to retrieve price and quantity sold for all books.

    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column.
    int max_title_width = 0;
    int max_author_width = 0;
    int max_genre_width = 0;
    double max_price = 0.0;
    int max_qty_sold = 0;

    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    // Print horizontal separator line.
    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 53); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers.
    printf("%s%-*s | %-*s | %-*s | %-10s | %-13s | %-13s | %s\n",
           BLUE,
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Price",
           "Quantity Sold",
           "Revenue",
           RESET);

    // Print horizontal separator line.
    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 53); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print sales report data.
    sqlite3_reset(stmt); // Reset the statement to re-execute.
    float totalRevenueTop_5 = 0;
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *title = (const char *)sqlite3_column_text(stmt, 0);
        const char *author = (const char *)sqlite3_column_text(stmt, 1);
        const char *genre = (const char *)sqlite3_column_text(stmt, 2);
        double price = sqlite3_column_double(stmt, 3);
        int quantitySold = sqlite3_column_int(stmt, 6);
        float revenue = price * quantitySold;
        printf("%-*s | %-*s | %-*s | $%-9.2f | %-13d | $%-12.2f |\n",
               max_title_width, title,
               max_author_width, author,
               max_genre_width, genre,
               price,
               quantitySold,
               revenue);
        totalRevenueTop_5 += revenue;
        for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 53); i++) {
            printf("-");
        }
        printf("\n");
    }

    float totalRevenue = 0;

    // Retrieve total revenue for all books.
    return_code = sqlite3_prepare_v2(db, sql2, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
       totalRevenue+= sqlite3_column_double(stmt, 0) * sqlite3_column_int(stmt, 1);
    }

    // Print total revenue.
    printf("\n%s*********** Revenue ***********%s\n\n",YELLOW,RESET);
    printf("Total Revenue of Top 5: %s$%.2f%s\n",GREEN, totalRevenueTop_5,RESET);
    printf("Total Revenue of All:   %s$%.2f%s\n\n",GREEN, totalRevenue,RESET);

    sqlite3_finalize(stmt);  // Finalize the statement.
    sqlite3_close(db);  // Close the database connection.
}

/**
  @brief Generate a rental report including top 5 rented books.
  @param None.
  @return void.
*/
void generateRentalReport() {
    sqlite3 *db;  // SQLite database object.
    sqlite3_stmt *stmt;  // SQLite statement object.
    int return_code;  // Return code for SQLite functions.

    // Open the database connection.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Print header for the rental report.
    printf("\n%s*********** Rental Report ************%s\n\n",PINK,RESET);
    printf("\n%s******* Top 5 Rented Books *********%s\n",YELLOW,RESET);

    // SQL query to retrieve top 5 rented books based on total quantity rented.
    const char *sql = "SELECT title, author, genre, quantity_rented_all, quantity_rented_days FROM books ORDER BY quantity_rented_all DESC LIMIT 5;";

    // Prepare the SQL statement.
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column.
    int max_title_width = 0;
    int max_author_width = 0;
    int max_genre_width = 0;
    int max_qty_rented = 0;

    // Iterate over the result set to find maximum widths.
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_double(stmt, 5));
    }

    // Print horizontal separator line.
    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 53); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers.
    printf("%s%-*s | %-*s | %-*s | %-13s | %-14s |%s\n",
           BLUE,
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Quantity Rented All",
           "Quantity Rented Days",
           RESET);

    // Print horizontal separator line.
    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 53); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    float totalRevenue;
    // Print rental report data.
    sqlite3_reset(stmt); // Reset the statement to re-execute.
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *title = (const char *)sqlite3_column_text(stmt, 0);
        const char *author = (const char *)sqlite3_column_text(stmt, 1);
        const char *genre = (const char *)sqlite3_column_text(stmt, 2);
        int quantityRentedAll = sqlite3_column_int(stmt, 3);
        int quantityRentedDays = sqlite3_column_int(stmt, 4);
        printf("%-*s | %-*s | %-*s | %-19d | %-20d |\n",
               max_title_width, title,
               max_author_width, author,
               max_genre_width, genre,
               quantityRentedAll,
               quantityRentedDays);     
        totalRevenue+=quantityRentedDays;
        for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 53); i++) {
            printf("-");
        }
        printf("\n");
    }
    // Print total revenue.
    printf("\n%s*********** Revenue ***********%s\n\n",YELLOW,RESET);
    printf("Total Revenue of All:   %s$%.2f%s\n\n",GREEN, totalRevenue,RESET);

    sqlite3_finalize(stmt);  // Finalize the statement.
    sqlite3_close(db);  // Close the database connection.
}


void advancedCLI() {
    printf("\033c");
    char command[100];
    
    do {
        printf("bms-> ");
        fgets(command, sizeof(command), stdin); 
        
        // Remove newline character if present.
        if (strlen(command) > 0 && command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }
        
        if (strcmp(command, "add user") == 0) {
            // Call function to add user.
            addUser();

        } else if (strcmp(command, "add book") == 0) {
            // Call function to add book.
            addBook();

        } else if (strcmp(command, "login") == 0) {
            // Call function to login.
            authenticateUser();

        } else if (strcmp(command, "update book") == 0) {
            // Call function to update book.
            updateBook();

        } else if (strcmp(command, "update user") == 0) {
            // Call function to update user.
            updateUser();  

        } else if (strcmp(command, "sell book") == 0) {
            // Call function to sell book.
            sellBook();

        } else if (strcmp(command, "del user") == 0) {
            // Call function to delete user.
            delUser();

        } else if (strcmp(command, "del book") == 0) {
            // Call function to delete book.
            delBook(1);

        } else if (strcmp(command, "del allbooks") == 0) {
            // Call function to delete all books.
            delBook(0);

        } else if (strcmp(command, "rent book") == 0){
            // Call function to rent books.
            rentBook();

        } else if (strcmp(command, "rent recall") == 0){
            // Call function to recall rented books.
            rentRecall();  

        } else if (strcmp(command, "rent late") == 0){
            // Call function to recall rented books.
            rentLate();   
        } 
        else if (strcmp(command, "show rents") == 0){
            // Call function to display rents.
            displayRent();

        } else if (strcmp(command, "show books") == 0) {
            // Call function to display all books.
            displayBooks();

        } else if (strcmp(command, "show users") == 0) {
            // Call function to display all users.
            displayUsers();

        } else if (strcmp(command, "search rent") == 0){
            // Call function to search rented books.
            searchRent();

        } else if (strcmp(command, "search book") == 0) {
            // Call function to search for books.
            searchBook();

        } else if (strcmp(command, "report sales") == 0) {
            // Call function to generate sales report.
            generateSalesReport();

        } else if (strcmp(command, "report rents") == 0) {
            // Call function to generate rental report.
            generateRentalReport();

        } else if (strcmp(command, "whoami") == 0) {
            // Call function to display current user information.
            whoami();

        } else if (strcmp(command, "help add") == 0) {
            // Display help for add commands.
            help("add");

        } else if (strcmp(command, "help del") == 0) {
            // Display help for del commands.
            help("del");

        } else if (strcmp(command, "help show") == 0) {
            // Display help for show commands.
            help("show");

        } else if (strcmp(command, "help login") == 0) {
            // Display help for login command.
            help("login");

        } else if (strcmp(command, "help search") == 0) {
            // Display help for search commands.
            help("search");

        } else if (strcmp(command, "help update") == 0) {
            // Display help for update commands.
            help("update");

        } else if (strcmp(command, "help sell") == 0) {
            // Display help for sell command.
            help("sell");

        } else if (strcmp(command, "help rent") == 0) {
            // Display help for rent command.
            help("rent");
            
        } else if (strcmp(command, "help report") == 0) {
            // Display help for rent command.
            help("report");

        } else if (strcmp(command, "help whoami") == 0) {
            // Display help for rent command.
            help("whoami");

        } else if (strcmp(command, "help clear") == 0) {
            // Display help for rent command.
            help("clear");
        }  
        else if (strcmp(command, "search") == 0) {
            // Display help for search command.
            help("search");

        } else if (strcmp(command, "update") == 0) {
            // Display help for update command.
            help("update");

        } else if (strcmp(command, "rent") == 0) {
            // Display help for rent command.
            help("rent");

        } else if (strcmp(command, "del") == 0) {
            // Display help for del command.
            help("del");
            
        } else if (strcmp(command, "add") == 0) {
            // Display help for add command.
            help("add");

        } else if (strcmp(command, "show") == 0) {
            // Display help for show command.
            help("show");

        } else if (strcmp(command, "sell") == 0) {
            // Display help for sell command.
            help("sell");

        } else if (strcmp(command, "report") == 0) {
            // Display help for report command.
            help("report");
        }
         else if (strcmp(command, "help") == 0) {
            // Display general help.
            help("all");

        } else if (strcmp(command, "back") == 0) {
            // Return to the friendly CLI.
            friendlyCLI();

        } else if (strcmp(command, "clear") == 0) {
            // Clear the screen and reset the advanced CLI.
            advancedCLI();

        } else if (strcmp(command, "exit") == 0) {
            // Clear the screen and reset the advanced CLI.
            printf("\n\nbye!\n");
            exit(0);
        } 
        else {
            // Invalid command.
            if (strlen(command) > 0) {
                printf("%sInvalid command:%s %s\n",RED,RESET,command);
            }
        }
    } while (true);
}

/**
 * @brief Main function for the friendly command-line interface (CLI) of the book management system.
 * 
 * This function provides a user-friendly interface for interacting with the book management system.
 * Users can perform various operations such as adding, displaying, updating, selling, and renting books,
 * as well as generating sales and rental reports.
 */
void friendlyCLI(){
    printf("\033c"); // Clear the screen.
    
    // Initialize database.
    if (initializeDatabase() != 0) {
        fprintf(stderr, "Failed to initialize database.\n");
    }

    int choice;
    
    do {
        printMenu(); // Display menu options.
        scanf("%d", &choice); // Get user choice.
        
        switch(choice) {
            case 1:
                addBook();
                break;
            case 2:
                displayBooks();
                break;
            case 3:
                searchBook();
                break;
            case 4:
                updateBook();
                break;
            case 5:
                sellBook();
                break;
            case 6:
                generateSalesReport();
                break;
            case 7:
                rentBook();
                break;
            case 8:
                rentRecall();
                break;
            case 9:
                displayRent();
                break;
            case 10:
                rentLate();
                break;
            case 11:
                searchRent();
                break;
            case 12:
                generateRentalReport();
                break;
            case 13:
                advancedCLI();
                break;
            case 0:
                printf("Exiting program. Goodbye!\n");
                exit(0);    
                break;
            default:
                printf("%sInvalid choice.%s Please try again.\n",RED,RESET);
        }

    } while(choice != 0);
}

int bms() {
    login();
    friendlyCLI();
}
int main(){
    bms();
    return 0;
}