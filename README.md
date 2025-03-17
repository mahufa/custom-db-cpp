# Custom SQL Database Implementation

---

This project is a custom implementation of an SQL-based database engine written in C++. It was developed as part of my academic work. The engine supports basic SQL operations such as creating tables, inserting and deleting data, querying with SELECT, and saving/loading the database.

## Features

---

- Create and drop tables
- Insert, update, and delete records
- Execute basic SELECT queries
- Save and load database state from a file

## Requirements

---

A C++ compiler supporting C++11 or later
CMake (version 3.10 or higher)
Building the Project


## Running the Project

---

After building, run the generated executable. The application demonstrates basic SQL operations and outputs results to the console.

## Project Structure

---

- **main.cpp** – The entry point of the application.
- **Database.cpp / Database.h** – The core implementation of the SQL database engine.
- **ProjectUtilities.cpp / ProjectUtilities.h** – Helper functions.
- **StructureElements.cpp / StructureElements.h** – Definitions of the custom data structures.
- **CMakeLists.txt** – Build configuration.