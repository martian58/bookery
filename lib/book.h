/*
 * File:          book.h
 * Authors:       Fuad Alizada, Mehdi Hasanli, Toghrul Abdullazada, Tural Gadirov, Ilham Bakhishov
 * Date:          May 09, 2024
 * Description:   File contains functions for managing the books inventory. 
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>

#include "const.h"


// Define structure for a book.
struct Book {
    char title[MAX_TITLE_LENGTH];
    char author[MAX_AUTHOR_LENGTH];
    char genre[MAX_GENRE_LENGTH];
    float price;
    int quantity_available;
    int quantity_rented;
    int quantity_sold;
};

struct Rent {
    int id;
    char title[MAX_TITLE_LENGTH];
    char customer_name[MAX_AUTHOR_LENGTH];
    char customer_phone[MAX_AUTHOR_LENGTH];
    int quantity_rented;
    int rented_for_days;
    char rented_date[MAX_AUTHOR_LENGTH];
    char return_date[MAX_AUTHOR_LENGTH];
};


//************************************************************************************************************************************************

/**
 * @brief Adds a new book to the database.
 *
 * Prompts the user to input details for a new book (title, author, genre, price, quantity available),
 * validates the input, and inserts the new book into the database.
 */
void addBook() {
    sqlite3 *db;            ///< SQLite database object.
    char *errMsg = 0;       ///< Error message string.
    int return_code;        ///< Return code from SQLite functions.

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    struct Book newBook;    ///< Structure to store details of the new book.

    // Input validation loop for title.
    do {
        printf("Enter title: ");
        scanf(" %[^\n]", newBook.title);
    } while (!validateTitle(newBook.title));

    // Input validation loop for author.
    do {
        printf("Enter author: ");
        scanf(" %[^\n]", newBook.author);
    } while (!validateAuthor(newBook.author));

    // Input validation loop for genre.
    do {
        printf("Enter genre: ");
        scanf(" %[^\n]", newBook.genre);
    } while (!validateGenre(newBook.genre));

    // Input validation loop for price.
    do {
        printf("Enter price: ");
        scanf("%f", &newBook.price);
    } while (!validatePrice(newBook.price));

    // Input validation loop for quantity available.
    do {
        printf("Enter quantity available: ");
        scanf("%d", &newBook.quantity_available);
    } while (!validateQuantity(newBook.quantity_available));
    newBook.quantity_rented = 0;
    newBook.quantity_sold = 0;

    char sql[1000]; ///< SQL query string.  
    sprintf(sql, "INSERT INTO books (title, author, genre, price, quantity_available, quantity_rented, quantity_sold, quantity_rented_all,quantity_rented_days) VALUES (?, ?, ?, ?, ?, ?, ?, 0, 0);");

    sqlite3_stmt *stmt;
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, newBook.title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, newBook.author, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, newBook.genre, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, newBook.price);
    sqlite3_bind_int(stmt, 5, newBook.quantity_available);
    sqlite3_bind_int(stmt, 6, newBook.quantity_rented);
    sqlite3_bind_int(stmt, 7, newBook.quantity_sold);

    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("%sBook added successfully.\n%s", GREEN, RESET);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}




//******************************************************************************************************************************************

/**
 * @brief Display the list of books from the database.
 * 
 * This function retrieves book information from the database.
 * and prints it in a formatted table.
 */
void displayBooks() {
    sqlite3 *db; // SQLite database connection.
    sqlite3_stmt *stmt; // SQLite statement.
    int return_code; // Return code for SQLite operations.

    // Open the SQLite database.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** List of Books **************\n");

    // SQL query to select book information.
    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books;";

    // Prepare the SQL statement.
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column.
    int max_title_width = 6;
    int max_author_width = 6;
    int max_genre_width = 0;
    double max_price = 0.0;
    int max_qty_available = 0;
    int max_qty_rented = 0;
    int max_qty_sold = 0;

    // Iterate through the result set to find maximum widths.
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    // Print separator line.
    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers.
    printf("%s%-*s | %-*s | %-*s | %-10s | %-15s | %-18s | %s |%s\n",
        BLUE,
        max_title_width, "Title",
        max_author_width, "Author",
        max_genre_width, "Genre",
        "Price",
        "Quantity Available",
        "Quantity Rented",
        "Quantity Sold",
        RESET);

    // Print separator line.
    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Reset the statement to re-execute.
    sqlite3_reset(stmt);

    // Print book data.
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-*s | %-*s | %-*s | $%-9.2f | %-18d | %-18d | %-13d |\n",
            max_title_width, (const char *)sqlite3_column_text(stmt, 0),
            max_author_width, (const char *)sqlite3_column_text(stmt, 1),
            max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
            sqlite3_column_double(stmt, 3),
            sqlite3_column_int(stmt, 4),
            sqlite3_column_int(stmt, 5),
            sqlite3_column_int(stmt, 6));
        // Print separator line.
        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
            printf("-");
        }
        printf("\n");
    }

    // Finalize the statement.
    sqlite3_finalize(stmt);
    // Close the database connection.
    sqlite3_close(db);
}


