#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 
#include <sqlite3.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <openssl/evp.h>


#define DATABASE_FILE "bookshop.db"
#define MAX_TITLE_LENGTH 100
#define MAX_AUTHOR_LENGTH 100
#define MAX_GENRE_LENGTH 50
#define SHA256_DIGEST_LENGTH 32
int userRole;
char userName[50];

char *RED = "\033[1;31m"; 
char *GREEN = "\033[1;32m"; 
char *YELLOW = "\033[1;33m"; 
char *WHITE = "\033[1;37m"; 
char *PINK = "\033[1;35m"; 
char *BLUE = "\033[1;34m"; 
char *RESET = "\033[0m";

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

// Define structure for a user
struct User {
    char username[50];
    char password[100];
    char email[100];
    int role;  
};

// Function prototypes
int initializeDatabase();
void friendlyCLI();

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}



// Function to validate title
bool validateTitle(const char *title) {

    if (title[0] == '\0' || strlen(title) > MAX_TITLE_LENGTH) {
        printf("Title cannot be empty. Please try again.\n");
        return false;
    }
    return true;
}

// Function to validate author
bool validateAuthor(const char *author) {

    if (author[0] == '\0' || strlen(author) > MAX_AUTHOR_LENGTH) {
        printf("Wrong input, please try again.\n");
        return false;
    }
    return true;
}

// Function to validate genre
bool validateGenre(const char *genre) {

    if (genre[0] == '\0' || strlen(genre) > MAX_GENRE_LENGTH) {
        printf("Genre cannot be empty. Please try again.\n");
        return false;
    }
    return true;
}

// Function to validate price
bool validatePrice(float price) {
    if (price < 0) {
        printf("Price must be non-negative. Please try again.\n");
        return false;
    }
    return true;
}

// Function to validate quantity
bool validateQuantity(int quantity) {

    if (quantity < 0) {
        printf("Quantity must be non-negative. Please try again.\n");
        return false;
    }
    return true;
}

// Validate Username
bool validateUsername(const char *username) {
    // Username must not be empty and must have at least 4 characters
    if (strlen(username) <= 3   ) {
        printf("Username must be at least 4 characters.\n");
        return false;
    }
    return true;
}

bool validatePassword(const char *password) {
    // Password must not be empty and must have at least 5 characters
    if (strlen(password) <= 4) {
        printf("Password is too short, Please try again.\n");
        return false;
    }

    return true;
}

bool validateRole(const int role) {
    // Password must not be empty and must have at least 5 characters
    return ( role >= 0 && role <=1);
}

bool validateEmail(const char *email) {
    // Basic email format validation (not comprehensive)
    int at_count = 0;
    int dot_count = 0;
    for (int i = 0; email[i] != '\0'; i++) {
        if (email[i] == '@') {
            at_count++;
        } else if (email[i] == '.') {
            dot_count++;
        }
    }
    return (at_count == 1 && dot_count > 0);
}

int initializeDatabase() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    // Open database connection
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return return_code;
    }

    // SQL statement to create books table
    const char *sql_books = "CREATE TABLE IF NOT EXISTS books ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "title TEXT NOT NULL,"
                            "author TEXT NOT NULL,"
                            "genre TEXT,"
                            "price REAL,"
                            "quantity_available INTEGER,"
                            "quantity_rented INTEGER,"
                            "quantity_sold INTEGER,"
                            "rented_for_days INTEGER"
                            ");";

    // Execute SQL statement to create books table.
    return_code = sqlite3_exec(db, sql_books, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return return_code;
    }

    // SQL statement to create users table
    const char *sql_users = "CREATE TABLE IF NOT EXISTS users ("
                            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                            "username TEXT NOT NULL,"
                            "password TEXT NOT NULL,"
                            "email TEXT NOT NULL,"
                            "role INTEGER NOT NULL"
                            ");";

    // Execute SQL statement to create users table
    return_code = sqlite3_exec(db, sql_users, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL Error: %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return return_code;
    }

    // Close database connection
    sqlite3_close(db);
    return 0;
}


//************************************************************************************************************************************************
/*
 Add Book.
*/

void addBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    struct Book newBook;
    do {
        printf("Enter title: ");
        scanf(" %[^\n]", newBook.title);
    } while (!validateTitle(newBook.title));

    // Input validation loop for author
    do {
        printf("Enter author: ");
        scanf(" %[^\n]", newBook.author);
    } while (!validateAuthor(newBook.author));

    // Input validation loop for genre
    do {
        printf("Enter genre: ");
        scanf(" %[^\n]", newBook.genre);
    } while (!validateGenre(newBook.genre));

    // Input validation loop for price
    do {
        printf("Enter price: ");
        scanf("%f", &newBook.price);
    } while (!validatePrice(newBook.price));

    // Input validation loop for quantity available
    do {
        printf("Enter quantity available: ");
        scanf("%d", &newBook.quantity_available);
    } while (!validateQuantity(newBook.quantity_available));
    newBook.quantity_rented = 0;
    newBook.quantity_sold = 0;

    char sql[1000];
    sprintf(sql, "INSERT INTO books (title, author, genre, price, quantity_available, quantity_rented, quantity_sold) VALUES ('%s', '%s', '%s', %.2f, %d, %d, %d);",
            newBook.title, newBook.author, newBook.genre, newBook.price, newBook.quantity_available, newBook.quantity_rented, newBook.quantity_sold);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Book added successfully.\n");
    }

    sqlite3_close(db);
}


//******************************************************************************************************************************************

void displayBooks() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** List of Books **************\n");

    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_author_width = 0;
    int max_genre_width = 0;
    double max_price = 0.0;
    int max_qty_available = 0;
    int max_qty_rented = 0;
    int max_qty_sold = 0;

    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s |%s\n",
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
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print book data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        printf("%-*s | %-*s | %-*s | $%-5.2f | %-18d | %-18d | %-13d |\n",
               max_title_width, (const char *)sqlite3_column_text(stmt, 0),
               max_author_width, (const char *)sqlite3_column_text(stmt, 1),
               max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
               sqlite3_column_double(stmt, 3),
               sqlite3_column_int(stmt, 4),
               sqlite3_column_int(stmt, 5),
               sqlite3_column_int(stmt, 6));
        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
             printf("-");
        }
        printf("\n");
        
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

//********************************************************************************************************************************************************

// Search book

void searchBook() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTerm[MAX_TITLE_LENGTH];
    printf("Enter search term (title, author, or genre): ");
    scanf(" %[^\n]s", searchTerm);

    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books WHERE title LIKE ? OR author LIKE ? OR genre LIKE ?;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
    int max_title_width = 0;
    int max_author_width = 0;
    int max_genre_width = 0;
    double max_price = 0.0;
    int max_qty_available = 0;
    int max_qty_rented = 0;
    int max_qty_sold = 0;

    // Bind search term to the prepared statement
    sqlite3_bind_text(stmt, 1, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, searchTerm, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, searchTerm, -1, SQLITE_STATIC);

    // Fetch data to calculate maximum widths
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        max_title_width = fmax(max_title_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
        max_author_width = fmax(max_author_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        max_genre_width = fmax(max_genre_width, (int)strlen((const char *)sqlite3_column_text(stmt, 2)));
        max_price = fmax(max_price, sqlite3_column_double(stmt, 3));
        max_qty_available = fmax(max_qty_available, sqlite3_column_int(stmt, 4));
        max_qty_rented = fmax(max_qty_rented, sqlite3_column_int(stmt, 5));
        max_qty_sold = fmax(max_qty_sold, sqlite3_column_int(stmt, 6));
    }

    // Reset the statement to re-execute
    sqlite3_reset(stmt);

    printf("\n***** Search Results ******\n");
    printf("%s",BLUE);
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-15s | %-18s | %s | %s\n",
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
    for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
        printf("-");
    }
    printf("%s\n",RESET);
    
    // Print search results with aligned columns    
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        if(strcmp((const char *)sqlite3_column_text(stmt,0),searchTerm) == 0){
            printf("%s%-*s %s| %-*s | %-*s | $%-5.2f | %-18d | %-18d | %-13d |\n",
                GREEN,max_title_width, (const char *)sqlite3_column_text(stmt, 0),RESET,
                max_author_width, (const char *)sqlite3_column_text(stmt, 1),
                max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));
        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
            printf("-");
        }
        printf("\n");
        }
        else if(strcmp((const char *)sqlite3_column_text(stmt,1),searchTerm) == 0){
            printf("%-*s | %s%-*s %s| %-*s | $%-5.2f | %-18d | %-18d | %-13d |\n",
                max_title_width, (const char *)sqlite3_column_text(stmt, 0),
                GREEN,max_author_width, (const char *)sqlite3_column_text(stmt, 1),RESET,
                max_genre_width, (const char *)sqlite3_column_text(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));

        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
            printf("-");
        }
        printf("\n");
        }
        else if(strcmp((const char *)sqlite3_column_text(stmt,2),searchTerm) == 0){
            printf("%-*s | %-*s | %s%-*s %s| $%-5.2f | %-18d | %-18d | %-13d | \n",
                max_title_width, (const char *)sqlite3_column_text(stmt, 0),
                max_author_width, (const char *)sqlite3_column_text(stmt, 1),
                GREEN,max_genre_width, (const char *)sqlite3_column_text(stmt, 2),RESET,
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6));

        for(int i =0;i < (max_title_width + max_author_width + max_genre_width + 75);i++){
            printf("-");
        }
        printf("\n");
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
//*******************************************************************************************************************************************

