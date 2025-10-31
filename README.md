# Supermarket Management (C)

![Language](https://img.shields.io/badge/Language-C-blue)
![Type](https://img.shields.io/badge/App-CLI%20tool-lightgrey)
![Focus](https://img.shields.io/badge/Focus-Data%20Structures-orange)
![License](https://img.shields.io/badge/License-MIT-yellow)

This project was developed as part of an *Introduction to Systems Programming* course in C.  
It implements a command-line system for managing a supermarket, including products, customers, and purchases.

## Features
- Product & customer management (add, update, delete, list)
- Categorized items and purchase limits
- Text & binary file storage
- Custom data structures and modular design


## Build & Run
```bash
# build
gcc -o supermarket main.c inventory.c customer.c io.c utils.c

# run
./supermarket