//**********************************************************************************************************************************************

    // Search book

    /**
 * @brief Search for books in the database based on a search term (title, author, or genre).
 * 
 * This function retrieves book information from a SQLite database based on a user-provided search term
 * and prints the search results in a formatted table.
 */
void searchBook() {
    sqlite3 *db; // SQLite database connection.
    sqlite3_stmt *stmt; // SQLite statement.
    int return_code; // Return code for SQLite operations.

    // Open the SQLite database.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTerm[MAX_TITLE_LENGTH];
    printf("Enter search term (title, author, or genre): ");
    scanf(" %[^\n]s", searchTerm);

    // SQL query to search for books based on the search term.
    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented,\
        quantity_sold FROM books WHERE title LIKE ? OR author LIKE ? OR genre LIKE ?;";

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
    double max_price = 0.0;
    int max_qty_available = 0;
    int max_qty_rented = 0;
    int max_qty_sold = 0;

    // Bind search term to the prepared statement.
    sqlite3_bind_text(stmt, 1, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, searchTerm, -1, SQLITE_STATIC);

    // Fetch data to calculate maximum widths.
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    // Reset the statement to re-execute.
    sqlite3_reset(stmt);

    printf("\n***** Search Results ******\n");
    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers.
    printf("%s%-*s | %-*s | %-*s | %-10s | %-15s | %-18s | %s | %s\n",
        BLUE,
        max_title_width, "Title",
        max_author_width, "Author",
        max_genre_width, "Genre",
        "Price",
        "Quantity Available",
        "Quantity Rented",
        "Quantity Sold",
        RESET);

    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
        printf("-");
    }
    printf("%s\n",RESET);
    
    // Print search results with aligned columns.
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        if(strcasestr((const char *)sqlite3_column_text(stmt,0),searchTerm) != NULL){
            printf("%s%-*s %s| %-*s | %-*s | $%-9.2f | %-18d | %-18d | %-13d |\n",
                GREEN,max_title_width, (const char *)sqlite3_column_text(stmt, 0),RESET,
                max_author_width, (const char *)sqlite3_column_text(stmt, 1),
                max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));
        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
            printf("-");
        }
        printf("\n");
        }
        else if(strcasestr((const char *)sqlite3_column_text(stmt,1),searchTerm) != NULL){
            printf("%-*s | %s%-*s %s| %-*s | $%-9.2f | %-18d | %-18d | %-13d |\n",
                max_title_width, (const char *)sqlite3_column_text(stmt, 0),
                GREEN,max_author_width, (const char *)sqlite3_column_text(stmt, 1),RESET,
                max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));

        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
            printf("-");
        }
        printf("\n");
        }
        else if(strcasestr((const char *)sqlite3_column_text(stmt,2),searchTerm) != NULL){
            printf("%-*s | %-*s | %s%-*s %s| $%-9.2f | %-18d | %-18d | %-13d | \n",
                max_title_width, (const char *)sqlite3_column_text(stmt, 0),
                max_author_width, (const char *)sqlite3_column_text(stmt, 1),
                GREEN,max_genre_width, (const char *)sqlite3_column_text(stmt, 2),RESET,
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));

        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 79);i++){
            printf("-");
        }
        printf("\n");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

//*******************************************************************************************************************************************



    /**
 * @brief Update details of a book in the database.
 * 
 * This function allows the user to update details of a book such as title, author, genre, price,
 * and quantity available in the database.
 */
