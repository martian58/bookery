/*
 * File:          user.h
 * Authors:       Fuad Alizada, Mehdi Hasanli, Toghrul Abdullazada, Tural Gadirov, Ilham Bakhishov
 * Date:          May 08, 2024
 * Description:   File contains functions necessery for managing employee accouts in the system
 *                Authentication, Login, Determination of user roles etc. 
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

#include "book.h"

// Define structure for a user.
struct User {
    char username[50];
    char password[100];
    char email[100];
    int role; // 1 for regular user, 0 for admin user.
 
};

//****************************************************************************************************************************************

/**
 * @brief Function to hash a password using SHA-256 algorithm.
 * 
 * This function takes a password as input and calculates its hash using the SHA-256 algorithm
 * provided by the OpenSSL library. The resulting hash is stored in the 'hash' array.
 * 
 * @param password The password to be hashed.
 * @param hash     The array to store the resulting hash.
 * 
 * @return void
 */
void hashPassword(const char *password, unsigned char *hash) {
    EVP_MD_CTX *mdctx;       // Message digest context.
    const EVP_MD *md;        // Message digest type.
    unsigned int md_len;     // Length of the message digest.

    // Select the SHA-256 message digest.
    md = EVP_sha256();

    // Create a new message digest context.
    mdctx = EVP_MD_CTX_new();

    // Initialize the message digest context with the selected message digest.
    EVP_DigestInit_ex(mdctx, md, NULL);

    // Update the message digest context with the password data.
    EVP_DigestUpdate(mdctx, password, strlen(password));

    // Finalize the message digest and store the resulting hash in the 'hash' array.
    EVP_DigestFinal_ex(mdctx, hash, &md_len);

    // Free the message digest context.
    EVP_MD_CTX_free(mdctx);
}

//*******************************************************************************************************************************************

/**
 * @brief Function to update user information in the database.
 * 
 * This function connects to the SQLite database, prompts the user for the username to update,
 * retrieves new username, email, and role information, validates the inputs, updates the user
 * information in the users table, and closes the database connection.
 * 
 * @return void
 */
void updateUser() {
    sqlite3 *db; // SQLite database pointer.
    char *errMsg = 0; // Error message pointer.
    int return_code; // Return code from SQLite functions.

    // Open the SQLite database.
    return_code = sqlite3_open(DATABASE_FILE, &db);
    if (return_code) {

        // If opening the database fails, print error message, close the database, and return.
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));

        sqlite3_close(db);
        return;
    }

    char searchUser[MAX_TITLE_LENGTH]; // Array to store the username to update.
    // Prompt the user for the username to update.
    do {
        printf("Enter the user to update: ");
        scanf(" %[^\n]s", searchUser);
    } while (!validateTitle(searchUser));

    struct User updatedUser; // User structure to store updated user information.

    // Prompt the user for the new username and validate it.
    do {
        printf("Enter new username: ");
        scanf(" %[^\n]s", updatedUser.username);
    } while (!validateTitle(updatedUser.username));
    // Prompt the user for the new email and validate it.
    do {
        printf("Enter new email: ");
        scanf(" %[^\n]s", updatedUser.email);
    } while (!validateEmail(updatedUser.email));
    // Prompt the user for the new role and validate it.
    do {
        printf("Enter new role: ");
        scanf("%d", &updatedUser.role);
    } while (!validateRole(updatedUser.role));

    char sql[1000]; // Array to store SQL query.
    // SQL query to update user information in the users table.
    sprintf(sql, "UPDATE users SET username=?, email=?, role=? WHERE username=?;");

    sqlite3_stmt *stmt; // SQLite statement pointer.
    // Prepare the SQL statement
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    // Bind parameters to the prepared statement.
    sqlite3_bind_text(stmt, 1, updatedUser.username, -1, SQLITE_STATIC);

    sqlite3_bind_text(stmt, 2, updatedUser.email, -1, SQLITE_STATIC);

    sqlite3_bind_int(stmt, 3, updatedUser.role);

    sqlite3_bind_text(stmt, 4, searchUser, -1, SQLITE_STATIC);

    // Execute the SQL statement
    return_code = sqlite3_step(stmt);
    if (return_code != SQLITE_DONE) {
        // If executing the SQL statement fails, print error message.
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    } else {
        // If update is successful, print success message.
        printf("%sUser updated successfully.%s\n", GREEN, RESET);
    }

    // Finalize the statement.
    sqlite3_finalize(stmt);
    // Close the database connection.
    sqlite3_close(db);
}