/*
 Update book
*/

void updateBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char searchTitle[MAX_TITLE_LENGTH];
    printf("Enter the title of the book to update: ");
    scanf(" %[^\n]s", searchTitle);

    struct Book updatedBook;
    printf("Enter new title: ");
    scanf(" %[^\n]s", updatedBook.title);
    printf("Enter new author: ");
    scanf(" %[^\n]s", updatedBook.author);
    printf("Enter new genre: ");
    scanf(" %[^\n]s", updatedBook.genre);
    printf("Enter new price: ");
    scanf("%f", &updatedBook.price);
    printf("Enter new quantity available: ");
    scanf("%d", &updatedBook.quantity_available);

    char sql[1000];
    sprintf(sql, "UPDATE books SET title='%s', author='%s', genre='%s', price=%.2f, quantity_available=%d WHERE title='%s';",
            updatedBook.title, updatedBook.author, updatedBook.genre, updatedBook.price, updatedBook.quantity_available, searchTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Book details updated successfully.\n");
    }

    sqlite3_close(db);
}

//***********************************************************************************************************************************************

// Sell Book

void sellBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char sellTitle[MAX_TITLE_LENGTH];
    printf("Enter the title of the book to sell: ");
    scanf(" %[^\n]s", sellTitle);

    int quantity;
    printf("Enter quantity to sell: ");
    scanf("%d", &quantity);

    char sql[1000];
    sprintf(sql, "UPDATE books SET quantity_sold = quantity_sold + %d, quantity_available = quantity_available - %d WHERE title='%s';", quantity, quantity, sellTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Sale successful.\n");
    }

    sqlite3_close(db);
}

//***********************************************************************************************************************************

/*
  Genrate raport for sales.
*/
void generateSalesReport() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("\n********** Sales Report **************\n");

    const char *sql = "SELECT title, author, genre, price, quantity_available, quantity_rented, quantity_sold FROM books;";
    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Calculate maximum widths for each column
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

    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 45); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print column headers
    printf("%s%-*s | %-*s | %-*s | %-6s | %-13s | %-7s | %s\n",
           BLUE,
           max_title_width, "Title",
           max_author_width, "Author",
           max_genre_width, "Genre",
           "Price",
           "Quantity Sold",
           "Revenue",
           RESET);

    printf("%s", BLUE);
    for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 45); i++) {
        printf("-");
    }
    printf("%s\n", RESET);

    // Print sales report data
    sqlite3_reset(stmt); // Reset the statement to re-execute
    float totalRevenue = 0;
    while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char *title = (const char *)sqlite3_column_text(stmt, 0);
        const char *author = (const char *)sqlite3_column_text(stmt, 1);
        const char *genre = (const char *)sqlite3_column_text(stmt, 2);
        double price = sqlite3_column_double(stmt, 3);
        int quantitySold = sqlite3_column_int(stmt, 6);
        float revenue = price * quantitySold;
        printf("%-*s | %-*s | %-*s | $%-5.2f | %-13d | $%-6.2f\n",
               max_title_width, title,
               max_author_width, author,
               max_genre_width, genre,
               price,
               quantitySold,
               revenue);
        totalRevenue += revenue;
        for (int i = 0; i < (max_title_width + max_author_width + max_genre_width + 45); i++) {
            printf("-");
        }
        printf("\n");
    }

    printf("Total Revenue: $%.2f\n", totalRevenue);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}