void updateBook() {
    sqlite3 *db; // SQLite database connection.
    char *errMsg = 0; // Error message for SQLite operations.
    int return_code; // Return code for SQLite operations.

    // Open the SQLite database.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTitle[MAX_TITLE_LENGTH];
    // Loop until a valid title is entered.
    do {
        printf("Enter the title of the book to update: ");
        scanf(" %[^\n]s", searchTitle);
    } while (!validateTitle(searchTitle));

    struct Book updatedBook;

    // Loop until a valid title is entered.
    do {
        printf("Enter new title: ");
        scanf(" %[^\n]s", updatedBook.title);
    } while (!validateTitle(updatedBook.title));

    printf("Enter new author: ");
    scanf(" %[^\n]s", updatedBook.author);
    printf("Enter new genre: ");
    scanf(" %[^\n]s", updatedBook.genre);
    printf("Enter new price: ");
    scanf("%f", &updatedBook.price);
    printf("Enter new quantity available: ");
    scanf("%d", &updatedBook.quantity_available);

    char sql[1000];
    sprintf(sql, "UPDATE books SET title=?, author=?, genre=?, price=?, quantity_available=? WHERE title=?;");

    sqlite3_stmt *stmt;
    // Prepare the SQL statement.
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    // Bind values to the prepared statement.
    sqlite3_bind_text(stmt, 1, updatedBook.title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, updatedBook.author, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, updatedBook.genre, -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 4, updatedBook.price);
    sqlite3_bind_int(stmt, 5, updatedBook.quantity_available);
    sqlite3_bind_text(stmt, 6, searchTitle, -1, SQLITE_STATIC);

    // Execute the SQL statement.
    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("%sBook details updated successfully.\n%s", GREEN, RESET);
    }

    sqlite3_finalize(stmt); // Finalize the prepared statement.
    sqlite3_close(db); // Close the database connection.
}


//***********************************************************************************************************************************************

    // Sell Book

 /**
 * @brief Sell a specified quantity of a book from the database.
 * 
 * This function allows the user to sell a specified quantity of a book from the database.
 * It updates the quantity sold and quantity available for the specified book.
 */
void sellBook() {
    sqlite3 *db; // SQLite database connection.
    char *errMsg = 0; // Error message for SQLite operations.
    int return_code; // Return code for SQLite operations.

    // Open the SQLite database.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char sellTitle[MAX_TITLE_LENGTH];
    // Loop until a valid title is entered.
    do {
        printf("Enter the title of the book to sell: ");
        scanf(" %[^\n]s", sellTitle);
    } while (!validateTitle(sellTitle));
    
    int quantity;
    // Loop until a valid quantity is entered.
    do {
        printf("Enter quantity to sell: ");
        scanf("%d", &quantity);
    } while (!validateQuantity(quantity));

    char sql[1000];
    // Construct SQL statement to check if enough books are available.
    sprintf(sql, "SELECT quantity_available FROM books WHERE title=?;");

    // Prepare the SQL statement.
    sqlite3_stmt *stmt;
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the title parameter to the prepared statement.
    sqlite3_bind_text(stmt, 1, sellTitle, -1, SQLITE_STATIC);

    // Execute the prepared statement.
    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        int available_quantity = sqlite3_column_int(stmt, 0);
        if (available_quantity < quantity) {
            printf("%sNot enough books available to sell.%s\n",RED,RESET);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }
    } else {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Finalize the statement.
    sqlite3_finalize(stmt);

    // Construct SQL statement to update quantity sold and available.
    sprintf(sql, "UPDATE books SET quantity_sold = quantity_sold + ?, quantity_available = quantity_available - ? WHERE title=?;");

    // Prepare the SQL statement.
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the parameters to the prepared statement.
    sqlite3_bind_int(stmt, 1, quantity);
    sqlite3_bind_int(stmt, 2, quantity);
    sqlite3_bind_text(stmt, 3, sellTitle, -1, SQLITE_STATIC);

    // Execute the prepared statement.
    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("%sSale successful.\n%s", GREEN, RESET);
    }

    // Finalize the statement and close the database connection.
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}



/**
 * @brief Delete book(s) from the database.
 * 
 * This function allows the user to delete a single book or all books from the database.
 * The mode parameter determines whether to delete a single book (mode = 1) or all books (mode = 0).
 */