// Who am i

/**
 * @brief Prints the role of the current user.
 * 
 * @details This function prints whether the current user is an admin or a regular user.
 *          It uses ANSI escape codes for colored output.
 * 
 * @param None
 * 
 * @return None
 */
void whoami(){
    // Check if the user role is admin (0) or not.
    if(userRole == 0){
        // Print the user name and indicate that they are an admin.
        printf("%s : You are an %sadmin.%s\n",userName,GREEN,RESET);
    } else {
        // Print the user name and indicate that they are a regular user.
        printf("%s : You are a %suser%s.\n",userName,GREEN,RESET);
    }
}


//************************************************************************************************************************************

// Function to add users.

/**
 * @brief Adds a new user to the database.
 * 
 * @details This function prompts the admin user to input details of the new user,
 *          including username, password, email, and role. It then validates the input,
 *          hashes the password, and stores the new user's information in the database.
 *          If the current user is not an admin, permission denial message is displayed.
 * 
 * @param None
 * 
 * @return None
 */
void addUser() {
    // Check if the current user is an admin (userRole == 0)
    if(userRole != 0){
        // Display permission denial message if the current user is not an admin.
        printf("%sYou don't have permission for this action! This incident will be reported.%s\n",RED,RESET);
    } else {
        // If the current user is an admin, proceed with adding a new user.
        
        // Declare variables for password input.
        char *passwordPtr,*password2Ptr;
        char password[100], password2[100];
        
        // Declare variables for interacting with the database.
        sqlite3 *db;
        char *errMsg = 0;
        int return_code;

        // Open the database
        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code) {
            // If unable to open the database, print error message and return.
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Declare a struct to store the details of the new user.
        struct User newUser;
        
        // Prompt the admin to enter the username.
        do {
            printf("Enter username: ");
            scanf("%49s", newUser.username);
        } while (!validateUsername(newUser.username));

        // Prompt the admin to enter the password twice and validate that they match.
        do {
            passwordPtr = getpass("Enter password: ");
            strcpy(password, passwordPtr);
            password2Ptr = getpass("Enter password again: ");
            strcpy(password2, password2Ptr);
            if(strcmp(password,password2) != 0){
                printf("%sPasswords don't match!%s\n",RED,RESET);
            } else {
                // If passwords match, store the password in newUser struct.
                strcpy(newUser.password, password);
            }
        } while (!validatePassword(newUser.password));

        // Prompt the admin to enter the email.
        do {
            printf("Enter email: ");
            scanf("%99s", newUser.email);
        } while (!validateEmail(newUser.email));

        // Prompt the admin to enter the role (0 for admin, 1 for regular user).
        do {
            printf("Enter role (0 for admin, 1 for regular user): ");
            scanf("%d", &newUser.role);
        } while (!validateRole(newUser.role));

        // Hash the password using SHA-256 algorithm.
        unsigned char hashed_password[SHA256_DIGEST_LENGTH];
        hashPassword(newUser.password, hashed_password);

        // Convert hashed password to hexadecimal string.
        char hashed_password_str[SHA256_DIGEST_LENGTH * 2 + 1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            sprintf(&hashed_password_str[i * 2], "%02x", hashed_password[i]);
        }

        // Construct SQL query to insert new user into the database.
        char sql[1000];
        sprintf(sql, "INSERT INTO users (username, password, email, role) VALUES (?, ?, ?, ?);");

        // Prepare SQL statement
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        
        // Bind parameters to SQL statement
        sqlite3_bind_text(stmt, 1, newUser.username, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashed_password_str, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, newUser.email, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, newUser.role);

        // Execute SQL statement
        return_code = sqlite3_step(stmt);
        if (return_code != SQLITE_DONE) {
            // If SQL execution fails, print error message.
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            // If user added successfully, print success message.
            printf("%sUser added successfully.%s\n",GREEN,RESET);
        }

        // Finalize the SQL statement and close the database connection.
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
}

// Delete a user.

/**
 * @brief Deletes a user from the database.
 * 
 * @details This function allows the admin user to delete a user from the database
 *          by providing the username of the user to be deleted. It validates the username,
 *          constructs a SQL query to delete the user, and executes the query.
 *          If the current user is not an admin, permission denial message is displayed.
 * 
 * @param None
 * 
 * @return None
 */
void delUser() {
    // Check if the current user is an admin (userRole == 0)
    if(userRole != 0){
        // Display permission denial message if the current user is not an admin.
        printf("%sYou don't have permission for this action! This incident will be reported.%s\n",RED,RESET);
    } else {
        // If the current user is an admin, proceed with deleting a user.
        
        // Declare variables for interacting with the database.
        sqlite3 *db;
        char *errMsg = 0;
        int return_code;

        // Open the database
        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code) {
            // If unable to open the database, print error message and return.
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Declare variable to store the username of the user to be deleted.
        char del_username[MAX_AUTHOR_LENGTH];
        
        // Prompt the admin to enter the username of the user to be deleted.
        do{
            printf("Enter the user to delete: ");
            scanf(" %[^\n]s", &del_username);
        }while(!validateUsername(del_username));

        // Construct SQL query to delete the user from the database.
        char sql[1000];
        sprintf(sql, "DELETE FROM users WHERE username=?;");
        
        // Prepare SQL statement
        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, del_username, -1, SQLITE_STATIC);

        // Execute SQL statement
        return_code = sqlite3_step(stmt);
        if (return_code != SQLITE_DONE) {
            // If SQL execution fails, print error message.
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        } else {
            // If user deleted successfully, print success message.
            printf("%sUser deleted successfully.%s\n",GREEN,RESET);
        }

        // Finalize the SQL statement and close the database connection.
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
}


/**
 * @brief Authenticates a user based on provided username and password.
 * 
 * @details This function prompts the user to enter a username and a password.
 *          It then hashes the password, queries the database to check if the
 *          provided username and hashed password match any user credentials in the database.
 *          If a match is found, it retrieves the role of the user and updates global variables
 *          `userName` and `userRole` accordingly. If authentication is successful, it returns true;
 *          otherwise, it returns false.
 * 
 * @param None
 * 
 * @return True if authentication is successful, false otherwise.
 */
bool authenticateUser() {
    // Declare variables for storing username, password, SQL query, and database interactions.
    char username[50], *password;
    char sql[1000];
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int return_code;

    // Open the database connection.
    return_code = sqlite3_open(DATABASE_FILE, &db);

    // Prompt the user to enter username and password.
    printf("Enter username: ");
    scanf("%49s", username);
    password = getpass("Enter password: ");

    // Hash the password
    unsigned char hashed_password[SHA256_DIGEST_LENGTH];
    hashPassword(password, hashed_password);

    // Convert the hashed password to a hexadecimal string.
    char hashed_password_str[SHA256_DIGEST_LENGTH * 2 + 1]; // Add 1 for null terminator.
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(&hashed_password_str[i * 2], "%02x", hashed_password[i]);
    }

    // Prepare SQL statement to query for user credentials and role,
    sprintf(sql, "SELECT role FROM users WHERE username=? AND password=?;");

    // Prepare SQL statement and bind parameters.
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashed_password_str, -1, SQLITE_STATIC);

    // Execute SQL statement
    return_code = sqlite3_step(stmt);
    if (return_code == SQLITE_ROW) {
        // If authentication is successful, retrieve the user's role.
        int role = sqlite3_column_int(stmt, 0);
        // Print authentication success message.
        printf("%sAuthentication successful!%s\n",GREEN ,RESET);
        // Update global variables with user information.
        strcpy(userName, username);
        userRole = role;
        // Finalize the SQL statement and close the database connection.
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        // Return true to indicate successful authentication.
        return true;
    } else {
        // If authentication fails, print error message.
        printf("%sIncorrect username or password.\n%s",RED,RESET);
        // Finalize the SQL statement and close the database connection.
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        // Return false to indicate authentication failure.
        return false;
    }
}

