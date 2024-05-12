# Bookery - Bookshop Management System

Bookery is a bookshop management system implemented in C. It provides functionality to manage book inventory, sales, and rentals, with a focus on speed, security, and user-friendliness.

## Features

- Add, update, and delete books from inventory
- Display all books or search for specific books by title, author, or genre
- Sell books to customers and update sales records
- Rent books to customers, manage rental records, and calculate rental fees
- Generate sales and rental reports
- User authentication with role-based access control

## Installation

### Clone the repository
```bash
git clone https://github.com/martian58/bookery.git

cd bookery
```
### Linux

1. Install the SQLite development library:
```bash
sudo apt-get install libsqlite3-dev
```


2. Install the OpenSSL development library:
```bash
sudo apt-get install libssl-dev
```


### Mac OS

1. Install SQLite using Homebrew:
```bash
brew install sqlite
```


2. Install OpenSSL using Homebrew:
```bash
brew install openssl
```

### Windows

1. Windows Os is not supported and not recommended.



## Compilation

Compile the code using GCC:
```bash
gcc bookery.c -o bookery -lsqlite3 -lssl -lm -lcrypto
```
**Note:** Compiled executable present in the repository is for debian based linux distrobutions.


## Usage

1. Run the compiled executable:
```bash
./bookery
```

2. Follow the on-screen instructions to use the system.

3. Advanced CLI commands can be obtained by running "help" command.

## Default Credentials

### Admin Account
- **Username:** admin
- **Password:** admin

### User Account
- **Username:** mehdi
- **Password:** mehdi


## Contributors

- Alizada Fuad
- Mehdi Hasanli
- Toghrul Abdullazada
- Tural Gadirov
- Ilham Bakhishov