void delBook(int mode) {
    if (userRole != 0) {
        printf("%sYou don't have permission for this action!\n This incident will be reported.\n%s", RED, RESET);
    } else {
        sqlite3 *db; // SQLite database connection.
        char *errMsg = 0; // Error message for SQLite operations.
        int return_code; // Return code for SQLite operations.

        // Open the SQLite database.
        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }
        if (mode == 1) {
            
            char del_book[MAX_TITLE_LENGTH];
            // Loop until a valid book title is entered.
            do {
                printf("Enter the book to delete: ");
                scanf(" %[^\n]s", del_book);
            } while (!validateUsername(del_book));

            char sql[1000];
            // Construct SQL statement to delete a single book.
            sprintf(sql, "DELETE FROM books WHERE title=?;");
            
            sqlite3_stmt *stmt;
            // Prepare the SQL statement.
            sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
            // Bind the book title to the prepared statement.
            sqlite3_bind_text(stmt, 1, del_book, -1, SQLITE_STATIC);

            // Execute the SQL statement.
            return_code = sqlite3_step(stmt);
            if (return_code != SQLITE_DONE) {
                fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            } else {
                printf("%sBook deleted successfully.\n%s", GREEN, RESET);
            } 

            sqlite3_finalize(stmt); // Finalize the prepared statement.
            sqlite3_close(db); // Close the database connection.
        } else if (mode == 0) {
            char choice[10];
            printf("%sDelete all books(yes/no): %s", YELLOW, RESET);
            fgets(choice, sizeof(choice), stdin); 
            
            // Remove newline character if present.
            if (strlen(choice) > 0 && choice[strlen(choice) - 1] == '\n') {
                choice[strlen(choice) - 1] = '\0';
            }

            if (strcmp(choice, "yes") == 0) {
                char sql[1000];
                // Construct SQL statement to delete all books.
                sprintf(sql, "DELETE FROM books;");
                
                sqlite3_stmt *stmt;
                // Prepare the SQL statement.
                sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

                // Execute the SQL statement.
                return_code = sqlite3_step(stmt);
                if (return_code != SQLITE_DONE) {
                    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
                } else {
                    printf("%sAll books deleted successfully.\n%s", GREEN, RESET);
                }
                sqlite3_finalize(stmt); // Finalize the prepared statement.
                sqlite3_close(db); // Close the database connection.
            } else {
                printf("%sDeletion aborted.\n%s", RED, RESET);
            }
        }
    }
}


//****************************************************************************************************************************************
    
/**
 * @brief Function to rent a book and update the database accordingly.
 * 
 * This function connects to the SQLite database, prompts the user to enter information about the book rental,
 * validates the input, calculates the return date based on the current date and rental duration,
 * updates the database with the rental details, and closes the database connection.
 * 
 * @return void
 */