/**
 * @brief Displays the login interface and authenticates the user.
 * 
 * @details This function clears the screen, displays the Bookery introduction,
 *          and prompts the user to log in by calling the `authenticateUser` function.
 *          If the authentication is unsuccessful, the program exits.
 * 
 * @param None
 * 
 * @return None
 */
void login(){
    // Clear the screen
    printf("\033c");
    // Display the Bookery introduction.
    introBookery();
    // Authenticate the user
    if(authenticateUser() != true){
        // If authentication fails, exit the program.
        exit(0);
    }
}

//********************************************************************************************************************************************************

/**
 * @brief Displays the list of users.
 * 
 * @details This function retrieves user data from the database and displays it in a tabular format.
 *          Only users with admin privileges can access this function.
 * 
 * @param None
 * 
 * @return None
 */
void displayUsers() {
    if(userRole != 0){
        printf("%sYou dont have permission for this action!\n this incident will be reported.\n%s",RED,RESET);
    }else{
        sqlite3 *db;
        sqlite3_stmt *stmt;
        int return_code;

        // Open the database
        return_code = sqlite3_open(DATABASE_FILE, &db);
        if (return_code != SQLITE_OK) {
            fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Print the header
        printf("\n********** List of Users **************\n");

        // SQL query to select user data.
        const char *sql = "SELECT username, email, role FROM users;";
        return_code = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (return_code != SQLITE_OK) {
            fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Calculate maximum widths for each column.
        int max_user_width = 0;
        int max_email_width = 0;
        int max_role_width = 5;
        
        // Iterate over the result set to calculate maximum column widths.
        while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            max_user_width = fmax(max_user_width, (int)strlen((const char *)sqlite3_column_text(stmt, 0)));
            max_email_width = fmax(max_email_width, (int)strlen((const char *)sqlite3_column_text(stmt, 1)));
        }

        // Print column separator
        printf("%s",BLUE);
        for(int i =0;i < (max_user_width + max_email_width + max_role_width) + 8;i++){
            printf("-");
        }
        printf("%s\n",RESET);

        // Print column headers
        printf("%s%-*s | %-*s | %-*s |%s\n",
            BLUE,
            max_user_width, "User",
            max_email_width, "Email",
            max_role_width,"Role",
                RESET);

        // Print column separator
        printf("%s",BLUE);
        for(int i =0;i < (max_user_width + max_email_width + max_role_width) + 8;i++){
            printf("-");
        }
        printf("%s\n",RESET);

        // Print user data
        sqlite3_reset(stmt); // Reset the statement to re-execute
        while ((return_code = sqlite3_step(stmt)) == SQLITE_ROW) {
            printf("%-*s | %-*s | %-*s | \n",
                max_user_width, (const char *)sqlite3_column_text(stmt, 0),
                max_email_width, (const char *)sqlite3_column_text(stmt, 1),
                max_role_width, (const char *)sqlite3_column_text(stmt, 2));

            // Print column separator
            for(int i =0;i < (max_user_width + max_email_width + max_role_width) + 8;i++){
                printf("-");
            }
            printf("\n");
        }

        // Finalize and close the database connection.
        sqlite3_finalize(stmt);
        sqlite3_close(db);   
    }

}