//****************************************************************************************************************************************

/*
  Rent a book.
*/

void rentBook() {
    sqlite3 *db;
    char *errMsg = 0;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char rentTitle[MAX_TITLE_LENGTH];
    printf("Enter the title of the book to rent: ");
    scanf(" %[^\n]s", rentTitle);

    int days;
    printf("Enter number of days to rent: ");
    scanf("%d", &days);

    char sql[1000];
    sprintf(sql, "UPDATE books SET quantity_rented = quantity_rented + 1, quantity_available = quantity_available - 1 WHERE title='%s';", rentTitle);

    return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {
        printf("Book rented successfully for %d days.\n", days);
    }

    sqlite3_close(db);
}


//****************************************************************************************************************************************
/*
Add user.
*/

// Function to hash a password using SHA-256
void hashPassword(const char *password, unsigned char *hash) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned int md_len;

    md = EVP_sha256();
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password, strlen(password));
    EVP_DigestFinal_ex(mdctx, hash, &md_len);
    EVP_MD_CTX_free(mdctx);
}

// Function to add a user
void addUser() {
    if(userRole != 0){
        printf("You dont have permission for this action!\n this incident will be reported.\n");
    }else{
        char *passwordPtr,*password2Ptr;
        char password[100], password2[100];
        sqlite3 *db;
        char *errMsg = 0;
        int return_code;

        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        struct User newUser;
        do {
            printf("Enter username: ");
            scanf("%49s", newUser.username);
        } while (!validateUsername(newUser.username));

        do {
            passwordPtr = getpass("Enter password: ");
            strcpy(password, passwordPtr);
            password2Ptr = getpass("Enter password again: ");
            strcpy(password2, password2Ptr);
            if(strcmp(password,password2) != 0){
                printf("Passwords don't match\n");
            }else{
                strcpy(newUser.password, password);
            }

        } while (!validatePassword(newUser.password));

        do {
            printf("Enter email: ");
            scanf("%99s", newUser.email);
        } while (!validateEmail(newUser.email));

        do {
            printf("Enter role (0 for admin, 1 for regular user): ");
            scanf("%d", &newUser.role);
        } while (!validateRole(newUser.role));

        unsigned char hashed_password[SHA256_DIGEST_LENGTH];
        hashPassword(newUser.password, hashed_password);

        char hashed_password_str[SHA256_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(&hashed_password_str[i * 2], "%02x", hashed_password[i]);
        }

        char sql[1000];
        sprintf(sql, "INSERT INTO users (username, password, email, role) VALUES ('%s', '%s', '%s', %d);",
                newUser.username, hashed_password_str, newUser.email, newUser.role);

        return_code = sqlite3_exec(db, sql, 0, 0, &errMsg);
        if (return_code != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", errMsg);
            sqlite3_free(errMsg);
        } else {
            printf("User added successfully.\n");
        }

        sqlite3_close(db);
    }

}