void rentBook() {
    sqlite3 *db; // SQLite database pointer
    char *errMsg = 0; // Error message pointer
    int return_code; // Return code from SQLite functions
    int quantity = 1;   

    // Open the SQLite database
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        // If opening the database fails, print error message, close the database, and return
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Structure to hold information about the new rental
    struct Rent newRent;

    // Get the current date
    char current_date[11]; // 10 characters for the date + 1 for the null terminator
    time_t t = time(NULL);
    struct tm *today = localtime(&t);
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", today); // Format as YYYY-MM-DD

    // Set the rented date to the current date
    strcpy(newRent.rented_date, current_date);

    // Prompt user to enter the title of the book to rent and validate it
    do {
        printf("Enter the title of the book to rent: ");
        scanf(" %[^\n]s", newRent.title);
    } while (!validateTitle(newRent.title));
    char sql[1000];
    // Construct SQL statement to check if enough books are available.
    sprintf(sql, "SELECT quantity_available FROM books WHERE title=?;");

    // Prepare the SQL statement.
    sqlite3_stmt *stmt;
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the title parameter to the prepared statement.
    sqlite3_bind_text(stmt, 1, newRent.title, -1, SQLITE_STATIC);

    // Execute the prepared statement.
    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        int available_quantity = sqlite3_column_int(stmt, 0);
        if (available_quantity < quantity) {
            printf("%sNot enough books available to rent.%s\n",RED,RESET);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }
    } else {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Finalize the statement.
    sqlite3_finalize(stmt);

    // Prompt user to enter the name of the customer and validate it
    do {
        printf("Enter name of the customer: ");
        clearInputBuffer();
        fgets(newRent.customer_name, sizeof(newRent.customer_name), stdin); 
        
        // Remove newline character if present.
        if (strlen(newRent.customer_name) > 0 && newRent.customer_name[strlen(newRent.customer_name) - 1] == '\n') {
            newRent.customer_name[strlen(newRent.customer_name ) - 1] = '\0';
        }
    } while (!validateUsername(newRent.customer_name));

    // Prompt user to enter the phone number of the customer
    do{
        printf("Enter phone number of customer: ");
        scanf("%s", newRent.customer_phone);
        if(!validatePhone(newRent.customer_phone)){
            printf("%sWrong phone number format.\n%s",RED,RESET);  
        }
    }while(!validatePhone(newRent.customer_phone));

    
    // Prompt user to enter the number of days to rent and validate it
    do {
        printf("Enter number of days to rent: ");
        scanf("%d", &newRent.rented_for_days);
    } while (!validateDays(newRent.rented_for_days));

    // Calculate the return date based on the rented date and rental duration
    today->tm_mday += newRent.rented_for_days;
    mktime(today);
    char return_date[11]; 
    strftime(return_date, sizeof(return_date), "%Y-%m-%d", today); // Format as YYYY-MM-DD
    strcpy(newRent.return_date, return_date); // Set the return date in the new rental information
    newRent.quantity_rented = 1; // Set the quantity rented to 1

    // SQL statements to update the books table and insert a new rental record into the rents table
    char sql1[1000];
    char sql2[1000];
    sprintf(sql1, "UPDATE books SET quantity_rented = quantity_rented + 1, quantity_available = quantity_available - 1, quantity_rented_all = quantity_rented_all + 1, quantity_rented_days = quantity_rented_days + ?     WHERE title=?;");
    sprintf(sql2, "INSERT INTO rents (title, Name, Phone, quantity_rented, rented_for_days, rent_date, return_date) VALUES (?, ?, ?, ?, ?, ?, ?);");

    // Prepare the SQL statements
    sqlite3_stmt *stmt1, *stmt2;
    return_code = sqlite3_prepare_v2(db, sql1, -1, &stmt1, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    return_code = sqlite3_prepare_v2(db, sql2, -1, &stmt2, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt1);
        sqlite3_close(db);
        return;
    }

    // Bind parameters for the first SQL statement
    sqlite3_bind_int(stmt1, 1, newRent.rented_for_days);
    sqlite3_bind_text(stmt1, 2, newRent.title, -1, SQLITE_STATIC);

    // Bind parameters for the second SQL statement
    sqlite3_bind_text(stmt2, 1, newRent.title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt2, 2, newRent.customer_name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt2, 3, newRent.customer_phone, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt2, 4, newRent.quantity_rented);
    sqlite3_bind_int(stmt2, 5, newRent.rented_for_days);
    sqlite3_bind_text(stmt2, 6, newRent.rented_date, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt2, 7, newRent.return_date, -1, SQLITE_STATIC);

    // Execute the prepared SQL statements
    return_code = sqlite3_step(stmt1);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt1);
        sqlite3_finalize(stmt2);
        sqlite3_close(db);
        return;
    }

    return_code = sqlite3_step(stmt2);
    if (return_code != SQLITE_DONE) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        printf("%sBook rented successfully for %d days.\n%s", GREEN, newRent.rented_for_days, RESET);
    }

    // Finalize the statements and close the database connection
    sqlite3_finalize(stmt1);
    sqlite3_finalize(stmt2);
    sqlite3_close(db);
}



//**********************************************************************************************************************

/**
 * @brief Function to display the list of rented books.
 * 
 * This function connects to the SQLite database, retrieves the list of rented books from the rents table,
 * calculates the maximum widths for each column, prints column headers and rent data,
 * and closes the database connection.
 * 
 * @return void
 */
void displayRent() {
    sqlite3 *db; // SQLite database pointer
    sqlite3_stmt *stmt; // SQLite statement pointer
    int return_code; // Return code from SQLite functions

    // Open the SQLite database
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        // If opening the database fails, print error message, close the database, and return
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** List of Rents **************\n");

    // SQL query to select rent information
    const char *sql = "SELECT id, title, Name, Phone, quantity_rented, rented_for_days, rent_date, return_date FROM rents;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        // If preparing the SQL statement fails, print error message, close the database, and return
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_name_width = 0;
    int max_phone_width = 0;
    int max_qty_rented = 0;
    int max_rfd_width = 17; // Assuming "Rented for Days" width is fixed
    int max_rentdate_width = 0;
    int max_returndate_width = 0;

    // Iterate through the result set to calculate maximum widths
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {  
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_name_width = fmax(max_name_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_phone_width = fmax(max_phone_width, (int)strlen((const char *)sqlite3_column_text(stmt, 3)));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 4));
        max_rentdate_width = fmax(max_rentdate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 6)));
        max_returndate_width = fmax(max_returndate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 7)));
    }

    // Print horizontal line separator
    printf("%s", BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers
    printf("%s %-7s | %-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s |%s\n",
        BLUE,"Id",
        max_title_width, "Title",
        max_name_width, "Name",
        max_phone_width, "Phone",
        "Quantity Rented",
        "Rented for Days",
        "Rent Date",
        "Return Date",
            RESET);

    // Print horizontal line separator
    printf("%s", BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
        printf("-");
    }
    printf("%s\n", RESET);

    // Print rent data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-8d | %-*s | %-*s | %-*s | %-15d | %-15d | %-18s | %-11s |\n",
            sqlite3_column_int(stmt, 0),
            max_title_width, (const char *)sqlite3_column_text(stmt, 1),
            max_name_width, (const char *)sqlite3_column_text(stmt, 2),
            max_phone_width, (const char *)sqlite3_column_text(stmt, 3),
            sqlite3_column_int(stmt, 4),
            sqlite3_column_int(stmt, 5),
            sqlite3_column_text(stmt, 6),
            sqlite3_column_text(stmt, 7));
            
        // Print horizontal line separator
        for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
            printf("-");
        }
        printf("\n");

    }

    // Finalize the SQLite statement
    sqlite3_finalize(stmt);
    // Close the database connection
    sqlite3_close(db);
}

//*******************************************************************************************************************

// Search rent

/**
 * @brief Function to search for rented books by title, customer name, or phone number.
 * 
 * This function connects to the SQLite database, prompts the user for a search term,
 * performs a search based on title, customer name, or phone number using a LIKE query,
 * prints the search results with aligned columns, and closes the database connection.
 * 
 * @return void
 */