bool authenticateUser() {
    char username[50], *password;
    char sql[1000];
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    return_code = sqlite3_open(DATABASE_FILE, &db);

    // Prompt the user to enter username and password
    printf("Enter username: ");
    scanf("%49s", username);
    password = getpass("Enter password: ");
    // scanf("%49s", password);

    unsigned char hashed_password[SHA256_DIGEST_LENGTH];
    hashPassword(password, hashed_password);

    // Convert the hashed password to a hexadecimal string
    char hashed_password_str[SHA256_DIGEST_LENGTH * 2 + 1]; // Add 1 for null terminator
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hashed_password_str[i * 2], "%02x", hashed_password[i]);
    }
    // Prepare SQL statement to query for user credentials and role
    sprintf(sql, "SELECT role FROM users WHERE username='%s' AND password='%s';", username, hashed_password_str);   

    return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (return_code != SQLITE_OK) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        return false;
    }

    // Execute the prepared statement
    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        int role = sqlite3_column_int(stmt, 0);
        printf("%sAuthentication successful!%s\n",GREEN ,RESET);
        strcpy(userName, username);
        userRole = role;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    } else {
        printf("%sIncorrect username or password.\n%s",RED,RESET);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
}

//**********************************************************************************************************************************

// Advanced CLI

void whoami(){
    if(userRole == 0){
        printf("%s : You are an admin user.\n",userName);
    }else{
        printf("%s : You are a user.\n",userName);
    }
}
void help(const char *command) {
    if (strcmp(command, "del") == 0) {
        printf("Usage: del [user/book/allbooks]\n");
        printf("Description: Delete a user, a book or all the books.\n");
    } else if (strcmp(command, "add") == 0) {
        printf("Usage: add [user/book]\n");
        printf("Description: Add a new user or a new book.\n");
    } else if (strcmp(command, "login") == 0) {
        printf("Usage: login \n");
        printf("Description: Login to another account.\n");
    } else if (strcmp(command, "update") == 0) {
        printf("Usage: update [book] \n");
        printf("Description: Update the details of a book.\n");
    }
    else if (strcmp(command, "show") == 0) {
        printf("Usage: show [users/books]\n");
        printf("Description: Display all books or users.\n");
    }else if (strcmp(command, "search") == 0) {
        printf("Usage: search [book]\n");
        printf("Description: Search for a book.\n");
    }else if (strcmp(command, "sell") == 0) {
        printf("Usage: sell [book]\n");
        printf("Description: Sell a book.\n");
    }
     else if(strcmp(command,"all") == 0){
        printf("Available commands:\n\n");
        printf("1. add user - Add a new user\n");
        printf("2. add book - Add a new book\n");
        printf("3. del user - Delete a user\n");
        printf("4. del book - Delete a book\n");
        printf("5. del allbooks - Delete all the books(%sno return%s).\n",RED,RESET);
        printf("6. show books - Display all books\n");
        printf("7. show users - Display all users\n");
        printf("7. search book - search for a book.\n");
        printf("7. update book - Update the details of a book.\n");
        printf("8. back - Go back to the previous menu\n");
        printf("9. login - Login to another account.\n");
        printf("10.exit - Exit the program\n");
    } 
    else {
        printf("Invalid command: %s\n", command);
    }
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
        
        if (strcmp(command, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
        
        if (strcmp(command, "add user") == 0) {
            // Call function to add user
            addUser();
        } else if (strcmp(command, "add book") == 0) {
            // Call function to add book
            addBook();
        } else if (strcmp(command, "login") == 0) {
            // Call function to login
            authenticateUser();
        } else if (strcmp(command, "update book") == 0) {
            // Call function to login
            updateBook();
        } else if (strcmp(command, "sell book") == 0) {
            // Call function to login
            sellBook();
        }  
        else if (strcmp(command, "login") == 0) {
            // Call function to login
            authenticateUser();
        }
        else if (strcmp(command, "del user") == 0) {
            printf("Deleting user...\n");
            // Call function to delete user
        } else if (strcmp(command, "del book") == 0) {
            printf("Deleting book...\n");
            // Call function to delete book
        } else if (strcmp(command, "help add") == 0) {
            help("add");
        } else if (strcmp(command, "help del") == 0) {
            help("del");
        } else if (strcmp(command, "help show") == 0) {
            help("show");
        } else if (strcmp(command, "help login") == 0) {
            help("login");
        } else if (strcmp(command, "help search") == 0) {
            help("search");
        }else if (strcmp(command, "help update") == 0) {
            help("update");
        }else if (strcmp(command, "help sell") == 0) {
            help("sell");
        }
        else if (strcmp(command, "search") == 0) {
            help("search");
        }else if (strcmp(command, "update") == 0) {
            help("update");
        }
        else if (strcmp(command, "login") == 0) {
            help("login ");
        } else if (strcmp(command, "whoami") == 0) {
            whoami();
        }
        else if (strcmp(command, "del") == 0) {
            help("del");
        } else if (strcmp(command, "add") == 0) {
            help("add");
        } else if (strcmp(command, "show") == 0) {
            help("show");
        } else if (strcmp(command, "sell") == 0) {
            help("sell");
        }  
        else if (strcmp(command, "help") == 0) {
            help("all");
        }
        else if (strcmp(command, "show books") == 0) {
            // Call function to display all books
            displayBooks();
        } else if (strcmp(command, "show users") == 0) {
            printf("Displaying all users...\n");
            // Call function to display all users
        }else if (strcmp(command, "search book") == 0) {
            // Call function to display all users
            searchBook();
        }
         else if (strcmp(command, "back") == 0) {
            // Call function to go back
            friendlyCLI();
        }else if (strcmp(command, "clear") == 0) {
            // Call function to go back
            advancedCLI();
        } 
        else {
            if (strlen(command) > 0) {
                printf("Invalid command: %s\n", command);
            }
        }
    } while (true);
}


//**********************************************************************************************************************************
/*
  Intro
*/

void print_colored_line(const char *line, const char *foreground_color, const char *background_color) {
    printf("\033[%s;%sm%s\033[0m\n", foreground_color, background_color, line);
}

// Intro for the Guess Num game.
void introBookery() {

    // Define color codes.
    const char *colors[] = {"34", "32", "33", "36", "35"};

    // ASCII art lines.
    const char *lines[] = {
    "██████   ██████   ██████  ██   ██ ███████ ██████  ██    ██",
    "██   ██ ██    ██ ██    ██ ██  ██  ██      ██   ██  ██  ██ ", 
    "██████  ██    ██ ██    ██ █████   █████   ██████    ████  ", 
    "██   ██ ██    ██ ██    ██ ██  ██  ██      ██   ██    ██    ",
    "██████   ██████   ██████  ██   ██ ███████ ██   ██    ██    ",  
    };

    // Print each line with color transition and a random character.
    printf("\n\n");

    for (int i = 0; i < 5; i++) {

        char line_copy[strlen(lines[i]) + 1];

        strcpy(line_copy, lines[i]);

        char rnd_char = 'O';

        for (int j = 0; line_copy[j] != '\0'; j++) {

            if (line_copy[j] == '@') {

                line_copy[j] = rnd_char;
            }
        }
        print_colored_line(line_copy, colors[i % 5], colors[(i + 1) % 5]);

        usleep(200000); // Sleep for 0.2 seconds.
    }


    printf("\n\n");
}


void printMenu() {
    printf("\n*********** Bookshop Management System ***********\n");
    printf("1. Add a Book\n");
    printf("2. Display All Books\n");
    printf("3. Search for a Book\n");
    printf("4. Update Book Details\n");
    printf("5. Sell a Book\n");
    printf("6. Generate Sales Report\n");
    printf("7. Rent a Book\n");
    printf("8. Generate Rental Report\n");
    printf("9. Advanced CLI\n");
    printf("0. Exit\n");
    printf("******************************************************\n");
    printf("Enter your choice: ");
}

void login(){
    printf("\033c");
    introBookery();
    if(authenticateUser() != true){
        exit(0);
    }
}

void friendlyCLI(){
    printf("\033c");
    // Initialize database
    if (initializeDatabase() != 0) {
        fprintf(stderr, "Failed to initialize database.\n");
    }

    int choice;
    
    do {
        printMenu();
        scanf("%d", &choice);
        
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
                // generateRentalReport();
                break;
            case 9:
                advancedCLI();
                break;
            case 10:
                addUser();
                break;
            case 0:
                printf("Exiting program. Goodbye!\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
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