void searchRent() {
    sqlite3 *db; // SQLite database pointer
    sqlite3_stmt *stmt; // SQLite statement pointer
    int return_code; // Return code from SQLite functions

    // Open the SQLite database
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        // If opening the database fails, print error message, close the database, and return
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTerm[MAX_TITLE_LENGTH]; // Array to store search term
    printf("Enter search term (title, name, or phone): ");
    scanf(" %[^\n]s", searchTerm); // Prompt user for search term

    // SQL query to select rent information based on title, name, or phone
    const char *sql = "SELECT id, title, Name, Phone, quantity_rented, rented_for_days, rent_date, return_date FROM rents WHERE title LIKE ? OR Name LIKE ? OR Phone LIKE ?;";
    
    // Prepare the SQL statement
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        // If preparing the SQL statement fails, print error message, close the database, and return
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_name_width = 0;
    int max_phone_width = 0;
    int max_qty_rented = 0;
    int max_rfd_width = 17; // Assuming "Rented for Days" width is fixed
    int max_rentdate_width = 0;
    int max_returndate_width = 0;

    // Bind search term to the prepared statement
    sqlite3_bind_text(stmt, 1, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, searchTerm, -1, SQLITE_STATIC);

    // Fetch data to calculate maximum widths
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_name_width = fmax(max_name_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_phone_width = fmax(max_phone_width, (int)strlen((const char *)sqlite3_column_text(stmt, 3)));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 4));
        max_rentdate_width = fmax(max_rentdate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 6)));
        max_returndate_width = fmax(max_returndate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 7)));
    }

    // Reset the statement to re-execute
    sqlite3_reset(stmt);

    // Print search results header
    printf("\n***** Search Results ******\n");
    printf("%s", BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers
    printf("%s %-7s | %-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s |%s\n",
        BLUE,"Id",
        max_title_width, "Title",
        max_name_width, "Name",
        max_phone_width, "Phone",
        "Quantity Rented",
        "Rented for Days",
        "Rent Date",
        "Return Date",
            RESET);

    printf("%s", BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
        printf("-");
    }
    printf("%s\n", RESET);
    
    // Print search results with aligned columns    
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        // Print search results based on the matching column
        if(strcmp((const char *)sqlite3_column_text(stmt,1),searchTerm) == 0){
            printf("%-8d | %s%-*s %s| %-*s | %-*s | %-15d | %-15d | %-18s | %-11s |\n",
                sqlite3_column_int(stmt, 0),
                GREEN,max_title_width, (const char *)sqlite3_column_text(stmt, 1),RESET,
                max_name_width, (const char *)sqlite3_column_text(stmt, 2),
                max_phone_width, (const char *)sqlite3_column_text(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_text(stmt, 6),
                sqlite3_column_text(stmt, 7));
                
        } else if(strcmp((const char *)sqlite3_column_text(stmt,2),searchTerm) == 0){
            printf("%-8d | %-*s | %s%-*s %s| %-*s | %-15d | %-15d | %-18s | %-11s |\n",
                sqlite3_column_int(stmt,0),
                max_title_width, (const char *)sqlite3_column_text(stmt, 1),
                GREEN,max_name_width, (const char *)sqlite3_column_text(stmt, 2),RESET,
                max_phone_width, (const char *)sqlite3_column_text(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_text(stmt, 6),
                sqlite3_column_text(stmt, 7));
        } else if(strcmp((const char *)sqlite3_column_text(stmt,3),searchTerm) == 0){
            printf("%-8d | %-*s | %-*s | %s%-*s %s| %-15d | %-15d | %-18s | %-11s | \n",
                sqlite3_column_int(stmt,0),
                max_title_width, (const char *)sqlite3_column_text(stmt, 1),
                max_name_width, (const char *)sqlite3_column_text(stmt, 2),
                GREEN,max_phone_width, (const char *)sqlite3_column_text(stmt, 3),RESET,
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_text(stmt, 6),
                sqlite3_column_text(stmt, 7));
        }
        // Print horizontal line separator
        for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
            printf("-");
        }
        printf("\n");
    }

    // Finalize the SQLite statement
    sqlite3_finalize(stmt);
    // Close the database connection
    sqlite3_close(db);
}

    //************************************************************************************************************

    //Recall a rent


    /**
 * @brief Function to recall a rented book by its ID.
 * 
 * This function connects to the SQLite database, prompts the user for the ID of the rent to recall,
 * retrieves the title of the rented book corresponding to the given ID, updates the book's quantity_rented
 * and quantity_available in the books table, deletes the rent record from the rents table, and closes the
 * database connection.
 * 
 * @return void
 */
void rentRecall() {
    sqlite3 *db; // SQLite database pointer
    sqlite3_stmt *stmt; // SQLite statement pointer
    char *errMsg = 0; // Error message pointer
    int return_code; // Return code from SQLite functions
    int id; // ID of the rent to recall
    char title[MAX_TITLE_LENGTH]; // Array to store the title of the rented book

    // Open the SQLite database
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        // If opening the database fails, print error message, close the database, and return
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Prompt the user for the ID of the rent to recall
    do {
        printf("Enter the id of the rent to recall: ");
        scanf("%d", &id);
    } while (!validateID(id)); // Assume validateID validates against valid ID range in the database

    char sql[1000]; // Array to store SQL query
    // SQL query to select the title of the rented book corresponding to the given ID
    sprintf(sql, "SELECT title FROM rents WHERE id=?;");

    // Prepare the SQL statement
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        // If preparing the SQL statement fails, print error message, close the database, and return
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the ID parameter to the prepared statement
    sqlite3_bind_int(stmt, 1, id);

    // Execute the SQL statement
    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        // If a row is fetched, copy the title of the rented book
        strcpy(title, (const char *)sqlite3_column_text(stmt, 0));
    } else {
        // If no row is fetched, print error message, finalize the statement, close the database, and return
        fprintf(stderr, "%sNo rent found with id %d%s\n",RED, id, RESET);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    char sql2[1000]; // Array to store SQL query for updating book quantity
    char sql3[1000]; // Array to store SQL query for deleting rent record
    // SQL query to update book quantity_rented and quantity_available
    sprintf(sql2, "UPDATE books SET quantity_rented = quantity_rented - 1, quantity_available = quantity_available + 1 WHERE title=?;");
    // SQL query to delete the rent record corresponding to the given ID
    sprintf(sql3, "DELETE FROM rents WHERE id=?;");

    // Prepare the SQL statements
    return_code = sqlite3_prepare_v2(db, sql2, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        // If preparing the SQL statement fails, print error message, close the database, and return
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the title parameter to the prepared statement
    sqlite3_bind_text(stmt, 1, title, -1, SQLITE_STATIC);

    // Execute the SQL statement to update book quantity
    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        // If executing the SQL statement fails, print error message, finalize the statement, close the database, and return
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    // Prepare the SQL statement
    return_code = sqlite3_prepare_v2(db, sql3, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        // If preparing the SQL statement fails, print error message, close the database, and return
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind the ID parameter to the prepared statement
    sqlite3_bind_int(stmt, 1, id);

    // Execute the SQL statement to delete the rent record
    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        // If executing the SQL statement fails, print error message, finalize the statement, close the database, and return
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);

    // Print success message
    printf("%sRent recalled successfully.\n%s", GREEN, RESET);

    // Close the database connection
    sqlite3_close(db);
}

void rentLate() {
    sqlite3 *db; // SQLite database pointer
    sqlite3_stmt *stmt; // SQLite statement pointer
    int return_code; // Return code from SQLite functions

    // Open the SQLite database
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        // If opening the database fails, print error message, close the database, and return
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** Late Rents **************\n\n"); 

    // SQL query to select late rent information
    const char *sql = "SELECT id, title, Name, Phone, quantity_rented, rented_for_days, rent_date, return_date FROM rents WHERE date(return_date) < date('now');";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        // If preparing the SQL statement fails, print error message, close the database, and return
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_name_width = 0;
    int max_phone_width = 0;
    int max_qty_rented = 0;
    int max_rfd_width = 17; // Assuming "Rented for Days" width is fixed
    int max_rentdate_width = 0;
    int max_returndate_width = 0;

    // Iterate through the result set to calculate maximum widths
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_name_width = fmax(max_name_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_phone_width = fmax(max_phone_width, (int)strlen((const char *)sqlite3_column_text(stmt, 3)));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 4));
        max_rentdate_width = fmax(max_rentdate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 6)));
        max_returndate_width = fmax(max_returndate_width, (int)strlen((const char *)sqlite3_column_text(stmt, 7)));
    }

    // Print horizontal line separator
    printf("%s", BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers
    printf("%s %-7s | %-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s |%s\n",
        BLUE,"Id",
        max_title_width, "Title",
        max_name_width, "Name",
        max_phone_width, "Phone",
        "Quantity Rented",
        "Rented for Days",
        "Rent Date",
        "Return Date",
            RESET);

    // Print horizontal line separator
    printf("%s", BLUE);
    for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
        printf("-");
    }
    printf("%s\n", RESET);

    // Print late rent data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-8d | %-*s | %-*s | %-*s | %-15d | %-15d | %-18s | %s%-11s%s |\n",
            sqlite3_column_int(stmt, 0),
            max_title_width, (const char *)sqlite3_column_text(stmt, 1),
            max_name_width, (const char *)sqlite3_column_text(stmt, 2),
            max_phone_width, (const char *)sqlite3_column_text(stmt, 3),
            sqlite3_column_int(stmt, 4),
            sqlite3_column_int(stmt, 5),
            sqlite3_column_text(stmt, 6),RED,
            sqlite3_column_text(stmt, 7),RESET);
            
        // Print horizontal line separator
        for(int i =0;i < (max_title_width + max_name_width + max_phone_width + 90);i++){
            printf("-");
        }
        printf("\n");

    }

    // Finalize the SQLite statement
    sqlite3_finalize(stmt);
    // Close the database connection
    sqlite3_close(db);
}




